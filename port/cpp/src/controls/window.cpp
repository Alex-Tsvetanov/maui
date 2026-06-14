// maui::controls::window — the IWindow lifecycle drive + page hosting + the handler seam + geometry
// (window.hpp). Ported from Window.cs.
#include "maui/controls/window.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector> // --- modal events + overlay (G4) ---

#include "maui/controls/content_page.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/menu_bar_tracker.hpp" // --- chrome (W1-11) ---
#include "maui/controls/modal_event_args.hpp" // --- modal events (G4) ---
#include "maui/controls/navigation_page.hpp"  // --- chrome (W1-11) ---
#include "maui/controls/title_bar.hpp"        // --- chrome (W1-11) ---
#include "maui/controls/toolbar.hpp"          // --- chrome (W1-11) ---
#include "maui/controls/toolbar_tracker.hpp"  // --- chrome (W1-11) ---
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/dimension.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_window_overlay.hpp" // --- overlay (G4) ---
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

    window::window()
    {
        // C# Window(): _menuBarTracker = new MenuBarTracker(this, "MenuBar") — every page-sourced menu
        // bar change re-runs the window handler's "menu_bar" map (the C# handlerProperty poke).
        menu_bar_tracker_ = std::make_unique<menu_bar_tracker>();
        menu_bar_items_token_ = maui::core::scoped_connection(
            menu_bar_tracker_->collection_changed,
            menu_bar_tracker_->collection_changed.connect([this] { update_handler_value("menu_bar"); }));
    }

    window::window(content_page& page) : window()
    {
        set_content(page);
    }

    // Out-of-line: the chrome unique_ptrs (toolbar / trackers) need their complete types here.
    window::~window() = default;

    // --- chrome (W1-11) -----------------------------------------------------------------------------
    maui::core::i_toolbar* window::toolbar() const
    {
        return toolbar_.get();
    }

    maui::core::i_menu_bar* window::menu_bar() const
    {
        // C# IMenuBarElement.MenuBar => _menuBarTracker.MenuBar (null while the aggregate is empty).
        return menu_bar_tracker_->menu_bar();
    }

    maui::core::i_title_bar* window::title_bar() const
    {
        return title_bar_;
    }

    void window::set_title_bar(maui::controls::title_bar* value)
    {
        // C# Window.TitleBar: replacing (or clearing) re-runs the handler's "title_bar" map, which
        // swaps the native titlebar accessory. NON-owning — the caller owns both old and new bars.
        if (title_bar_ == value)
        {
            return;
        }
        title_bar_ = value;
        update_handler_value("title_bar");
    }

    void window::setup_chrome()
    {
        // The menu bar tracker always follows the hosted page (C# tracker.Target = window.Page).
        menu_bar_tracker_->set_target(content_);

        // C# NavigationPage assigns a NavigationPageToolbar to its window when it lands in one
        // (NavigationPage.TryHookupToolbar: w.Toolbar = new NavigationPageToolbar(w, w.Page)); the port
        // creates the window toolbar here when the hosted page IS a navigation_page (the only
        // chrome-bearing page container at this layer — documented simplification).
        if (dynamic_cast<navigation_page*>(content_) == nullptr)
        {
            if (toolbar_ != nullptr)
            {
                // The new page has no navigation chrome: hide the toolbar and empty its aggregate
                // (C# NavigationPageToolbar sets IsVisible = false when no NavigationPage is current).
                toolbar_tracker_->set_target(nullptr);
                toolbar_->set_is_visible(false);
                refresh_toolbar_items();
            }
            return;
        }
        if (toolbar_ == nullptr)
        {
            toolbar_ = std::make_unique<maui::controls::toolbar>(this);
            // The Toolbar.SetProperty → Handler.UpdateValue collapse: any chrome change re-runs the
            // WINDOW handler's "toolbar" map (see toolbar.hpp).
            toolbar_->set_notify([this](std::string_view /*property*/) { update_handler_value("toolbar"); });
            toolbar_tracker_ = std::make_unique<toolbar_tracker>();
            toolbar_items_token_ = maui::core::scoped_connection(
                toolbar_tracker_->collection_changed,
                toolbar_tracker_->collection_changed.connect([this] { refresh_toolbar_items(); }));
        }
        toolbar_tracker_->set_target(content_);
        toolbar_->set_is_visible(true); // C# NavigationPageToolbar: visible while a nav page is current
        refresh_toolbar_items();
        update_handler_value("toolbar"); // a freshly-created toolbar reaches an already-attached handler
    }

    void window::refresh_toolbar_items()
    {
        // C# NavigationPageToolbar.OnToolbarItemsChanged: ToolbarItems = _toolbarTracker.ToolbarItems.
        if (toolbar_ != nullptr && toolbar_tracker_ != nullptr)
        {
            toolbar_->set_toolbar_items(toolbar_tracker_->toolbar_items());
        }
    }
    // --- end chrome (W1-11) -------------------------------------------------------------------------

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
        setup_chrome();                  // chrome (W1-11): retarget the menu bar tracker + (un)wire the window toolbar
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

    // --- modal events (G4) -------------------------------------------------------------------------
    // The four Window.OnModal* drivers + OnPopCanceled. C# also adds/removes the modal from
    // _visualChildren and notifies the application; in the port the modal visual-tree bookkeeping lives
    // in the navigation subsystem (which owns the modal stack) and the application has no modal-event
    // notification (out of scope), so these are the pure event drives — the events themselves match
    // Window.cs exactly, fired in the same order around the modal push/pop.
    void window::on_modal_pushing(content_page& modal) const
    {
        modal_pushing_event_args args;
        args.modal = &modal;
        modal_pushing.raise(args); // C# ModalPushing?.Invoke(this, new ModalPushingEventArgs(modalPage))
    }

    void window::on_modal_pushed(content_page& modal) const
    {
        modal_pushed_event_args args;
        args.modal = &modal;
        modal_pushed.raise(args); // C# ModalPushed?.Invoke(this, new ModalPushedEventArgs(modalPage))
    }

    bool window::on_modal_popping(content_page& modal) const
    {
        // C# OnModalPopping: raise the event, then return args.Cancel so the caller can abort the pop.
        modal_popping_event_args args;
        args.modal = &modal;
        modal_popping.raise(args);
        return args.cancel;
    }

    void window::on_modal_popped(content_page& modal) const
    {
        modal_popped_event_args args;
        args.modal = &modal;
        modal_popped.raise(args); // C# ModalPopped?.Invoke(this, new ModalPoppedEventArgs(modalPage))
    }

    void window::on_pop_canceled() const
    {
        pop_canceled.raise(); // C# PopCanceled?.Invoke(this, EventArgs.Empty)
    }

    bool window::add_overlay(maui::core::i_window_overlay& overlay)
    {
        // C# Window.AddOverlay: reject a duplicate (HashSet.Add semantics), else add + initialize +
        // invalidate so it is drawn. (The IVisualDiagnosticsOverlay rejection is moot — that overlay is
        // out of scope here.)
        if (std::ranges::find(overlays_, &overlay) != overlays_.end())
        {
            return false;
        }
        overlays_.push_back(&overlay);
        overlay.initialize();
        overlay.invalidate();
        return true;
    }

    bool window::remove_overlay(maui::core::i_window_overlay& overlay)
    {
        // C# Window.RemoveOverlay: remove (HashSet.Remove semantics), then deinitialize the layer.
        const auto it = std::ranges::find(overlays_, &overlay);
        if (it == overlays_.end())
        {
            return false;
        }
        overlays_.erase(it);
        overlay.deinitialize();
        return true;
    }
    // --- end (G4) ----------------------------------------------------------------------------------

    void window::on_property_changed(std::string_view name)
    {
        // Route through element (raise property_changed + drive bindings + fan effects out — G3).
        maui::controls::element::on_property_changed(name);
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
