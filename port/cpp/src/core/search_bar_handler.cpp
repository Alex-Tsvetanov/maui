// search_bar_handler — cross-platform part: the shared mapper table + ctor (SearchBarHandler.cs). The
// platform recipe (create/connect/disconnect/map/measure) lives in the per-backend partial.

#include "maui/core/search_bar_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_search_bar.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // Keyed on i_search_bar. Chained onto the shared view_mapper so the generic IView properties map
    // first (keys() walks the chain first). Mirrors SearchBarHandler.Mapper (character_spacing/font/
    // alignments/is_read_only/prediction/spellcheck/max_length/placeholder(+color)/text(+color)/cursor/
    // selection/cancel_button_color/search_icon_color/return_type); MapBackground rides the shared
    // view_mapper, Keyboard stays out of scope, and the platform-only IsEnabled/FlowDirection overrides
    // collapse into the shared pushes.
    property_mapper<i_search_bar, search_bar_handler>& search_bar_handler::mapper()
    {
        static property_mapper<i_search_bar, search_bar_handler> table{
            view_mapper(),
            {
                {"text", &search_bar_handler::map_text},
                {"placeholder", &search_bar_handler::map_placeholder},
                {"placeholder_color", &search_bar_handler::map_placeholder_color},
                {"is_read_only", &search_bar_handler::map_is_read_only},
                {"max_length", &search_bar_handler::map_max_length},
                {"text_color", &search_bar_handler::map_text_color},
                {"font", &search_bar_handler::map_font},
                {"character_spacing", &search_bar_handler::map_character_spacing},
                {"horizontal_text_alignment", &search_bar_handler::map_horizontal_text_alignment},
                {"vertical_text_alignment", &search_bar_handler::map_vertical_text_alignment},
                {"is_text_prediction_enabled", &search_bar_handler::map_is_text_prediction_enabled},
                {"is_spell_check_enabled", &search_bar_handler::map_is_spell_check_enabled},
                {"cursor_position", &search_bar_handler::map_cursor_position},
                {"selection_length", &search_bar_handler::map_selection_length},
                {"cancel_button_color", &search_bar_handler::map_cancel_button_color},
                {"search_icon_color", &search_bar_handler::map_search_icon_color},
                {"return_type", &search_bar_handler::map_return_type},
            },
        };
        return table;
    }

    // The type must be qualified inside the body: the method name `command_mapper` shadows the template.
    maui::core::command_mapper<i_search_bar, search_bar_handler>& search_bar_handler::command_mapper()
    {
        static maui::core::command_mapper<i_search_bar, search_bar_handler> table{};
        return table;
    }

    search_bar_handler::search_bar_handler() : view_handler(&mapper(), &command_mapper())
    {
    }
} // namespace maui::core
