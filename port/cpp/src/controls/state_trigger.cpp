// maui::controls state triggers — the StateTriggerBase lifecycle + the concrete trigger family
// (state_trigger.hpp). Ported from StateTriggerBase.cs / StateTrigger.cs / CompareStateTrigger.cs /
// DeviceStateTrigger.cs / OrientationStateTrigger.cs / AdaptiveTrigger.cs.
#include "maui/controls/state_trigger.hpp"

#include <cmath>
#include <string>
#include <utility>

#include "maui/controls/element.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/event.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/essentials/device_display.hpp"
#include "maui/essentials/device_info.hpp"

namespace maui::controls
{
    void state_trigger_base::set_active(bool active)
    {
        if (is_active_ != active)
        {
            is_active_ = active;
            is_active_changed.raise(); // StateTriggerBase.IsActive's private setter
        }
        // C# SetActive pokes the group UNCONDITIONALLY (VisualState?.VisualStateGroup?.UpdateStateTriggers()).
        if (update_state_triggers_)
        {
            update_state_triggers_();
        }
    }

    void state_trigger_base::send_attached()
    {
        if (is_attached_)
        {
            return;
        }
        on_attached();
        is_attached_ = true;
    }

    void state_trigger_base::send_detached()
    {
        if (!is_attached_)
        {
            return;
        }
        on_detached();
        is_attached_ = false;
    }

    void state_trigger_base::set_owner(element* visual_element,
                                       maui::core::move_only_function<void()> update_state_triggers)
    {
        visual_element_ = visual_element;
        update_state_triggers_ = std::move(update_state_triggers);
    }

    // ---- device_state_trigger (DeviceStateTrigger.cs) ----

    void device_state_trigger::set_device(std::string value)
    {
        if (device_ == value)
        {
            return;
        }
        device_ = std::move(value);
        update_state(); // OnDeviceChanged → UpdateState (not gated on IsAttached)
    }

    void device_state_trigger::update_state()
    {
        if (device_.empty())
        {
            return; // C# UpdateState bails on a null/empty Device (DevicePlatform.Create would throw)
        }
        const maui::devices::device_platform device = maui::devices::device_platform::create(device_);
        if (device == maui::devices::device_platform::create("UWP")) // the C# legacy "UWP" → WinUI alias
        {
            set_active(maui::devices::device_info::platform() == maui::devices::device_platform::win_ui());
        }
        else
        {
            set_active(maui::devices::device_info::platform() == device);
        }
    }

    // ---- orientation_state_trigger (OrientationStateTrigger.cs) ----

    orientation_state_trigger::~orientation_state_trigger()
    {
        unsubscribe(); // never leak the MainDisplayInfoChanged subscription (PROFILE §8 teardown)
    }

    void orientation_state_trigger::set_orientation(maui::devices::display_orientation value)
    {
        if (orientation_ == value)
        {
            return;
        }
        orientation_ = value;
        update_state(); // OnOrientationChanged → UpdateState (not gated on IsAttached)
    }

    void orientation_state_trigger::on_attached()
    {
        update_state();
        unsubscribe(); // defensive — C# `+=` would stack, the port keeps exactly one subscription
        display_token_ = maui::devices::device_display::add_main_display_info_changed(
            [this](const maui::devices::display_info& /*info*/) { update_state(); });
    }

    void orientation_state_trigger::on_detached()
    {
        unsubscribe();
    }

    void orientation_state_trigger::unsubscribe()
    {
        if (display_token_ != 0)
        {
            maui::devices::device_display::remove_main_display_info_changed(display_token_);
            display_token_ = 0;
        }
    }

    void orientation_state_trigger::update_state()
    {
        using maui::devices::display_orientation;
        const display_orientation current = maui::devices::device_display::main_display_info().orientation;
        // C# branches through DisplayOrientationExtensions.IsLandscape/IsPortrait; with the two concrete
        // orientations that collapses to equality, and Unknown never activates.
        if (orientation_ == display_orientation::landscape || orientation_ == display_orientation::portrait)
        {
            set_active(current == orientation_);
        }
        else
        {
            set_active(false);
        }
    }

    // ---- adaptive_trigger (AdaptiveTrigger.cs) ----

    adaptive_trigger::~adaptive_trigger()
    {
        detach_events(); // RAII teardown of the size_changed subscription
    }

    void adaptive_trigger::set_min_window_width(double value)
    {
        min_window_width_ = value;
        update_state(); // OnMinWindowDimensionChanged → UpdateState (gated on IsAttached inside)
    }

    void adaptive_trigger::set_min_window_height(double value)
    {
        min_window_height_ = value;
        update_state();
    }

    void adaptive_trigger::on_attached()
    {
        attach_events();
        update_state(/*known_attached=*/true); // OnAttached runs before IsAttached flips true
    }

    void adaptive_trigger::on_detached()
    {
        detach_events();
    }

    void adaptive_trigger::attach_events()
    {
        detach_events();
        // _visualElement = VisualState?.VisualStateGroup?.VisualElement; _window = _visualElement?.Window.
        window_ = attached_element() != nullptr ? attached_element()->containing_window() : nullptr;
        if (window_ != nullptr)
        {
            size_token_ = maui::core::connect_scoped(window_->size_changed, [this] { update_state(); });
        }
    }

    void adaptive_trigger::detach_events()
    {
        size_token_.reset();
        window_ = nullptr;
    }

    void adaptive_trigger::update_state(bool known_attached)
    {
        if (!known_attached && !is_attached())
        {
            return;
        }
        const double width = window_ != nullptr ? window_->width() : -1;
        const double height = window_ != nullptr ? window_->height() : -1;
        // C# bails while the window size is unknown (-1); the port's window geometry additionally starts
        // at Dimension.Unset (NaN), treated the same way.
        if (std::isnan(width) || std::isnan(height) || width == -1 || height == -1)
        {
            return;
        }
        set_active(width >= min_window_width_ && height >= min_window_height_);
    }
} // namespace maui::controls
