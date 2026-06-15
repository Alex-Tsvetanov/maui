// entry_handler — cross-platform part: the shared mapper table + ctor (EntryHandler.cs). The platform
// recipe (create/connect/disconnect/map/measure) lives in the per-backend partial.

#include "maui/core/entry_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_entry.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_command_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // Keyed on i_entry, which exposes text + placeholder + the value flags + i_text_style appearance +
    // i_text_alignment directly. Chained onto the shared view_mapper so the generic IView properties
    // (Visibility/Opacity/IsEnabled/AutomationId) map first (keys() walks the chain first). Mirrors
    // EntryHandler.Mapper (text/placeholder/is_password/is_read_only/max_length/alignment/text_color/font/
    // character_spacing/keyboard/return_type/clear_button_visibility/prediction/spellcheck/cursor/selection).
    property_mapper<i_entry, entry_handler>& entry_handler::mapper()
    {
        static property_mapper<i_entry, entry_handler> table{
            view_mapper(),
            {
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
                {"is_text_prediction_enabled", &entry_handler::map_is_text_prediction_enabled},
                {"is_spell_check_enabled", &entry_handler::map_is_spell_check_enabled},
                // Keyboard maps AFTER prediction/spellcheck (C# EntryHandler.Mapper order): for a custom
                // keyboard its flags must win over the standalone prediction/spellcheck pushes.
                {"keyboard", &entry_handler::map_keyboard},
                {"return_type", &entry_handler::map_return_type},
                {"clear_button_visibility", &entry_handler::map_clear_button_visibility},
                {"cursor_position", &entry_handler::map_cursor_position},
                {"selection_length", &entry_handler::map_selection_length},
                // --- platform configuration (W2-24): C# appends this mapping from the Controls layer
                // (Entry.Mapper.cs ReplaceMapping(CursorColorProperty.PropertyName, MapCursorColor),
                // iOS only); the port's table is core-owned, so the key (the namespaced knob name the
                // store raises) lives here and the per-backend body reads the i_ios_entry_specifics face.
                {"ios.Entry.CursorColor", &entry_handler::map_cursor_color},
            },
        };
        return table;
    }

    // The type must be qualified inside the body: the method name `command_mapper` shadows the template.
    // Chained onto the shared view_command_mapper (Focus / Unfocus) — the entry adds no command of its own
    // (C# EntryHandler.CommandMapper chains ViewCommandMapper and only re-keys Focus on platforms with a
    // bespoke focus path, which the port handles uniformly through the chained base).
    maui::core::command_mapper<i_entry, entry_handler>& entry_handler::command_mapper()
    {
        static maui::core::command_mapper<i_entry, entry_handler> table{view_command_mapper(), {}};
        return table;
    }

    entry_handler::entry_handler() : view_handler(&mapper(), &command_mapper())
    {
    }
} // namespace maui::core
