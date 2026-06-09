#pragma once
// maui::graphics::corner_radius  <=  Microsoft.Maui.CornerRadius
//
// The four per-corner radii of a rounded rectangle. Ported from src/Core/src/Primitives/CornerRadius.cs.
// (In C# this lives in Microsoft.Maui, i.e. maui::core; it is placed in maui::graphics here because its
// only consumer in the port so far is the round_rectangle clip shape — a graphics-layer type — and it
// keeps the per-corner-radii backlog item self-contained in the graphics unit.)
//
// A value type: four doubles (top_left/top_right/bottom_left/bottom_right), a uniform single-double
// ctor, a 4-arg ctor, and equality. C#'s `_isParameterized` flag (set by both ctors, clear on a
// default-constructed value) is mirrored for exact fidelity to CornerRadius.Equals — though it is only
// an early-out: when it does not short-circuit, the field comparison yields the same result (both
// default instances are all-zero). C#'s implicit double->CornerRadius conversion maps to the
// non-explicit uniform ctor.

namespace maui::graphics
{
    struct corner_radius
    {
        // C# `new CornerRadius()` — all zero, not parameterized.
        corner_radius() = default;

        // C# CornerRadius(double uniformRadius): all four corners share one radius. Non-explicit to
        // mirror C#'s `implicit operator CornerRadius(double)`.
        corner_radius(double uniform_radius);

        // C# CornerRadius(double topLeft, double topRight, double bottomLeft, double bottomRight).
        corner_radius(double top_left, double top_right, double bottom_left, double bottom_right);

        double top_left = 0;
        double top_right = 0;
        double bottom_left = 0;
        double bottom_right = 0;

        [[nodiscard]] bool equals(const corner_radius& other) const;

    private:
        bool is_parameterized_ = false;
    };

    bool operator==(const corner_radius& a, const corner_radius& b);
    bool operator!=(const corner_radius& a, const corner_radius& b);
} // namespace maui::graphics
