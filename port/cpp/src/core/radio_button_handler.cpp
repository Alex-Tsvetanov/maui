// radio_button_handler — cross-platform part: the shared mapper tables + ctor (RadioButtonHandler.cs).
// The platform recipe (create/connect/disconnect/map/measure) lives in the per-backend partial.

#include "maui/core/radio_button_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_radio_button.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // RadioButtonHandler.Mapper over ViewHandler.ViewMapper. The keys match the radio_button control's
    // bindable-property names exactly (the nameof(IRadioButton.*) convention); Background rides the
    // chained view_mapper instead of a dedicated key (see the header's deviation note).
    property_mapper<i_radio_button, radio_button_handler>& radio_button_handler::mapper()
    {
        static property_mapper<i_radio_button, radio_button_handler> table{
            view_mapper(),
            {
                {"is_checked", &radio_button_handler::map_is_checked},
                {"character_spacing", &radio_button_handler::map_character_spacing},
                {"font", &radio_button_handler::map_font},
                {"text_color", &radio_button_handler::map_text_color},
                {"content", &radio_button_handler::map_content},
                {"stroke_color", &radio_button_handler::map_stroke_color},
                {"stroke_thickness", &radio_button_handler::map_stroke_thickness},
                {"corner_radius", &radio_button_handler::map_corner_radius},
            },
        };
        return table;
    }

    // The type must be qualified inside the body: the method name `command_mapper` shadows the template.
    maui::core::command_mapper<i_radio_button, radio_button_handler>& radio_button_handler::command_mapper()
    {
        static maui::core::command_mapper<i_radio_button, radio_button_handler> table{};
        return table;
    }

    radio_button_handler::radio_button_handler() : view_handler(&mapper(), &command_mapper())
    {
    }
} // namespace maui::core
