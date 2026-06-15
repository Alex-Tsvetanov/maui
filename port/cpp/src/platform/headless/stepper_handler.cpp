// stepper_handler — headless platform recipe. A testable stand-in for a native stepper: the mapped
// properties mirror into stepper_platform, and `value` doubles as the native stepper value — a test
// simulates a minus/plus tap by stepping it and invoking on_value_changed (the UIStepper.ValueChanged
// analog), which writes the value back through i_range::set_value exactly like C#'s StepperProxy
// .OnValueChanged. The Apple/iOS .mm partials are the real twins.

#include "maui/core/stepper_handler.hpp"

#include <memory>

#include "maui/core/i_stepper.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Headless has no native view in the `native` slot, so destruction is trivial.
    stepper_platform::~stepper_platform() = default;

    std::unique_ptr<stepper_platform> stepper_handler::create_platform_view()
    {
        return std::make_unique<stepper_platform>();
    }

    void stepper_handler::on_connect_handler(stepper_platform& platform)
    {
        // StepperProxy.OnValueChanged: write the native value back to the virtual view.
        platform.on_value_changed = [this] {
            auto* view = virtual_view();
            auto* platform_view = typed_platform_view();
            if (view != nullptr && platform_view != nullptr)
            {
                view->set_value(platform_view->value);
            }
        };
    }

    void stepper_handler::on_disconnect_handler(stepper_platform& platform)
    {
        platform.on_value_changed = nullptr;
    }

    void stepper_handler::map_increment(stepper_handler& handler, i_stepper& view)
    {
        // StepperExtensions.UpdateIncrement: only a positive increment lands on the native step.
        if (auto* platform = handler.typed_platform_view())
        {
            if (view.interval() > 0)
            {
                platform->increment = view.interval();
            }
        }
    }

    void stepper_handler::map_minimum(stepper_handler& handler, i_stepper& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->minimum = view.minimum();
        }
    }

    void stepper_handler::map_maximum(stepper_handler& handler, i_stepper& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->maximum = view.maximum();
        }
    }

    void stepper_handler::map_value(stepper_handler& handler, i_stepper& view)
    {
        // StepperExtensions.UpdateValue: refresh the native minimum first (a stale higher minimum
        // would make the native control clamp the incoming value), then write when it differs.
        if (auto* platform = handler.typed_platform_view())
        {
            if (platform->minimum != view.minimum())
            {
                platform->minimum = view.minimum();
            }
            if (platform->value != view.value())
            {
                platform->value = view.value();
            }
        }
    }

    void stepper_handler::map_flow_direction(stepper_handler& handler, i_stepper& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            // Headless mirror of StepperHandler.MapFlowDirection: record the RESOLVED direction (the
            // MatchParent → parent-IView fallback) the native steppers push to SemanticContentAttribute.
            platform->resolved_flow_direction = resolved_flow_direction(view);
        }
    }

    maui::graphics::size stepper_handler::get_desired_size(double /*width_constraint*/,
                                                           double /*height_constraint*/) const
    {
        // Headless placeholder metric: the UIStepper natural size (94x32).
        return {94.0, 32.0};
    }

    void stepper_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native layout to apply.
    }
} // namespace maui::core
