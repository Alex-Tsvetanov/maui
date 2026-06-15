// editor_handler — cross-platform part: the shared mapper table + ctor (EditorHandler.cs). The platform
// recipe (create/connect/disconnect/map/measure) lives in the per-backend partial.

#include "maui/core/editor_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_editor.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_command_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // Keyed on i_editor, which exposes text + placeholder + the value flags + i_text_style appearance +
    // i_text_alignment directly. Chained onto the shared view_mapper so the generic IView properties
    // (Visibility/Opacity/IsEnabled/AutomationId/Background/…) map first (keys() walks the chain first).
    // Mirrors EditorHandler.Mapper (character_spacing/font/is_read_only/prediction/spellcheck/max_length/
    // placeholder/placeholder_color/text/text_color/alignments/keyboard/cursor/selection); MapBackground
    // rides the shared view_mapper, and the iOS-only MapIsEnabled override collapses into the shared
    // is_enabled push.
    property_mapper<i_editor, editor_handler>& editor_handler::mapper()
    {
        static property_mapper<i_editor, editor_handler> table{
            view_mapper(),
            {
                {"text", &editor_handler::map_text},
                {"placeholder", &editor_handler::map_placeholder},
                {"placeholder_color", &editor_handler::map_placeholder_color},
                {"is_read_only", &editor_handler::map_is_read_only},
                {"max_length", &editor_handler::map_max_length},
                {"text_color", &editor_handler::map_text_color},
                {"font", &editor_handler::map_font},
                {"character_spacing", &editor_handler::map_character_spacing},
                {"horizontal_text_alignment", &editor_handler::map_horizontal_text_alignment},
                {"vertical_text_alignment", &editor_handler::map_vertical_text_alignment},
                {"is_text_prediction_enabled", &editor_handler::map_is_text_prediction_enabled},
                {"is_spell_check_enabled", &editor_handler::map_is_spell_check_enabled},
                // Keyboard maps AFTER prediction/spellcheck (C# EditorHandler.Mapper order): a custom
                // keyboard's flags must win over the standalone prediction/spellcheck pushes.
                {"keyboard", &editor_handler::map_keyboard},
                {"cursor_position", &editor_handler::map_cursor_position},
                {"selection_length", &editor_handler::map_selection_length},
            },
        };
        return table;
    }

    // The type must be qualified inside the body: the method name `command_mapper` shadows the template.
    // Chained onto the shared view_command_mapper (Focus / Unfocus); the editor adds no command of its own.
    maui::core::command_mapper<i_editor, editor_handler>& editor_handler::command_mapper()
    {
        static maui::core::command_mapper<i_editor, editor_handler> table{view_command_mapper(), {}};
        return table;
    }

    editor_handler::editor_handler() : view_handler(&mapper(), &command_mapper())
    {
    }
} // namespace maui::core
