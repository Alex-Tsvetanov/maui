// check_box_handler — cross-platform part: the shared mapper tables + ctor (CheckBoxHandler.cs). The
// platform recipe (create/connect/disconnect/map_*/measure) lives in the per-backend partial.

#include "maui/core/check_box_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_check_box.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // CheckBoxHandler.Mapper: IsChecked / Foreground over ViewHandler.ViewMapper (the chained shared
    // view_mapper supplies the generic IView keys; the Android-only Background override is N/A here).
    property_mapper<i_check_box, check_box_handler>& check_box_handler::mapper()
    {
        static property_mapper<i_check_box, check_box_handler> table{
            view_mapper(),
            {
                {"is_checked", &check_box_handler::map_is_checked},
                {"foreground", &check_box_handler::map_foreground},
            }};
        return table;
    }

    // No check-box-specific commands (C#'s CommandMapper is empty). Qualified return type: the method
    // name `command_mapper` shadows the `command_mapper` template.
    maui::core::command_mapper<i_check_box, check_box_handler>& check_box_handler::command_mapper()
    {
        static maui::core::command_mapper<i_check_box, check_box_handler> table{};
        return table;
    }

    check_box_handler::check_box_handler() : view_handler(&mapper(), &command_mapper())
    {
    }
} // namespace maui::core
