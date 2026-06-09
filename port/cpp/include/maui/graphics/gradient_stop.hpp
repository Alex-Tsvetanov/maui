#pragma once
// maui::graphics::gradient_stop  <=  Microsoft.Maui.Graphics.PaintGradientStop
//
// A single color stop in a gradient paint: an offset (0..1 along the gradient) and the color at that
// offset. Ported from src/Graphics/src/Graphics/PaintGradientStop.cs. C#'s PaintGradientStop is an
// IComparable<PaintGradientStop> ordered by offset; here it is a value type with an offset/color plus an
// operator< for sorting (gradient_paint::get_sorted_stops sorts by offset, mirroring C#'s Array.Sort
// which relies on CompareTo).
//
// Naming note: C# `PaintGradientStop` becomes `gradient_stop` — the snake_case form drops the redundant
// "paint" qualifier (the type only ever lives inside a gradient_paint). Members are an `offset` (float,
// matching C#) and a `color`.
//
// Header-only (a small value type with trivial bodies, like point/size — PROFILE §3 permits inline
// definitions for trivial value types).

#include "maui/graphics/color.hpp"

namespace maui::graphics
{
    class gradient_stop
    {
    public:
        gradient_stop() = default;

        // C# PaintGradientStop(float offset, Color color).
        gradient_stop(float offset, maui::graphics::color color) : offset_(offset), color_(color)
        {
        }

        // C# PaintGradientStop.Offset — the position of this stop along the gradient (typically 0..1).
        [[nodiscard]] float offset() const
        {
            return offset_;
        }
        void set_offset(float value)
        {
            offset_ = value;
        }

        // C# PaintGradientStop.Color — the color at this stop.
        [[nodiscard]] maui::graphics::color color() const
        {
            return color_;
        }
        void set_color(maui::graphics::color value)
        {
            color_ = value;
        }

        // C# PaintGradientStop.CompareTo — orders by offset (used by GetSortedStops's Array.Sort).
        friend bool operator<(const gradient_stop& a, const gradient_stop& b)
        {
            return a.offset_ < b.offset_;
        }

    private:
        float offset_ = 0.0F;
        maui::graphics::color color_;
    };
} // namespace maui::graphics
