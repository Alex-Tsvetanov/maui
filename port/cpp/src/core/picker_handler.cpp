// picker_handler — cross-platform part: the shared mapper table + ctor (PickerHandler.cs). The
// platform recipe (create/connect/disconnect/map/measure) lives in the per-backend partial.

#include "maui/core/picker_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_picker.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // Mirrors PickerHandler.Mapper (character_spacing/font/is_open/selected_index/text_color/title/
    // title_color/alignments/items), chained onto the shared view_mapper. The Android/Windows-only
    // Background and IsEnabled remaps stay platform-specific in C# and are not replicated. The control
    // re-runs the "items" key on every collection change (Handler?.UpdateValue(nameof(IPicker.Items))).
    property_mapper<i_picker, picker_handler>& picker_handler::mapper()
    {
        static property_mapper<i_picker, picker_handler> table{
            view_mapper(),
            {
                {"character_spacing", &picker_handler::map_character_spacing},
                {"font", &picker_handler::map_font},
                {"is_open", &picker_handler::map_is_open},
                {"selected_index", &picker_handler::map_selected_index},
                {"text_color", &picker_handler::map_text_color},
                {"title", &picker_handler::map_title},
                {"title_color", &picker_handler::map_title_color},
                {"horizontal_text_alignment", &picker_handler::map_horizontal_text_alignment},
                {"vertical_text_alignment", &picker_handler::map_vertical_text_alignment},
                {"items", &picker_handler::map_items},
            },
        };
        return table;
    }

    // The type must be qualified inside the body: the method name `command_mapper` shadows the template.
    maui::core::command_mapper<i_picker, picker_handler>& picker_handler::command_mapper()
    {
        static maui::core::command_mapper<i_picker, picker_handler> table{};
        return table;
    }

    picker_handler::picker_handler() : view_handler(&mapper(), &command_mapper())
    {
    }
} // namespace maui::core
