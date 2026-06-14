// maui::controls::span — out-of-line definitions: the shared bindable-property descriptors, the gesture
// collection wiring (Span.ValidateGesture + logical-child parenting), and GetEffectiveFont. See span.hpp.

#include "maui/controls/span.hpp"

#include <functional>
#include <stdexcept>
#include <string>

#include "maui/controls/element.hpp"
#include "maui/controls/gestures/gesture_recognizer.hpp"
#include "maui/controls/gestures/gesture_recognizer_collection.hpp"
#include "maui/controls/gestures/tap_gesture_recognizer.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/font.hpp"
#include "maui/core/font_attributes.hpp"
#include "maui/core/text_decorations.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    // C# Span.TextProperty default "".
    const maui::core::bindable_property<std::string>& span::text_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"text", std::string{}};
        return descriptor;
    }

    const maui::core::bindable_property<std::string>& span::font_family_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"font_family", std::string{}};
        return descriptor;
    }

    // C# IFontElement.FontSizeDefaultValueCreator returns NaN; the port uses 0 ("unset"), which the
    // effective-font path treats identically (size <= 0 || NaN both fall back to the default size).
    const maui::core::bindable_property<double>& span::font_size_property()
    {
        static const maui::core::bindable_property<double> descriptor{"font_size", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::font_attributes>& span::font_attributes_property()
    {
        static const maui::core::bindable_property<maui::core::font_attributes> descriptor{
            "font_attributes", maui::core::font_attributes::none};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& span::font_auto_scaling_enabled_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"font_auto_scaling_enabled", true};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& span::text_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"text_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& span::background_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"background_color"};
        return descriptor;
    }

    const maui::core::bindable_property<double>& span::character_spacing_property()
    {
        static const maui::core::bindable_property<double> descriptor{"character_spacing", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::text_decorations>& span::text_decorations_property()
    {
        static const maui::core::bindable_property<maui::core::text_decorations> descriptor{
            "text_decorations", maui::core::text_decorations::none};
        return descriptor;
    }

    // C# LineHeightElement default -1 (the "use the default" sentinel).
    const maui::core::bindable_property<double>& span::line_height_property()
    {
        static const maui::core::bindable_property<double> descriptor{"line_height", -1.0};
        return descriptor;
    }

    span::span()
        : gesture_recognizers_{
              {.attach = [this](gesture_recognizer& recognizer) { this->attach_logical_child(recognizer); },
               .detach = [](gesture_recognizer& recognizer) { element::detach_logical_child(recognizer); },
               // A span has no native handler — a recognizer change only re-raises Spans so the owning
               // formatted_string / label re-syncs its composite recognizers (no platform load here).
               .changed = [this] { this->on_property_changed("gesture_recognizers"); },
               .validate = [](const gesture_recognizer& recognizer) { span::validate_gesture(recognizer); }}}
    {
        // Span.StyleProperty / MergedStyle(GetType(), this): a Style (implicit/class or explicit) targeting
        // `span` matches this element (SpanTests.StyleApplied resolves Span.TextColorProperty via the style).
        this->set_style_target_type<span>();
    }

    void span::validate_gesture(const gesture_recognizer& recognizer)
    {
        // Span.ValidateGesture: only a TapGestureRecognizer is supported (else InvalidOperationException).
        if (dynamic_cast<const tap_gesture_recognizer*>(&recognizer) == nullptr)
        {
            throw std::runtime_error("only a tap_gesture_recognizer is supported on a span");
        }
    }

    void span::for_each_logical_child(const std::function<void(element&)>& visit) const
    {
        for (const auto& recognizer : gesture_recognizers_.items())
        {
            if (recognizer)
            {
                visit(*recognizer);
            }
        }
    }

    maui::core::font span::get_effective_font(double default_font_size, const maui::core::font& default_font) const
    {
        // C# FontExtensions.GetEffectiveFont — each field uses the span value when IsSet, else the default.
        const std::string family = is_font_family_set() ? std::string(font_family()) : default_font.family();
        const double size = is_font_size_set() ? font_size() : default_font_size;
        const maui::core::font_attributes attributes =
            is_font_attributes_set() ? font_attributes() : maui::core::attributes_of(default_font);
        const bool auto_scaling =
            is_font_auto_scaling_enabled_set() ? font_auto_scaling_enabled() : default_font.auto_scaling_enabled();

        return maui::core::with_attributes(maui::core::font::of_size(family, size, maui::core::font_weight::regular,
                                                                     maui::core::font_slant::normal, auto_scaling),
                                           attributes);
    }
} // namespace maui::controls
