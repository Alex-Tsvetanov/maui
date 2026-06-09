// maui::controls::window — the IWindow lifecycle drive + page hosting + the handler seam + geometry
// (window.hpp). Ported from Window.cs.
#include "maui/controls/window.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/content_page.hpp"
#include "maui/controls/element.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/dimension.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/window_handler.hpp"
#include "maui/graphics/rect.hpp"

namespace maui::controls
{
    // The geometry descriptors (Window.X/Y/Width/Height). One shared instance per property (the C#
    // XProperty/… fields), default Dimension.Unset (NaN) — IWindow returns Unset until the platform reports
    // a frame. Names match the window_handler's mapper keys.
    const maui::core::bindable_property<double>& window_x_property()
    {
        static const maui::core::bindable_property<double> descriptor{"x", maui::core::dimension::unset};
        return descriptor;
    }
    const maui::core::bindable_property<double>& window_y_property()
    {
        static const maui::core::bindable_property<double> descriptor{"y", maui::core::dimension::unset};
        return descriptor;
    }
    const maui::core::bindable_property<double>& window_width_property()
    {
        static const maui::core::bindable_property<double> descriptor{"width", maui::core::dimension::unset};
        return descriptor;
    }
    const maui::core::bindable_property<double>& window_height_property()
    {
        static const maui::core::bindable_property<double> descriptor{"height", maui::core::dimension::unset};
        return descriptor;
    }

    window::window() = default;

    window::window(content_page& page)
    {
        set_content(page);
    }

    void window::set_handler(std::shared_ptr<maui::core::i_element_handler> value)
    {
        if (handler_ == value)
        {
            return;
        }
        std::shared_ptr<maui::core::i_element_handler> const previous = handler_;
        handler_ = std::move(value);
        if (handler_)
        {
            handler_->set_virtual_view(*this); // creates the NSWindow + runs the mapper (Title/Content/geometry)
        }
        if (previous && previous != handler_)
        {
            previous->disconnect_handler();
        }
    }

    void window::set_content(element& page)
    {
        if (content_ == &page)
        {
            return;
        }
        if (content_ != nullptr && is_activated_)
        {
            detach_page(); // tear the old page off the active window first
        }
        content_ = &page;
        content_->set_inherited_binding_context(raw_binding_context()); // the page inherits the window context
        if (is_activated_)
        {
            attach_page();
        }
        update_handler_value("content"); // Window.OnPropertyChanged(Page) -> MapContent re-hosts the page
    }

    void window::set_title(std::string value)
    {
        if (title_ == value)
        {
            return;
        }
        title_ = std::move(value);
        update_handler_value("title"); // Window.OnPropertyChanged(Title) -> MapTitle
    }

    void window::frame_changed(const maui::graphics::rect& frame)
    {
        // C# Window.FrameChanged: a no-op when the frame is unchanged.
        if (this->frame() == frame)
        {
            return;
        }
        ++batch_frame_update_; // suppress the per-property re-push while we set all four (UpdateHandlerValue)
        const bool size_changed_flag = (width_.get() != frame.width) || (height_.get() != frame.height);

        // Set at the handler specificity (Window.cs SetterSpecificity.FromHandler). A subsequent developer
        // set (Window.Width = …, manual specificity) then overrides AND removes this handler value, so the
        // developer can still pin the geometry after the platform reports a frame.
        x_.set(frame.x, maui::core::setter_specificity::from_handler);
        y_.set(frame.y, maui::core::setter_specificity::from_handler);
        width_.set(frame.width, maui::core::setter_specificity::from_handler);
        height_.set(frame.height, maui::core::setter_specificity::from_handler);

        --batch_frame_update_;
        batch_frame_update_ = std::max(0, batch_frame_update_); // Window.cs clamp (never below zero)
        if (batch_frame_update_ == 0 && size_changed_flag)
        {
            size_changed.raise();
        }
    }

    void window::send_created()
    {
        if (is_created_)
        {
            return; // C# throws "already created"; the port no-ops
        }
        is_created_ = true;
        created.raise();
    }

    void window::send_activated()
    {
        if (is_activated_)
        {
            return;
        }
        is_activated_ = true;
        activated.raise();
        if (content_ != nullptr)
        {
            attach_page(); // window now active -> page Loaded + Appearing
        }
    }

    void window::send_deactivated()
    {
        if (!is_activated_)
        {
            return;
        }
        is_activated_ = false;
        if (content_ != nullptr)
        {
            detach_page(); // page Disappearing + Unloaded
        }
        deactivated.raise();
    }

    void window::send_destroying()
    {
        if (is_activated_)
        {
            send_deactivated();
        }
        is_created_ = false;
        destroying.raise();
    }

    void window::send_resumed()
    {
        resumed.raise();
        if (on_resume_)
        {
            on_resume_(); // Window.cs: Application?.SendResume
        }
    }

    void window::send_stopped()
    {
        stopped.raise();
        if (on_sleep_)
        {
            on_sleep_(); // Window.cs: Application?.SendSleep
        }
    }

    void window::send_backgrounding()
    {
        backgrounding.raise(); // Window.cs: Backgrounding?.Invoke (IPersistedState payload omitted)
    }

    void window::on_property_changed(std::string_view name)
    {
        maui::core::bindable_object::on_property_changed(name); // raise property_changed + drive bindings
        update_handler_value(name);
    }

    void window::update_handler_value(std::string_view property)
    {
        // C# Window.UpdateHandlerValue: skip the geometry re-push while frame_changed is batching (the
        // native frame is already correct — re-pushing would loop).
        if (batch_frame_update_ > 0 &&
            (property == "x" || property == "y" || property == "width" || property == "height"))
        {
            return;
        }
        if (handler_)
        {
            handler_->update_value(property);
        }
    }

    void window::attach_page()
    {
        content_->set_containing_window(this); // page (+ its subtree) is now in a window -> Loaded
        if (auto* page = dynamic_cast<content_page*>(content_))
        {
            page->send_appearing(); // window-rooted Appearing (idempotent via has_appeared_)
        }
    }

    void window::detach_page()
    {
        if (auto* page = dynamic_cast<content_page*>(content_))
        {
            page->send_disappearing();
        }
        content_->set_containing_window(nullptr); // page (+ subtree) leaves the window -> Unloaded
    }
} // namespace maui::controls

// Opt-in self-registration (PROFILE §6): window resolves to window_handler in the default registry. Lives
// in the controls layer (like every other control's registration) so the Core handler stays controls-free.
// Same OBJECT-library tree-shaking caveat as the other handlers (see handler_registry.hpp).
MAUI_REGISTER_HANDLER(maui::controls::window, maui::core::window_handler)
