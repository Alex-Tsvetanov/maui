#pragma once
// maui::controls::span  <=  Microsoft.Maui.Controls.Span
//
// One attributed portion of a formatted_string's text. In C# Span derives GestureElement and implements
// IFontElement / ITextElement / IDecorableTextElement / ILineHeightElement (+ IStyleElement). The port's
// element base already carries the bindable-property change-notification surface (each property<T> change
// raises bindable_object::property_changed by name — the INotifyPropertyChanged role IFontElement etc.
// rely on), the logical-parent + inherited BindingContext, the SetBinding channel (SpanTests.BindingApplied),
// and merged-style resolution via set_style_target_type (SpanTests.StyleApplied). So a span IS-A element.
//
// Carried per the C# Span surface:
//   - Text                                 (ContentProperty; default "")
//   - FontFamily / FontSize / FontAttributes / FontAutoScalingEnabled   (IFontElement)
//   - TextColor / BackgroundColor / CharacterSpacing                    (ITextElement; +BackgroundColor)
//   - TextDecorations                                                   (IDecorableTextElement)
//   - LineHeight                                                        (ILineHeightElement)
//   - GestureRecognizers (the span tap surface)                         (GestureElement)
//
// Surface deviations (documented, narrow):
//   - TextTransform / Style / UpdateFormsText are NOT ported here: TextTransform's transform pipeline +
//     the per-span Style object are markup-era niceties the headless/apple attributed-string build does
//     not consume; the merged (implicit/class) Style still resolves through element (SpanTests.StyleApplied),
//     it is just not exposed as a per-span settable object property. Recorded in port/STATUS.md.
//   - FontSize default is NaN in C# (IFontElement.FontSizeDefaultValueCreator); the port uses 0 to mean
//     "unset" (font::is_default / the GetEffectiveFont path both treat <=0 and NaN identically — size is
//     replaced by the default font size when not explicitly set), so is_set drives the choice exactly.
//
// get_effective_font(default_font_size, default_font) ports FontExtensions.GetEffectiveFont: each font
// field uses the span value when IsSet, else the inherited default. The attributed-string build
// (formatted_string_extensions on apple/ios + the headless attributed-run mirror) reads it per span.

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/element.hpp"
#include "maui/controls/gestures/gesture_recognizer.hpp"
#include "maui/controls/gestures/gesture_recognizer_collection.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/font.hpp"
#include "maui/core/font_attributes.hpp"
#include "maui/core/property.hpp"
#include "maui/core/text_decorations.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class span : public element
    {
    public:
        span();

        // Shared bindable-property descriptors (one instance per type, like Span.*Property).
        static const maui::core::bindable_property<std::string>& text_property();
        static const maui::core::bindable_property<std::string>& font_family_property();
        static const maui::core::bindable_property<double>& font_size_property();
        static const maui::core::bindable_property<maui::core::font_attributes>& font_attributes_property();
        static const maui::core::bindable_property<bool>& font_auto_scaling_enabled_property();
        static const maui::core::bindable_property<maui::graphics::color>& text_color_property();
        static const maui::core::bindable_property<maui::graphics::color>& background_color_property();
        static const maui::core::bindable_property<double>& character_spacing_property();
        static const maui::core::bindable_property<maui::core::text_decorations>& text_decorations_property();
        static const maui::core::bindable_property<double>& line_height_property();

        // ---- Text (ContentProperty) ----
        [[nodiscard]] std::string_view text() const
        {
            return text_.get();
        }
        void set_text(std::string value)
        {
            text_.set(std::move(value));
        }

        // ---- IFontElement ----
        [[nodiscard]] std::string_view font_family() const
        {
            return font_family_.get();
        }
        void set_font_family(std::string value)
        {
            font_family_.set(std::move(value));
        }
        [[nodiscard]] double font_size() const
        {
            return font_size_.get();
        }
        void set_font_size(double value)
        {
            font_size_.set(value);
        }
        [[nodiscard]] maui::core::font_attributes font_attributes() const
        {
            return font_attributes_.get();
        }
        void set_font_attributes(maui::core::font_attributes value)
        {
            font_attributes_.set(value);
        }
        [[nodiscard]] bool font_auto_scaling_enabled() const
        {
            return font_auto_scaling_enabled_.get();
        }
        void set_font_auto_scaling_enabled(bool value)
        {
            font_auto_scaling_enabled_.set(value);
        }

        // ---- ITextElement ----
        [[nodiscard]] maui::graphics::color text_color() const
        {
            return text_color_.get();
        }
        void set_text_color(maui::graphics::color value)
        {
            text_color_.set(value);
        }
        [[nodiscard]] maui::graphics::color background_color() const
        {
            return background_color_.get();
        }
        void set_background_color(maui::graphics::color value)
        {
            background_color_.set(value);
        }
        [[nodiscard]] double character_spacing() const
        {
            return character_spacing_.get();
        }
        void set_character_spacing(double value)
        {
            character_spacing_.set(value);
        }

        // ---- IDecorableTextElement ----
        [[nodiscard]] maui::core::text_decorations text_decorations() const
        {
            return text_decorations_.get();
        }
        void set_text_decorations(maui::core::text_decorations value)
        {
            text_decorations_.set(value);
        }

        // ---- ILineHeightElement ----
        [[nodiscard]] double line_height() const
        {
            return line_height_.get();
        }
        void set_line_height(double value)
        {
            line_height_.set(value);
        }

        // ---- IsSet probes (BindableObject.IsSet — GetEffectiveFont uses them; also TextColor/
        // CharacterSpacing/TextDecorations checks in the C# attributed-string build) ----
        [[nodiscard]] bool is_font_family_set() const
        {
            return font_family_.is_set();
        }
        [[nodiscard]] bool is_font_size_set() const
        {
            return font_size_.is_set();
        }
        [[nodiscard]] bool is_font_attributes_set() const
        {
            return font_attributes_.is_set();
        }
        [[nodiscard]] bool is_font_auto_scaling_enabled_set() const
        {
            return font_auto_scaling_enabled_.is_set();
        }
        [[nodiscard]] bool is_text_color_set() const
        {
            return text_color_.is_set();
        }
        [[nodiscard]] bool is_background_color_set() const
        {
            return background_color_.is_set();
        }
        [[nodiscard]] bool is_character_spacing_set() const
        {
            return character_spacing_.is_set();
        }
        [[nodiscard]] bool is_text_decorations_set() const
        {
            return text_decorations_.is_set();
        }

        // C# FontExtensions.GetEffectiveFont(span, defaultFontSize, defaultFont): synthesize the font for
        // this span's run — each field falls back to the inherited default when this span has not set it.
        [[nodiscard]] maui::core::font get_effective_font(double default_font_size,
                                                          const maui::core::font& default_font) const;

        // ---- GestureRecognizers (the span tap surface, GestureElement) ----
        // Span.ValidateGesture only allows TapGestureRecognizer (else InvalidOperationException). The
        // collection raises element::property_changed via the changed hook so the owning formatted_string /
        // label re-syncs (Label.SetupSpanGestureRecognizers); a span is a leaf with no native handler, so
        // the recognizers are stored + parented but not pushed to a platform view here (the label hosts the
        // composite recognizers — deferred, recorded in port/STATUS.md).
        [[nodiscard]] gesture_recognizer_collection& gesture_recognizers()
        {
            return gesture_recognizers_;
        }
        [[nodiscard]] const gesture_recognizer_collection& gesture_recognizers() const
        {
            return gesture_recognizers_;
        }
        // The non-template reach for the XAML loader (element::gesture_recognizers_or_null): a span owns
        // a real collection too (C# Span : GestureElement), so <Span.GestureRecognizers><Tap/></…> lands
        // here — subject to validate_gesture's tap-only rule below.
        [[nodiscard]] gesture_recognizer_collection* gesture_recognizers_or_null() override
        {
            return &gesture_recognizers_;
        }

    protected:
        // Element.LogicalChildren: a span's gesture recognizers are its logical children (so they inherit
        // the span's BindingContext — Span.OnBindingContextChanged propagates into the recognizers).
        void for_each_logical_child(const std::function<void(element&)>& visit) const override;

    private:
        // Span.ValidateGesture — only a tap recognizer is allowed (throws std::runtime_error otherwise,
        // the port's InvalidOperationException stand-in). Called from the collection's validate hook.
        static void validate_gesture(const gesture_recognizer& recognizer);

        maui::core::property<std::string> text_{*this, text_property()};
        maui::core::property<std::string> font_family_{*this, font_family_property()};
        maui::core::property<double> font_size_{*this, font_size_property()};
        maui::core::property<maui::core::font_attributes> font_attributes_{*this, font_attributes_property()};
        maui::core::property<bool> font_auto_scaling_enabled_{*this, font_auto_scaling_enabled_property()};
        maui::core::property<maui::graphics::color> text_color_{*this, text_color_property()};
        maui::core::property<maui::graphics::color> background_color_{*this, background_color_property()};
        maui::core::property<double> character_spacing_{*this, character_spacing_property()};
        maui::core::property<maui::core::text_decorations> text_decorations_{*this, text_decorations_property()};
        maui::core::property<double> line_height_{*this, line_height_property()};

        gesture_recognizer_collection gesture_recognizers_;
    };
} // namespace maui::controls
