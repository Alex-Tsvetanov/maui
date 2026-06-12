// time_picker_handler — cross-platform part: the shared mapper table + ctor (TimePickerHandler.cs).
// The platform recipe (create/connect/disconnect/map/measure) lives in the per-backend partial.

#include "maui/core/time_picker_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_time_picker.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // Mirrors TimePickerHandler.Mapper (character_spacing/font/format/text_color/time), chained onto
    // the shared view_mapper. The Android/Windows-only Background remap and the iOS/Android
    // FlowDirection+TextAlignment remap stay platform-specific in C# and are not replicated (the
    // shared view_mapper carries flow_direction); IsOpen is deferred with the focus subsystem (see
    // i_time_picker.hpp).
    property_mapper<i_time_picker, time_picker_handler>& time_picker_handler::mapper()
    {
        static property_mapper<i_time_picker, time_picker_handler> table{
            view_mapper(),
            {
                {"character_spacing", &time_picker_handler::map_character_spacing},
                {"font", &time_picker_handler::map_font},
                {"format", &time_picker_handler::map_format},
                {"text_color", &time_picker_handler::map_text_color},
                {"time", &time_picker_handler::map_time},
            },
        };
        return table;
    }

    // The type must be qualified inside the body: the method name `command_mapper` shadows the template.
    maui::core::command_mapper<i_time_picker, time_picker_handler>& time_picker_handler::command_mapper()
    {
        static maui::core::command_mapper<i_time_picker, time_picker_handler> table{};
        return table;
    }

    time_picker_handler::time_picker_handler() : view_handler(&mapper(), &command_mapper())
    {
    }
} // namespace maui::core
