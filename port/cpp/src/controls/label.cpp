// maui::controls::label — out-of-line definitions: the shared bindable-property descriptors and the
// default-handler self-registration. See label.hpp.

#include "maui/controls/label.hpp"

#include <string>

#include "maui/core/bindable_property.hpp"
#include "maui/core/font.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/text_decorations.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<std::string>& label::text_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"text", std::string{}};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& label::text_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"text_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::font>& label::font_property()
    {
        static const maui::core::bindable_property<maui::core::font> descriptor{"font"};
        return descriptor;
    }

    const maui::core::bindable_property<double>& label::character_spacing_property()
    {
        static const maui::core::bindable_property<double> descriptor{"character_spacing", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::thickness>& label::padding_property()
    {
        static const maui::core::bindable_property<maui::core::thickness> descriptor{"padding"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::text_alignment>& label::horizontal_text_alignment_property()
    {
        static const maui::core::bindable_property<maui::core::text_alignment> descriptor{
            "horizontal_text_alignment", maui::core::text_alignment::start};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::text_alignment>& label::vertical_text_alignment_property()
    {
        static const maui::core::bindable_property<maui::core::text_alignment> descriptor{
            "vertical_text_alignment", maui::core::text_alignment::start};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::text_decorations>& label::text_decorations_property()
    {
        static const maui::core::bindable_property<maui::core::text_decorations> descriptor{
            "text_decorations", maui::core::text_decorations::none};
        return descriptor;
    }

    const maui::core::bindable_property<double>& label::line_height_property()
    {
        static const maui::core::bindable_property<double> descriptor{"line_height", -1.0};
        return descriptor;
    }
} // namespace maui::controls

// Self-register the default handler for label (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::label, maui::core::label_handler)
