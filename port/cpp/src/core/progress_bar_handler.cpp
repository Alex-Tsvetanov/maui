// progress_bar_handler — cross-platform part: the shared mapper tables + ctor (ProgressBarHandler.cs).
// The platform recipe (create/map_*/measure) lives in the per-backend partial.

#include "maui/core/progress_bar_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_progress.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // ProgressBarHandler.Mapper: Progress / ProgressColor over ViewHandler.ViewMapper (the iOS
    // FlowDirection override is deferred — see the header).
    property_mapper<i_progress, progress_bar_handler>& progress_bar_handler::mapper()
    {
        static property_mapper<i_progress, progress_bar_handler> table{
            view_mapper(),
            {
                {"progress", &progress_bar_handler::map_progress},
                {"progress_color", &progress_bar_handler::map_progress_color},
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
} // namespace maui::core
