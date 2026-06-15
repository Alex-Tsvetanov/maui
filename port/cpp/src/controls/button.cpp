// maui::controls::button — out-of-line definitions: the shared bindable-property descriptors (one
// instance per type, like Button.*Property) and the default-handler self-registration. See button.hpp.

#include "maui/controls/button.hpp"

#include <memory>
#include <string>

#include "maui/controls/button_content_layout.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/font.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<std::string>& button::text_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"text", std::string{}};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& button::text_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"text_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::font>& button::font_property()
    {
        static const maui::core::bindable_property<maui::core::font> descriptor{"font"};
        return descriptor;
    }

    const maui::core::bindable_property<double>& button::character_spacing_property()
    {
        static const maui::core::bindable_property<double> descriptor{"character_spacing", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::thickness>& button::padding_property()
    {
        static const maui::core::bindable_property<maui::core::thickness> descriptor{"padding"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& button::stroke_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"stroke_color"};
        return descriptor;
    }

    const maui::core::bindable_property<double>& button::stroke_thickness_property()
    {
        static const maui::core::bindable_property<double> descriptor{"stroke_thickness", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<int>& button::corner_radius_property()
    {
        static const maui::core::bindable_property<int> descriptor{"corner_radius", 0};
        return descriptor;
    }

    const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>>& button::image_source_property()
    {
        // C# Button.ImageSourceProperty = ImageElement.ImageSourceProperty, default null. The key "source"
        // matches button_handler's ImageButtonMapper entry ([Source] → map_image_source).
        static const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>> descriptor{"source"};
        return descriptor;
    }

    const maui::core::bindable_property<button_content_layout>& button::content_layout_property()
    {
        // C# Button.ContentLayoutProperty default: new ButtonContentLayout(ImagePosition.Left, DefaultSpacing).
        // The key "content_layout" matches button_handler's map_content_layout entry (stored + pushed; the
        // text+image composition is deferred — no container infra).
        static const maui::core::bindable_property<button_content_layout> descriptor{
            "content_layout",
            button_content_layout{button_content_layout::image_position::left, button_content_layout::default_spacing}};
        return descriptor;
    }
} // namespace maui::controls

// Self-register the default handler for button (opt-in, PROFILE §6). This TU is always linked (button's
// out-of-line members above are referenced by every user of the control), so the registrar runs.
MAUI_REGISTER_HANDLER(maui::controls::button, maui::core::button_handler)
