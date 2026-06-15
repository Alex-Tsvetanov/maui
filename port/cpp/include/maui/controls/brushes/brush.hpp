#pragma once
// maui::controls::brush  <=  Microsoft.Maui.Controls.Brush
//
// Abstract base of the controls-level brush family (solid / gradient / image), the developer-facing
// fill described by an Element so it participates in BindingContext inheritance + resources. Ported from
// src/Controls/src/Core/Brush/Brush.cs (the base + Brush.Static.cs's named statics, which here live in
// the same Brush.cs) plus the family files (SolidColorBrush/GradientBrush/.../ImageBrush.cs).
//
// The brush feeds MAUI's Paint-consuming handlers via the brush↔paint bridge (brush_paint_bridge.hpp,
// the C# implicit operators) — VisualElement.Background and Shape.Fill/Stroke accept a brush and convert
// it to the maui::graphics::paint the native layer already renders.
//
// NAMED STATICS: the 147 system brushes (Brush.AliceBlue …) are immutable SolidColorBrushes returned by
// reference, generated off the SAME MAUI_GRAPHICS_NAMED_COLORS macro as graphics::colors (so they can
// never drift). They are immutable_brush instances — settable Color is ignored and setting Parent throws
// (BrushTypeConverterUnitTests.ImmutableBrushDoesntSetParent / InvalidOperationException…). Declared
// here (returning solid_color_brush&), defined in brush.cpp.
//
// Out-of-line definitions live in brush.cpp.

#include "maui/controls/element.hpp"
#include "maui/graphics/colors.hpp" // MAUI_GRAPHICS_NAMED_COLORS — the shared named-brush/color table

namespace maui::controls
{
    class solid_color_brush; // the named statics' concrete return type (defined in solid_color_brush.hpp)

    class brush : public element
    {
    public:
        // C# Brush.IsEmpty — overridden per subclass (a solid brush with null Color, a gradient with no
        // stops, an image with an empty/absent source).
        [[nodiscard]] virtual bool is_empty() const = 0;

        // C# Brush.IsNullOrEmpty(Brush) — null OR IsEmpty. (Null is the absent shared_ptr at the call site;
        // the pointer overload mirrors the static helper exactly.)
        [[nodiscard]] static bool is_null_or_empty(const brush* value)
        {
            return value == nullptr || value->is_empty();
        }

        // C# Brush.HasTransparency(Brush) — true if the (solid) color has alpha < 1, or any gradient stop's
        // color has alpha < 1. Null / image / colorless stops do not count as transparent (matching C#:
        // null.Alpha is not < 1, and only Solid + Gradient are inspected). Defined in brush.cpp.
        [[nodiscard]] static bool has_transparency(const brush* background);

        // ---- named statics (Brush.AliceBlue … — immutable SolidColorBrushes) ----
        // Generated from the shared color table; the snake_case accessor name matches graphics::colors.
        // (Defined in brush.cpp, where solid_color_brush + immutable_brush are complete.)
#define MAUI_CONTROLS_NAMED_BRUSH(name, str, argb) [[nodiscard]] static solid_color_brush& name();
#include "maui/controls/brushes/named_brushes.inc"
#undef MAUI_CONTROLS_NAMED_BRUSH

    protected:
        brush() = default;
    };
} // namespace maui::controls
