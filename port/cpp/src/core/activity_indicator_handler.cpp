// activity_indicator_handler — cross-platform part: the shared mapper tables + ctor
// (ActivityIndicatorHandler.cs). The platform recipe (create/map_*/measure) lives in the per-backend
// partial.

#include "maui/core/activity_indicator_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_activity_indicator.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // ActivityIndicatorHandler.Mapper: Color / IsRunning over ViewHandler.ViewMapper — PLUS the
    // "visibility" key remapped onto MapIsRunning (the C# iOS/Android override: "Visibility and
    // IsRunning are dependent on each other, so we handle Visibility explicitly"). The own entry
    // overrides the chained view_mapper's visibility action but keeps its chain position.
    property_mapper<i_activity_indicator, activity_indicator_handler>& activity_indicator_handler::mapper()
    {
        static property_mapper<i_activity_indicator, activity_indicator_handler> table{
            view_mapper(),
            {
                {"color", &activity_indicator_handler::map_color},
                {"is_running", &activity_indicator_handler::map_is_running},
                {"visibility", &activity_indicator_handler::map_is_running},
            }};
        return table;
    }

    // No indicator-specific commands (C#'s CommandMapper is empty). Qualified return type: the method
    // name `command_mapper` shadows the `command_mapper` template.
    maui::core::command_mapper<i_activity_indicator, activity_indicator_handler>& activity_indicator_handler::
        command_mapper()
    {
        static maui::core::command_mapper<i_activity_indicator, activity_indicator_handler> table{};
        return table;
    }

    activity_indicator_handler::activity_indicator_handler() : view_handler(&mapper(), &command_mapper())
    {
    }
} // namespace maui::core
