#pragma once
// maui::graphics::gradient_paint  <=  Microsoft.Maui.Graphics.GradientPaint
//
// Abstract base for the gradient brush kinds (linear / radial). Ported from
// src/Graphics/src/Graphics/GradientPaint.cs: it holds the array of gradient_stops and exposes the
// color-at-offset interpolation (get_color_at), the start/end color accessors (which read the stops at the
// lowest/highest offset), the sorted-stops query, and the IsTransparent override (true if any stop's color
// has alpha < 1). Derives `paint` (PROFILE §11 — runtime polymorphism via the view_mapper's dynamic_cast).
//
// Default stops mirror C#: a white-to-white gradient (stop 0 -> white, stop 1 -> white). Setting an
// empty/zero-length stop list restores that default (GradientStops setter), exactly as C#.
//
// Subset note: this port models the read/compute surface the renderers need (the stops, get_color_at,
// start/end colors, sorted stops, IsTransparent, blend, set_gradient_stops, background_color). The mutating
// helpers add_offset / remove_offset are ported (they round-trip through the stop array like C#). The C#
// background_color is not part of GradientPaint (Paint.BackgroundColor is the base member, unused by a
// gradient's renderer); here background_color() returns the blended start/end midpoint (BlendStartAndEndColors)
// so a gradient still satisfies the abstract paint::background_color() contract with a representative color —
// documented as a port decision (C#'s base BackgroundColor is left at its default for gradients).
//
// Out-of-line definitions live in gradient_paint.cpp.

#include <vector>

#include "maui/graphics/color.hpp"
#include "maui/graphics/gradient_stop.hpp"
#include "maui/graphics/paint.hpp"

namespace maui::graphics
{
    class gradient_paint : public paint
    {
    public:
        // C# GradientPaint.GradientStops getter.
        [[nodiscard]] const std::vector<gradient_stop>& gradient_stops() const;
        // C# GradientPaint.GradientStops setter — a null/empty list restores the default white-to-white pair.
        void set_gradient_stops(std::vector<gradient_stop> value);

        // C# GradientPaint.StartColor / EndColor — the color of the stop at the lowest / highest offset.
        [[nodiscard]] maui::graphics::color start_color() const;
        void set_start_color(maui::graphics::color value);
        [[nodiscard]] maui::graphics::color end_color() const;
        void set_end_color(maui::graphics::color value);

        // C# GradientPaint.StartColorIndex / EndColorIndex — the index of the lowest- / highest-offset stop.
        [[nodiscard]] int start_color_index() const;
        [[nodiscard]] int end_color_index() const;

        // C# GradientPaint.GetSortedStops — a copy of the stops sorted by ascending offset.
        [[nodiscard]] std::vector<gradient_stop> get_sorted_stops() const;

        // C# GradientPaint.SetGradientStops — replace the stops from parallel offset/color arrays
        // (min length of the two, exactly as C#'s Math.Min).
        void set_gradient_stops(const std::vector<float>& offsets, const std::vector<maui::graphics::color>& colors);

        // C# GradientPaint.AddOffset(offset) — adds a stop at the interpolated color for that offset.
        void add_offset(float offset);
        // C# GradientPaint.AddOffset(offset, color) — appends a stop.
        void add_offset(float offset, maui::graphics::color color);
        // C# GradientPaint.RemoveOffset(index) — removes the stop at index (out-of-range is a no-op).
        void remove_offset(int index);

        // C# GradientPaint.GetColorAt(offset) — linear interpolation between the bracketing stops.
        [[nodiscard]] maui::graphics::color get_color_at(float offset) const;

        // C# GradientPaint.BlendStartAndEndColors() — the .5 midpoint of start/end (white if < 2 stops).
        [[nodiscard]] maui::graphics::color blend_start_and_end_colors() const;
        // C# GradientPaint.BlendStartAndEndColors(start, end, factor) — per-channel linear blend.
        [[nodiscard]] static maui::graphics::color blend_start_and_end_colors(maui::graphics::color start_color,
                                                                              maui::graphics::color end_color,
                                                                              float factor);

        // ---- paint ----
        // C# GradientPaint does not override BackgroundColor (it stays the base default). To satisfy the
        // abstract paint::background_color() contract with a representative color, return the start/end blend.
        [[nodiscard]] maui::graphics::color background_color() const override;
        // C# GradientPaint.IsTransparent — true if any stop's color has alpha < 1.
        [[nodiscard]] bool is_transparent() const override;

    protected:
        gradient_paint();
        // C# GradientPaint(GradientPaint source) — copy the stops. The compiler-generated copy already
        // does a deep copy of the vector<gradient_stop> (a value type), so the defaulted copy ctor suffices;
        // the subclasses' "copy from another gradient paint" ctors use it.
        gradient_paint(const gradient_paint&) = default;
        gradient_paint(gradient_paint&&) = default;
        gradient_paint& operator=(const gradient_paint&) = default;
        gradient_paint& operator=(gradient_paint&&) = default;

    private:
        std::vector<gradient_stop> gradient_stops_;
    };
} // namespace maui::graphics
