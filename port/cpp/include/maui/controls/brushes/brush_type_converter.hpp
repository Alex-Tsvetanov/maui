#pragma once
// maui::controls::convert_brush  <=  Microsoft.Maui.Controls.BrushTypeConverter (string → Brush).
//
// Ported from src/Controls/src/Core/Brush/BrushTypeConverter.cs's ConvertFrom + its nested
// GradientBrushParser. A controls-level converter (the C# type lives in Microsoft.Maui.Controls, in
// Controls.Core — NOT the XAML assembly), so it lives in maui_controls; the XAML layer merely REGISTERS it
// (register_standard_xaml_converters) under the Brush value type. Cases:
//   - null / unparseable           → SolidColorBrush with null Color (the "new SolidColorBrush(null)" tail).
//   - "linear-gradient(…)" / "radial-gradient(…)"  → the parsed gradient brush (CSS gradient grammar).
//   - "rgb(…)"/"rgba(…)"/"hsl(…)"/"hsla(…)"          → SolidColorBrush of the parsed color.
//   - a hex "#rrggbb" / a single color token / "Color.Name"  → SolidColorBrush of the parsed color.
//
// Self-contained (X1): the registration is APPENDED to register_standard_xaml_converters so the sibling X4
// converter additions union-merge trivially. Never throws on a plain unrecognized token (C#'s null-color
// SolidColorBrush fallback — BrushTypeConverterUnitTests.ConvertNullTest).
//
// Out-of-line definitions live in brush_type_converter.cpp.

#include <memory>
#include <string_view>

namespace maui::controls
{
    class brush;

    [[nodiscard]] std::shared_ptr<brush> convert_brush(std::string_view text);
} // namespace maui::controls
