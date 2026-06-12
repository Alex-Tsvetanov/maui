// date_picker_handler — cross-platform part: the shared mapper table + ctor (DatePickerHandler.cs).
// The platform recipe (create/connect/disconnect/map/measure) lives in the per-backend partial.

#include "maui/core/date_picker_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_date_picker.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // Mirrors DatePickerHandler.Mapper (character_spacing/date/font/format/maximum_date/minimum_date/
    // text_color), chained onto the shared view_mapper. The Android/Windows-only Background remap and
    // the iOS FlowDirection+TextAlignment remap stay platform-specific in C# and are not replicated
    // (the shared view_mapper carries flow_direction); IsOpen is deferred with the focus subsystem
    // (see i_date_picker.hpp).
    property_mapper<i_date_picker, date_picker_handler>& date_picker_handler::mapper()
    {
        static property_mapper<i_date_picker, date_picker_handler> table{
            view_mapper(),
            {
                {"character_spacing", &date_picker_handler::map_character_spacing},
                {"date", &date_picker_handler::map_date},
                {"font", &date_picker_handler::map_font},
                {"format", &date_picker_handler::map_format},
                {"maximum_date", &date_picker_handler::map_maximum_date},
                {"minimum_date", &date_picker_handler::map_minimum_date},
                {"text_color", &date_picker_handler::map_text_color},
            },
        };
        return table;
    }

    // The type must be qualified inside the body: the method name `command_mapper` shadows the template.
    maui::core::command_mapper<i_date_picker, date_picker_handler>& date_picker_handler::command_mapper()
    {
        static maui::core::command_mapper<i_date_picker, date_picker_handler> table{};
        return table;
    }

    date_picker_handler::date_picker_handler() : view_handler(&mapper(), &command_mapper())
    {
    }
} // namespace maui::core
