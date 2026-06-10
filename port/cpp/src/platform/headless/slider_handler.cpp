// slider_handler — headless platform recipe. A testable stand-in for a native slider: the mapped
// properties mirror into slider_platform, and `value` doubles as the native thumb position — a test
// simulates a user drag by setting it and invoking on_value_changed (the UISlider.ValueChanged analog),
// with on_drag_started/on_drag_completed standing in for the TouchDown / TouchUp(Inside|Outside)
// control events C#'s SliderProxy wires. The Apple/iOS .mm partials are the real twins.

#include "maui/core/slider_handler.hpp"

#include <memory>

#include "maui/core/i_slider.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Headless has no native view in the `native` slot, so destruction is trivial.
    slider_platform::~slider_platform() = default;

    std::unique_ptr<slider_platform> slider_handler::create_platform_view()
    {
        return std::make_unique<slider_platform>();
    }

    void slider_handler::on_connect_handler(slider_platform& platform)
    {
        // SliderProxy.Connect: ValueChanged → Value write-back; TouchDown → DragStarted;
        // TouchUpInside|TouchUpOutside → DragCompleted.
        platform.on_value_changed = [this] {
            auto* view = virtual_view();
            auto* platform_view = typed_platform_view();
            if (view != nullptr && platform_view != nullptr)
            {
                view->set_value(platform_view->value);
            }
        };
        platform.on_drag_started = [this] {
            if (auto* view = virtual_view())
            {
                view->send_drag_started();
            }
        };
        platform.on_drag_completed = [this] {
            if (auto* view = virtual_view())
            {
                view->send_drag_completed();
            }
        };
    }

    void slider_handler::on_disconnect_handler(slider_platform& platform)
    {
        platform.on_value_changed = nullptr;
        platform.on_drag_started = nullptr;
        platform.on_drag_completed = nullptr;
    }

    void slider_handler::map_minimum(slider_handler& handler, i_slider& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->minimum = view.minimum();
        }
    }

    void slider_handler::map_maximum(slider_handler& handler, i_slider& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->maximum = view.maximum();
        }
    }

    void slider_handler::map_value(slider_handler& handler, i_slider& view)
    {
        // SliderExtensions.UpdateValue: write only when it differs (prevents the native ValueChanged
        // echo from looping).
        if (auto* platform = handler.typed_platform_view())
        {
            if (platform->value != view.value())
            {
                platform->value = view.value();
            }
        }
    }

    void slider_handler::map_minimum_track_color(slider_handler& handler, i_slider& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->minimum_track_color = view.minimum_track_color();
        }
    }

    void slider_handler::map_maximum_track_color(slider_handler& handler, i_slider& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->maximum_track_color = view.maximum_track_color();
        }
    }

    void slider_handler::map_thumb_color(slider_handler& handler, i_slider& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->thumb_color = view.thumb_color();
        }
    }

    maui::graphics::size slider_handler::get_desired_size(double /*width_constraint*/,
                                                          double /*height_constraint*/) const
    {
        // Headless placeholder metric: a nominal track (the UISlider natural height is ~31).
        return {100.0, 31.0};
    }

    void slider_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native layout to apply.
    }
} // namespace maui::core
