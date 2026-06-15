// progress_bar_handler — cross-platform part: the shared mapper tables + ctor (ProgressBarHandler.cs).
// The platform recipe (create/map_*/measure) lives in the per-backend partial.

#include "maui/core/progress_bar_handler.hpp"

#include <memory>

#include "maui/core/command_mapper.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/i_element.hpp"
#include "maui/core/i_progress.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // ProgressBarHandler.Mapper: Progress / ProgressColor over ViewHandler.ViewMapper, plus the
    // bar-specific FlowDirection override (ProgressBarHandler.MapFlowDirection — the
    // UISemanticContentAttribute recipe with the parent fallback). The "flow_direction" key OVERRIDES the
    // chained view_mapper's generic flow push (the nearer mapper wins, running in the farther's position).
    property_mapper<i_progress, progress_bar_handler>& progress_bar_handler::mapper()
    {
        static property_mapper<i_progress, progress_bar_handler> table{
            view_mapper(),
            {
                {"progress", &progress_bar_handler::map_progress},
                {"progress_color", &progress_bar_handler::map_progress_color},
                {"flow_direction", &progress_bar_handler::map_flow_direction},
            }};
        return table;
    }

    // No progress-bar-specific commands (C#'s CommandMapper is empty). Qualified return type: the
    // method name `command_mapper` shadows the `command_mapper` template.
    maui::core::command_mapper<i_progress, progress_bar_handler>& progress_bar_handler::command_mapper()
    {
        static maui::core::command_mapper<i_progress, progress_bar_handler> table{};
        return table;
    }

    progress_bar_handler::progress_bar_handler() : view_handler(&mapper(), &command_mapper())
    {
    }

    // ProgressBarHandler.GetSemanticContentAttribute + GetParentSemanticContentAttribute (collapsed to the
    // flow_direction enum the per-backend map applies natively): an explicit LeftToRight/RightToLeft wins;
    // MatchParent resolves against the parent IView's FlowDirection ((progress as IView)?.Parent as IView),
    // and stays MatchParent when the parent is absent or is not an IView (C#'s Unspecified).
    maui::core::flow_direction progress_bar_handler::resolved_flow_direction(const i_progress& view)
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
