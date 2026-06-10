// stepper_handler — cross-platform part: the shared mapper tables + ctor (StepperHandler.cs). The
// platform recipe (create/connect/disconnect/map_*/measure) lives in the per-backend partial.

#include "maui/core/stepper_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_stepper.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // StepperHandler.Mapper (C# key order): Interval / Maximum / Minimum / Value over
    // ViewHandler.ViewMapper. The key is "increment" — the control's bindable-property name (C# remaps
    // the Controls-layer Increment onto the Core Interval mapping via Stepper.Mapper.cs; keying the
    // table on the property name folds the two-step remap into one entry).
    property_mapper<i_stepper, stepper_handler>& stepper_handler::mapper()
    {
        static property_mapper<i_stepper, stepper_handler> table{view_mapper(),
                                                                 {
                                                                     {"increment", &stepper_handler::map_increment},
                                                                     {"maximum", &stepper_handler::map_maximum},
                                                                     {"minimum", &stepper_handler::map_minimum},
                                                                     {"value", &stepper_handler::map_value},
                                                                 }};
        return table;
    }

    // No stepper-specific commands (C#'s CommandMapper is empty). Qualified return type: the method
    // name `command_mapper` shadows the `command_mapper` template.
    maui::core::command_mapper<i_stepper, stepper_handler>& stepper_handler::command_mapper()
    {
        static maui::core::command_mapper<i_stepper, stepper_handler> table{};
        return table;
    }

    stepper_handler::stepper_handler() : view_handler(&mapper(), &command_mapper())
    {
    }
} // namespace maui::core
