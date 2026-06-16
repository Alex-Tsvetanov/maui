// search_bar_handler — cross-platform part: the shared mapper table + ctor (SearchBarHandler.cs). The
// platform recipe (create/connect/disconnect/map/measure) lives in the per-backend partial.

#include "maui/core/search_bar_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_search_bar.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_command_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"
#include "maui/platform/ios/hide_soft_input_on_tapped_manager.hpp" // is_focused → HideSoftInputOnTapped routing

namespace maui::core
{
    // Keyed on i_search_bar. Chained onto the shared view_mapper so the generic IView properties map
    // first (keys() walks the chain first). Mirrors SearchBarHandler.Mapper (character_spacing/font/
    // alignments/is_read_only/prediction/spellcheck/max_length/placeholder(+color)/text(+color)/keyboard/
    // cursor/selection/cancel_button_color/search_icon_color/return_type); MapBackground rides the shared
    // view_mapper, and the platform-only IsEnabled/FlowDirection overrides collapse into the shared pushes.
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
                // Keyboard maps AFTER prediction/spellcheck (C# SearchBarHandler.Mapper order): a custom
                // keyboard's flags must win over the standalone prediction/spellcheck pushes.
                {"keyboard", &search_bar_handler::map_keyboard},
                {"cursor_position", &search_bar_handler::map_cursor_position},
                {"selection_length", &search_bar_handler::map_selection_length},
                {"cancel_button_color", &search_bar_handler::map_cancel_button_color},
                {"search_icon_color", &search_bar_handler::map_search_icon_color},
                {"return_type", &search_bar_handler::map_return_type},
                // C# SearchBar.Mapper.cs: AppendToMapping(nameof(IsFocused), InputView.MapIsFocused) — an
                // InputView focus change arms/disarms the page's HideSoftInputOnTapped tap gesture. The
                // funnel (view::set_is_focused) calls update_value("is_focused") which fires this.
                {"is_focused", [](search_bar_handler& /*handler*/,
                                  i_search_bar& view) { maui::platform::ios::route_input_view_focus(view); }},
            },
        };
        return table;
    }

    // The type must be qualified inside the body: the method name `command_mapper` shadows the template.
    // Chained onto the shared view_command_mapper (Focus / Unfocus); the search bar adds no command of
    // its own (C# SearchBarHandler.CommandMapper re-keys Focus on Android only — the port handles focus
    // uniformly through the chained base).
    maui::core::command_mapper<i_search_bar, search_bar_handler>& search_bar_handler::command_mapper()
    {
        static maui::core::command_mapper<i_search_bar, search_bar_handler> table{view_command_mapper(), {}};
        return table;
    }

    search_bar_handler::search_bar_handler() : view_handler(&mapper(), &command_mapper())
    {
    }
} // namespace maui::core
