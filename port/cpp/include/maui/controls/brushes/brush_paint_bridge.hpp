#pragma once
// maui::controls — the Brush ⇄ Paint bridge (the C# implicit operators) + Paint.IsNullOrEmpty.
//
// C# Brush.cs defines `implicit operator Paint(Brush)` and `implicit operator Brush(Paint)`; this is how a
// developer-facing Brush feeds MAUI's Paint-consuming handlers (IView.Background returns the Brush AS a
// Paint). C++ has no implicit user-defined cross-namespace conversions on existing types, so the port
// exposes the two directions as free functions:
//
//   to_paint(const brush&)  <=  implicit operator Paint(Brush)
//        solid_color_brush      → graphics::solid_paint   (Color, default value-type when null)
//        linear_gradient_brush  → graphics::linear_gradient_paint (stops + start/end points)
//        radial_gradient_brush  → graphics::radial_gradient_paint (stops + center/radius)
//        image_brush            → core::image_source_paint (borrowing the brush's i_image_source)
//
//   to_brush(const graphics::paint&)  <=  implicit operator Brush(Paint)
//        the reverse mapping (solid/linear/radial/image), producing a new owned brush.
//
// NULL-COLOR STOPS (the value-type-color deviation, documented): C#'s Color is nullable, so a gradient
// brush with null-color stops converts to a paint whose StartColor/EndColor is null → Paint.IsNullOrEmpty
// is true. The port's graphics gradient_stop color is a VALUE type (cannot be null), so to_paint SKIPS
// null-color stops; an all-null-color gradient becomes a zero-stop paint. is_null_or_empty then reports it
// empty exactly as C# would — the OBSERVABLE behavior matches (verified by the Linear/RadialGradientBrush
// IsNullOrEmpty oracle tests); only the unrepresentable intermediate (a stop carrying a null color) differs.
//
// IMAGE SOURCE LIFETIME: image_source_paint holds the source by raw borrow, so the returned paint borrows
// the brush's shared image_source; keep the brush alive while the paint is in use (the view/shape owner
// holds both — see view.hpp / shape.hpp set_background/set_fill(brush) overloads).
//
// Out-of-line definitions live in brush_paint_bridge.cpp.

#include <memory>

#include "maui/controls/brushes/brush.hpp"
#include "maui/graphics/paint.hpp"

namespace maui::controls
{
    // C# implicit operator Paint(Brush) — null brush yields null paint (the empty shared_ptr); an
    // unsupported brush kind also yields null (C# returns null for the default/empty base brush).
    [[nodiscard]] std::shared_ptr<maui::graphics::paint> to_paint(const brush& source);
    // The null-tolerant overload (the call site usually holds a shared_ptr<brush> that may be empty).
    [[nodiscard]] std::shared_ptr<maui::graphics::paint> to_paint(const std::shared_ptr<brush>& source);

    // C# implicit operator Brush(Paint) — null paint yields null brush; an unsupported paint kind yields null.
    [[nodiscard]] std::shared_ptr<brush> to_brush(const maui::graphics::paint& source);
    [[nodiscard]] std::shared_ptr<brush> to_brush(const maui::graphics::paint* source);

    // Microsoft.Maui.Graphics.PaintExtensions.IsNullOrEmpty (src/Core/src/Graphics/PaintExtensions.cs):
    //   solid   → null OR Color is null
    //   gradient→ null OR no stops OR StartColor/EndColor is null   (here: null OR no stops — the value-type
    //             color cannot be null, and to_paint drops null-color stops, so "no stops" is the faithful
    //             observable equivalent for the all-null-color case; see the header note)
    //   image   → null OR no image source
    // The pointer form mirrors the C# extension on a nullable Paint (the test casts brush→Paint then calls it).
    [[nodiscard]] bool paint_is_null_or_empty(const maui::graphics::paint* paint);

    // `Paint p = brush; p.IsNullOrEmpty()` computed over the BRUSH's nullable data — the faithful port of
    // the C# pattern that casts a Brush to Paint then calls PaintExtensions.IsNullOrEmpty. This reads the
    // brush's nullable stop/colors directly (which the value-type graphics::paint cannot represent), so the
    // null-color gradient cases match C# exactly:
    //   solid    → null OR Color is null
    //   gradient → null OR no stops OR every stop's Color is null (StartColor/EndColor null)
    //   image    → null OR no source
    // (The pointer overload accepts the null brush.)
    [[nodiscard]] bool brush_is_null_or_empty_as_paint(const brush* source);
} // namespace maui::controls
