// maui::controls::button — out-of-line definitions: the shared bindable-property descriptors (one
// instance per type, like Button.*Property) and the default-handler self-registration. See button.hpp.

#include "maui/controls/button.hpp"

#include <limits>
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
        // C# Button.PaddingDefaultValueCreator() => new Thickness(double.NaN) — UNLIKE ImageButton
        // (default(Thickness)). The NaN default is what makes ButtonHandler.iOS.MapPadding substitute
        // DefaultPadding(12,7): UpdatePadding sees Padding.IsNaN and falls back to the native-default
        // content insets. A zero (default-constructed) thickness here would be is_nan()==false, so the
        // handler would set contentEdgeInsets to ~0 and the UIButton would collapse to bare glyph width
        // (the `clipping` page's crammed digit row). The uniform-double ctor sets all four to NaN, the
        // exact mirror of C#'s `new Thickness(double.NaN)`.
        static const maui::core::bindable_property<maui::core::thickness> descriptor{
            "padding", maui::core::thickness{std::numeric_limits<double>::quiet_NaN()}};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& button::stroke_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"stroke_color"};
        return descriptor;
    }

    const maui::core::bindable_property<double>& button::stroke_thickness_property()
    {
        // -1, NOT 0: C#'s BorderElement.BorderWidthProperty defaults to -1d (a "not set" SENTINEL), and
        // every handler's UpdateStrokeThickness keys off `>= 0` to decide whether to override the native
        // default at all. With 0 here the port pushed an EXPLICIT zero border on every button - invisible
        // on iOS/Android (their native default is already 0) but on WinUI it erased the theme's button
        // border outright. An explicit BorderWidth=0 must still mean "no border"; that is exactly why the
        // unset value has to be a distinct sentinel.
        static const maui::core::bindable_property<double> descriptor{"stroke_thickness", -1.0};
        return descriptor;
    }

    const maui::core::bindable_property<int>& button::corner_radius_property()
    {
        // -1 (BorderElement.DefaultCornerRadius), NOT 0 - same sentinel argument as stroke_thickness
        // above: 0 is a legitimate explicit "square corners" request and must be distinguishable from
        // "unset". On WinUI the 0 default squared off every themed button.
        static const maui::core::bindable_property<int> descriptor{"corner_radius", -1};
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
