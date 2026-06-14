#pragma once
// maui::core::label_run  <=  the per-span result of Span.ToNSAttributedString
//   (src/Controls/src/Core/Platform/{iOS,Android,Windows}/Extensions/FormattedStringExtensions.cs)
//
// One run of a label's FormattedText: a piece of text plus the attributes the platform applies to it,
// already RESOLVED against the label's defaults. The C# ToNSAttributedString resolves each span's
// effective font / color / kerning / decorations (falling back to the label's font/TextColor/etc. when
// the span leaves them unset) and emits a per-span NSAttributedString; the port's label control does the
// same resolution and hands the handler this flat list (i_label::formatted_text_runs), keeping the core
// handler ignorant of the controls-layer span / formatted_string types (the IImageSource boundary
// pattern). The handler's per-backend map_formatted_text walks the runs and builds the native attributed
// string (NSAttributedString on apple/ios; a mirror of these runs headless).
//
// Optional colors stand in for C#'s nullable Color (span.TextColor ?? defaultColor — already folded in;
// nullopt means "no color attribute / system default"). character_spacing is clamped to >= 0 by the
// resolver (Math.Max(0, characterSpacing)). line_height < 0 means "no line-height multiple" (the C#
// LineHeight >= 0 gate). The font is the span's GetEffectiveFont result.

#include <optional>
#include <string>

#include "maui/core/font.hpp"
#include "maui/core/text_decorations.hpp"
#include "maui/graphics/color.hpp"

namespace maui::core
{
    struct label_run
    {
        std::string text;                                      // the transformed span text (Span.Text)
        font run_font;                                         // GetEffectiveFont(defaultFontSize, defaultFont)
        std::optional<maui::graphics::color> text_color;       // span.TextColor ?? label default (nullopt = none)
        std::optional<maui::graphics::color> background_color; // span.BackgroundColor (nullopt = none)
        double character_spacing = 0;                          // Math.Max(0, span ?? default), the kerning
        text_decorations decorations = text_decorations::none; // underline / strikethrough flags
        double line_height = -1;                               // span.LineHeight >= 0 ? span : default; <0 = none

        friend bool operator==(const label_run& a, const label_run& b) = default;
    };
} // namespace maui::core
