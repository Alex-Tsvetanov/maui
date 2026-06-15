// stepper_handler — cross-platform part: the shared mapper tables + ctor (StepperHandler.cs). The
// platform recipe (create/connect/disconnect/map_*/measure) lives in the per-backend partial.

#include "maui/core/stepper_handler.hpp"

#include <memory>

#include "maui/core/command_mapper.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/i_element.hpp"
#include "maui/core/i_stepper.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // StepperHandler.Mapper (C# key order): Interval / Maximum / Minimum / Value over
    // ViewHandler.ViewMapper, plus the stepper-specific FlowDirection override
    // (StepperHandler.MapFlowDirection — the UISemanticContentAttribute recipe with the parent fallback).
    // The key is "increment" — the control's bindable-property name (C# remaps the Controls-layer
    // Increment onto the Core Interval mapping via Stepper.Mapper.cs; keying the table on the property
    // name folds the two-step remap into one entry). The "flow_direction" key OVERRIDES the chained
    // view_mapper's generic flow push (the nearer mapper wins, running in the farther's position) —
    // exactly as progress_bar_handler.
    property_mapper<i_stepper, stepper_handler>& stepper_handler::mapper()
    {
        static property_mapper<i_stepper, stepper_handler> table{
            view_mapper(),
            {
                {"increment", &stepper_handler::map_increment},
                {"maximum", &stepper_handler::map_maximum},
                {"minimum", &stepper_handler::map_minimum},
                {"value", &stepper_handler::map_value},
                {"flow_direction", &stepper_handler::map_flow_direction},
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

    // StepperHandler.GetSemanticContentAttribute + GetParentSemanticContentAttribute (collapsed to the
    // flow_direction enum the per-backend map applies natively): an explicit LeftToRight/RightToLeft
    // wins; MatchParent resolves against the parent IView's FlowDirection ((stepper as IView)?.Parent as
    // IView), and stays MatchParent when the parent is absent or is not an IView (C#'s Unspecified).
    // Ported from progress_bar_handler::resolved_flow_direction, substituting i_stepper.
    maui::core::flow_direction stepper_handler::resolved_flow_direction(const i_stepper& view)
    {
        if (view.flow_direction() != maui::core::flow_direction::match_parent)
        {
            return view.flow_direction();
        }
        const std::shared_ptr<i_element> parent = view.parent();
        if (const auto* parent_view = dynamic_cast<const i_view*>(parent.get()))
        {
            return parent_view->flow_direction();
        }
        return maui::core::flow_direction::match_parent;
    }
} // namespace maui::core
