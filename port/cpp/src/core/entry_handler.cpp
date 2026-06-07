// entry_handler — cross-platform part: the shared mapper table + ctor (EntryHandler.cs). The platform
// recipe (create/connect/disconnect/map/measure) lives in the per-backend partial.

#include "maui/core/entry_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_entry.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"

namespace maui::core
{
    // Keyed on i_entry, which exposes text + placeholder + the value flags + i_text_style appearance +
    // i_text_alignment directly, so no chained mapper is needed (cf. label). Mirrors EntryHandler.Mapper
    // (text/placeholder/is_password/is_read_only/max_length/alignment/text_color/font/character_spacing);
    // ReturnType / ClearButtonVisibility / keyboard / prediction are out of scope this cut.
    property_mapper<i_entry, entry_handler>& entry_handler::mapper()
    {
        static property_mapper<i_entry, entry_handler> table{
            {"text", &entry_handler::map_text},
            {"placeholder", &entry_handler::map_placeholder},
            {"placeholder_color", &entry_handler::map_placeholder_color},
            {"is_password", &entry_handler::map_is_password},
            {"is_read_only", &entry_handler::map_is_read_only},
            {"max_length", &entry_handler::map_max_length},
            {"text_color", &entry_handler::map_text_color},
            {"font", &entry_handler::map_font},
            {"character_spacing", &entry_handler::map_character_spacing},
            {"horizontal_text_alignment", &entry_handler::map_horizontal_text_alignment},
            {"vertical_text_alignment", &entry_handler::map_vertical_text_alignment},
        };
        return table;
    }

    // The type must be qualified inside the body: the method name `command_mapper` shadows the template.
    maui::core::command_mapper<i_entry, entry_handler>& entry_handler::command_mapper()
    {
        static maui::core::command_mapper<i_entry, entry_handler> table{};
        return table;
    }

    entry_handler::entry_handler() : view_handler(&mapper(), &command_mapper())
    {
    }
} // namespace maui::core
