// switch_handler — headless platform recipe. A testable stand-in for a native switch: the mapped
// properties mirror into switch_platform, and `is_on` doubles as the native on/off state — a test
// simulates a user toggle by flipping it and invoking on_value_changed (the UISwitch.ValueChanged
// analog), which writes the value back through i_switch::set_is_on exactly like C#'s SwitchProxy
// .OnControlValueChanged. The Apple backend (src/platform/apple/switch_handler.mm) is the real twin.

#include "maui/core/switch_handler.hpp"

#include <memory>

#include "maui/core/i_switch.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Headless has no native view in the `native` slot, so destruction is trivial.
    switch_platform::~switch_platform() = default;

    std::unique_ptr<switch_platform> switch_handler::create_platform_view()
    {
        return std::make_unique<switch_platform>();
    }

    void switch_handler::on_connect_handler(switch_platform& platform)
    {
        // SwitchProxy.OnControlValueChanged: write the native state back only when it differs (the
        // guard prevents the virtual→native map from echoing back into a second set).
        platform.on_value_changed = [this] {
            auto* view = virtual_view();
            auto* platform_view = typed_platform_view();
            if (view != nullptr && platform_view != nullptr && view->is_on() != platform_view->is_on)
            {
                view->set_is_on(platform_view->is_on);
            }
        };
    }

    void switch_handler::on_disconnect_handler(switch_platform& platform)
    {
        platform.on_value_changed = nullptr;
    }

    void switch_handler::map_is_on(switch_handler& handler, i_switch& view)
    {
        // C# MapIsOn: UpdateIsOn(handler) re-runs the TrackColor mapper (the effective track color
        // depends on the toggle state), then the native state is pushed.
        handler.update_value("track_color");
        if (auto* platform = handler.typed_platform_view())
        {
            platform->is_on = view.is_on();
        }
    }

    void switch_handler::map_track_color(switch_handler& handler, i_switch& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->track_color = view.track_color();
        }
    }

    void switch_handler::map_thumb_color(switch_handler& handler, i_switch& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->thumb_color = view.thumb_color();
        }
    }

    maui::graphics::size switch_handler::get_desired_size(double /*width_constraint*/,
                                                          double /*height_constraint*/) const
    {
        // Headless placeholder metric: the UISwitch natural size (51x31), fixed (the real control does
        // not grow with constraints either).
        return {51.0, 31.0};
    }

    void switch_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native layout to apply.
    }
} // namespace maui::core
