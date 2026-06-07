// button_handler — cross-platform part: the shared mapper tables + ctor (ButtonHandler.cs). The
// platform recipe (create/connect/disconnect/map_text/measure) lives in the per-backend partial.

#include "maui/core/button_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_button.hpp"
#include "maui/core/i_text_button.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"

namespace maui::core
{
    // Text + text appearance, keyed by i_text_button (C# TextButtonMapper<ITextButton>).
    property_mapper<i_text_button, button_handler>& button_handler::text_mapper()
    {
        static property_mapper<i_text_button, button_handler> table{
            {"text", &button_handler::map_text},
            {"text_color", &button_handler::map_text_color},
            {"font", &button_handler::map_font},
            {"character_spacing", &button_handler::map_character_spacing},
        };
        return table;
    }

    // The button's own mapper (padding + the i_button_stroke border), chained onto the text mapper —
    // mirroring C# ButtonHandler.Mapper chaining TextButtonMapper. ImageButtonMapper + the shared
    // ViewMapper (the generic IView properties) are deferred to M3/M4.
    property_mapper<i_button, button_handler>& button_handler::mapper()
    {
        static property_mapper<i_button, button_handler> table{
            text_mapper(),
            {
                {"padding", &button_handler::map_padding},
                {"stroke_color", &button_handler::map_stroke_color},
                {"stroke_thickness", &button_handler::map_stroke_thickness},
                {"corner_radius", &button_handler::map_corner_radius},
            },
        };
        return table;
    }

    // No button-specific commands beyond the (currently empty) view command set. The type must be
    // qualified inside the body: the method name `command_mapper` shadows the `command_mapper` template.
    maui::core::command_mapper<i_button, button_handler>& button_handler::command_mapper()
    {
        static maui::core::command_mapper<i_button, button_handler> table{};
        return table;
    }

    button_handler::button_handler() : view_handler(&mapper(), &command_mapper())
    {
    }
} // namespace maui::core
