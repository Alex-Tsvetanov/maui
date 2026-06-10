// switch_handler — cross-platform part: the shared mapper tables + ctor (SwitchHandler.cs). The
// platform recipe (create/connect/disconnect/map_*/measure) lives in the per-backend partial.

#include "maui/core/switch_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_switch.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // SwitchHandler.Mapper: IsOn / ThumbColor / TrackColor over ViewHandler.ViewMapper (the chained
    // shared view_mapper supplies the generic IView keys; no keys collide).
    property_mapper<i_switch, switch_handler>& switch_handler::mapper()
    {
        static property_mapper<i_switch, switch_handler> table{view_mapper(),
                                                               {
                                                                   {"is_on", &switch_handler::map_is_on},
                                                                   {"thumb_color", &switch_handler::map_thumb_color},
                                                                   {"track_color", &switch_handler::map_track_color},
                                                               }};
        return table;
    }

    // No switch-specific commands (C#'s CommandMapper is empty). Qualified return type: the method name
    // `command_mapper` shadows the `command_mapper` template.
    maui::core::command_mapper<i_switch, switch_handler>& switch_handler::command_mapper()
    {
        static maui::core::command_mapper<i_switch, switch_handler> table{};
        return table;
    }

    switch_handler::switch_handler() : view_handler(&mapper(), &command_mapper())
    {
    }
} // namespace maui::core
