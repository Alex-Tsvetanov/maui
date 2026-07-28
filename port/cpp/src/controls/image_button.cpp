// maui::controls::image_button — out-of-line definitions: the shared bindable-property descriptors (one
// instance per type, like ImageButton.*Property) and the default-handler self-registration. See
// image_button.hpp. Defaults mirror ImageButton/ImageElement/ButtonElement: Aspect.AspectFit, null
// Source, IsOpaque false, IsLoading false (read-only), zero Padding, default stroke color, BorderWidth
// 0, corner_radius 0 (the port button's convention — C#'s -1 "platform default" sentinel documented in
// the header).

#include "maui/controls/image_button.hpp"

#include <memory>

#include "maui/core/aspect.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/image_button_handler.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<maui::core::aspect>& image_button::aspect_property()
    {
        // C# ImageElement.AspectProperty default: Aspect.AspectFit.
        static const maui::core::bindable_property<maui::core::aspect> descriptor{"aspect",
                                                                                  maui::core::aspect::aspect_fit};
        return descriptor;
    }

    const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>>& image_button::source_property()
    {
        // C# ImageElement.SourceProperty default: null.
        static const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>> descriptor{"source"};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& image_button::is_opaque_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"is_opaque", false};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& image_button::is_loading_property()
    {
        // Read-only in C# (IsLoadingPropertyKey); the port has no public setter — update_is_loading
        // (the loader's channel) is the only writer.
        static const maui::core::bindable_property<bool> descriptor{"is_loading", false};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::thickness>& image_button::padding_property()
    {
        static const maui::core::bindable_property<maui::core::thickness> descriptor{"padding",
                                                                                     maui::core::thickness{}};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& image_button::stroke_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"stroke_color"};
        return descriptor;
    }

    const maui::core::bindable_property<double>& image_button::stroke_thickness_property()
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

    const maui::core::bindable_property<int>& image_button::corner_radius_property()
    {
        // -1 (BorderElement.DefaultCornerRadius), NOT 0 - same sentinel argument as stroke_thickness
        // above: 0 is a legitimate explicit "square corners" request and must be distinguishable from
        // "unset". On WinUI the 0 default squared off every themed button.
        static const maui::core::bindable_property<int> descriptor{"corner_radius", -1};
        return descriptor;
    }
} // namespace maui::controls

// Self-register the default handler for image_button (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::image_button, maui::core::image_button_handler)
