// button_handler — cross-platform part: the shared mapper tables + ctor (ButtonHandler.cs). The
// platform recipe (create/connect/disconnect/map_text/measure) lives in the per-backend partial.

#include "maui/core/button_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_button.hpp"
#include "maui/core/i_text_button.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

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

    // The button's own mapper (padding + the i_button_stroke border), chained onto BOTH the shared
    // view_mapper (the generic IView properties) and the text mapper — mirroring C# ButtonHandler.Mapper
    // chaining TextButtonMapper, which ultimately chains ViewHandler.ViewMapper. The chain is ordered so
    // the generic IView keys run first (keys() walks the chain in reverse), then the text keys, then the
    // button's own keys; no keys collide across the three mappers. ImageButtonMapper is still deferred.
    property_mapper<i_button, button_handler>& button_handler::mapper()
    {
        static property_mapper<i_button, button_handler> table = [] {
            property_mapper<i_button, button_handler> mapped{
                {"padding", &button_handler::map_padding},
                {"stroke_color", &button_handler::map_stroke_color},
                {"stroke_thickness", &button_handler::map_stroke_thickness},
                {"corner_radius", &button_handler::map_corner_radius},
            };
            // Reverse-order iteration in keys() means the LAST chained mapper's keys come first, so
            // listing text_mapper then view_mapper yields: view (generic IView) keys, then text keys.
            mapped.set_chained({&text_mapper(), &view_mapper()});
            return mapped;
        }();
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
