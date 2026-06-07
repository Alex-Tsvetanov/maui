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
    // Text-bearing properties, keyed by i_text (C# TextButtonMapper<ITextButton>). M2 first cut maps
    // Text; text_color / font / character_spacing are deferred (documented in STATUS).
    property_mapper<i_text_button, button_handler>& button_handler::text_mapper()
    {
        static property_mapper<i_text_button, button_handler> table{
            {"text", &button_handler::map_text},
        };
        return table;
    }

    // The button's own mapper, chained onto the text mapper (C# ButtonHandler.Mapper chains
    // TextButtonMapper / ImageButtonMapper / ViewMapper). Padding/stroke/background + the view mapper
    // are deferred to later M2/M3 cuts.
    property_mapper<i_button, button_handler>& button_handler::mapper()
    {
        static property_mapper<i_button, button_handler> table{text_mapper()};
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
