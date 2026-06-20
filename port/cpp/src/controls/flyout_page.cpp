// maui::controls::flyout_page — out-of-line definitions: the shared bindable-property descriptors, the
// pane setters with the C# guards, the IsPresented/behavior plumbing (ShouldShowSplitMode /
// UpdateFlyoutLayoutBehavior), the pane lifecycle propagation, and the default-handler
// self-registration. See flyout_page.hpp.

#include "maui/controls/flyout_page.hpp"

#include <stdexcept>
#include <string>
#include <string_view>

#include "maui/controls/content_page.hpp"
#include "maui/controls/flyout_layout_behavior.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/flyout_page_handler.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/essentials/device_display.hpp"
#include "maui/essentials/device_info.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<bool>& flyout_page::is_presented_property()
    {
        // The C# default(bool) is false; the macOS defaultValueCreator quirk is applied per-instance in
        // the constructor (see flyout_page()).
        static const maui::core::bindable_property<bool> descriptor{"is_presented", false};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& flyout_page::is_gesture_enabled_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"is_gesture_enabled", true};
        return descriptor;
    }

    const maui::core::bindable_property<flyout_layout_behavior>& flyout_page::flyout_layout_behavior_property()
    {
        static const maui::core::bindable_property<flyout_layout_behavior> descriptor{"flyout_layout_behavior",
                                                                                      flyout_layout_behavior::default_};
        return descriptor;
    }

    flyout_page::flyout_page()
    {
        this->set_style_target_type<flyout_page>(); // implicit / class style match

        // C# GetDefaultValue (IsPresentedProperty's defaultValueCreator): presented by default ONLY on
        // classic macOS (NOT MacCatalyst — the literal C# check). Evaluated once here; install a
        // device_info mock BEFORE constructing in tests, like the C# fixture does.
        if (maui::devices::device_info::platform() == maui::devices::device_platform::mac_os())
        {
            is_presented_.set(true);
        }

        // C# OnMainDisplayInfoChanged → Handler.UpdateValue(nameof(FlyoutBehavior)): an orientation flip
        // can move the page in/out of split mode, so the handler re-reads the computed behavior.
        display_changed_token_ = maui::devices::device_display::add_main_display_info_changed(
            [this](const maui::devices::display_info& /*info*/) {
                if (const auto& element_handler = handler())
                {
                    element_handler->update_value("flyout_behavior");
                }
            });

        // C# OnParentSet: a flyout page joining a container must already have both panes.
        parent_set.connect([this] {
            if (logical_parent() != nullptr && (flyout_ == nullptr || detail_ == nullptr))
            {
                throw std::runtime_error("Flyout and Detail must be set before adding FlyoutPage to a container");
            }
        });
    }

    flyout_page::~flyout_page()
    {
        maui::devices::device_display::remove_main_display_info_changed(display_changed_token_);
    }

    void flyout_page::set_pane(content_page*& slot, content_page* value, const char* role)
    {
        // C# throws ArgumentNullException once a value was set; the port rejects an initial null too
        // (C# would NullReference on the very next line — see the header).
        if (value == nullptr)
        {
            throw std::invalid_argument(std::string{role} + " cannot be set to null once a value is set.");
        }
        if (slot == value)
        {
            return;
        }
        // C#: "must not already have a parent" (value.RealParent != null).
        if (value->logical_parent() != nullptr)
        {
            throw std::runtime_error(std::string{role} + " must not already have a parent.");
        }

        content_page* const previous = slot;
        // C# SendNavigatingFrom(previous → value) — deferred (no page navigation plumbing; header note).
        if (previous != nullptr)
        {
            detach_logical_child(*previous);
        }
        slot = value;
        attach_logical_child(*value);
        this->on_property_changed(role); // the event + the handler's MapFlyout / MapDetail

        // C#: the appearing/disappearing handoff only once this page has appeared.
        if (has_appeared())
        {
            if (previous != nullptr)
            {
                previous->send_disappearing();
            }
            value->send_appearing();
        }
        // C# SendNavigatedFrom/To + the replaced pane's DisconnectHandlers — deferred (header note).
    }

    void flyout_page::set_flyout(content_page* value)
    {
        // C#: "Title property must be set on Flyout page" (checked before the parent guard).
        if (value != nullptr && value->title().empty())
        {
            throw std::runtime_error("Title property must be set on Flyout page");
        }
        set_pane(flyout_, value, "flyout");
    }

    void flyout_page::set_detail(content_page* value)
    {
        set_pane(detail_, value, "detail");
    }

    void flyout_page::set_is_presented(bool value)
    {
        if (is_presented_.get() == value)
        {
            return; // SetValue with an equal value runs no callbacks (and so no guard) in C#
        }
        // C# OnIsPresentedPropertyChanging (non-shimmed branch): the flyout cannot be hidden while the
        // page is showing split mode.
        if (!value && should_show_split_mode())
        {
            throw std::runtime_error("Can't change IsPresented when setting the current FlyoutLayoutBehavior");
        }
        is_presented_.set(value);
    }

    bool flyout_page::should_show_split_mode() const
    {
        // C# IFlyoutPageController.ShouldShowSplitMode. Orientation comes from the MAIN display (the
        // port's stand-in for Window.GetOrientation() — header note).
        if (maui::devices::device_info::idiom() == maui::devices::device_idiom::phone())
        {
            return false;
        }
        const flyout_layout_behavior behavior = layout_behavior();
        const maui::devices::display_orientation orientation =
            maui::devices::device_display::main_display_info().orientation;
        const bool split_on_landscape =
            (behavior == flyout_layout_behavior::split_on_landscape || behavior == flyout_layout_behavior::default_) &&
            orientation == maui::devices::display_orientation::landscape;
        const bool split_on_portrait = behavior == flyout_layout_behavior::split_on_portrait &&
                                       orientation == maui::devices::display_orientation::portrait;
        return behavior == flyout_layout_behavior::split || split_on_landscape || split_on_portrait;
    }

    bool flyout_page::should_show_toolbar_button() const
    {
        // C# FlyoutPage.ShouldShowToolbarButton — the inverse split test, with the phone fast-path.
        if (maui::devices::device_info::idiom() == maui::devices::device_idiom::phone())
        {
            return true;
        }
        const flyout_layout_behavior behavior = layout_behavior();
        const maui::devices::display_orientation orientation =
            maui::devices::device_display::main_display_info().orientation;
        const bool split_on_landscape =
            (behavior == flyout_layout_behavior::split_on_landscape || behavior == flyout_layout_behavior::default_) &&
            orientation == maui::devices::display_orientation::landscape;
        const bool split_on_portrait = behavior == flyout_layout_behavior::split_on_portrait &&
                                       orientation == maui::devices::display_orientation::portrait;
        return behavior != flyout_layout_behavior::split && !split_on_landscape && !split_on_portrait;
    }

    void flyout_page::update_flyout_layout_behavior()
    {
        // C# UpdateFlyoutLayoutBehavior(page): entering split mode presents the flyout (true never
        // trips the hiding guard); a non-default behavior then locks IsPresented.
        if (should_show_split_mode())
        {
            is_presented_.set(true);
            if (layout_behavior() != flyout_layout_behavior::default_)
            {
                can_change_is_presented_ = false;
            }
        }
    }

    bool flyout_page::send_back_button_pressed() const
    {
        // C# OnBackButtonPressed: the presented flyout's and the detail's Page.SendBackButtonPressed
        // legs are deferred (no page-level back hook — header note); the FlyoutPage-level
        // BackButtonPressed event leg is the portable branch.
        back_button_pressed_event_args args;
        back_button_pressed.raise(args);
        return args.handled;
    }

    void flyout_page::send_appearing()
    {
        if (has_appeared())
        {
            return;
        }
        // FlyoutPage.OnAppearing: both panes appear first, then CanChangeIsPresented resets, the layout
        // behavior re-applies, and finally the page's own appearing fires (base.OnAppearing).
        if (flyout_ != nullptr)
        {
            flyout_->send_appearing();
        }
        if (detail_ != nullptr)
        {
            detail_->send_appearing();
        }
        can_change_is_presented_ = true;
        update_flyout_layout_behavior();
        content_page::send_appearing();
    }

    void flyout_page::send_disappearing()
    {
        if (!has_appeared())
        {
            return;
        }
        // FlyoutPage.OnDisappearing: both panes disappear first, then the page's own event fires.
        if (flyout_ != nullptr)
        {
            flyout_->send_disappearing();
        }
        if (detail_ != nullptr)
        {
            detail_->send_disappearing();
        }
        content_page::send_disappearing();
    }

    maui::graphics::size flyout_page::arrange(const maui::graphics::rect& bounds)
    {
        // Frame this page + size the native split-view host (the flyout handler's platform_arrange), exactly
        // as content_page::arrange — but then arrange the two PANES, which content_page::arrange SKIPS (it
        // only knows the inherited content_, null on a flyout page). Without this the detail pane's content
        // (scroll view + controls) never gets a frame and renders blank. Each pane arranges HOST-RELATIVE
        // ({0,0,w,h}): its native view is a subview of the split VC's column (UIKit positions the column),
        // so the pane content must start at its column origin — the host-relative arrange convention. On a
        // collapsed phone the detail column fills the screen; in split mode the split VC clips each pane.
        frame_ = bounds;
        if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(handler().get()))
        {
            view_handler->platform_arrange(bounds);
        }
        const maui::graphics::rect pane_bounds{0, 0, bounds.width, bounds.height};
        if (detail_ != nullptr)
        {
            detail_->arrange(pane_bounds);
        }
        if (flyout_ != nullptr)
        {
            flyout_->arrange(pane_bounds);
        }
        return {bounds.width, bounds.height};
    }

    void flyout_page::on_property_changed(std::string_view name)
    {
        content_page::on_property_changed(name); // the event + the handler's matching mapper entry
        if (name == "is_presented")
        {
            is_presented_changed.raise(); // C# OnIsPresentedPropertyChanged → IsPresentedChanged
        }
        else if (name == "flyout_layout_behavior")
        {
            // C# OnFlyoutLayoutBehaviorPropertyChanged + Controls' MapFlyoutLayoutBehavior: re-apply the
            // behavior, then have the handler re-read the COMPUTED IFlyoutView.FlyoutBehavior.
            update_flyout_layout_behavior();
            if (const auto& element_handler = handler())
            {
                element_handler->update_value("flyout_behavior");
            }
        }
    }
} // namespace maui::controls

// Self-register the default handler for flyout_page (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::flyout_page, maui::core::flyout_page_handler)
