#pragma once
// maui::graphics::abstract_pattern  <=  Microsoft.Maui.Graphics.AbstractPattern
//
// Base implementation of i_pattern: stores the tile width/height and the repeat step_x/step_y, set once
// at construction. Ported from src/Graphics/src/Graphics/AbstractPattern.cs — the three protected ctors
// (full dims+steps; dims with steps == dims; a single step size used for all four). draw() stays abstract
// (the concrete pattern, e.g. picture_pattern, supplies the tile rendering).

#include "maui/graphics/i_canvas.hpp"
#include "maui/graphics/i_pattern.hpp"

namespace maui::graphics
{
    class abstract_pattern : public i_pattern
    {
    public:
        // ---- i_pattern ----
        [[nodiscard]] float width() const override
        {
            return width_;
        }
        [[nodiscard]] float height() const override
        {
            return height_;
        }
        [[nodiscard]] float step_x() const override
        {
            return step_x_;
        }
        [[nodiscard]] float step_y() const override
        {
            return step_y_;
        }

        // C# AbstractPattern.Draw stays abstract.
        void draw(i_canvas& canvas) override = 0;

    protected:
        // C# AbstractPattern(width, height, stepX, stepY).
        abstract_pattern(float width, float height, float step_x, float step_y)
            : width_(width), height_(height), step_x_(step_x), step_y_(step_y)
        {
        }
        // C# AbstractPattern(width, height) : this(width, height, width, height) — a non-overlapping tile.
        abstract_pattern(float width, float height) : abstract_pattern(width, height, width, height)
        {
        }
        // C# AbstractPattern(stepSize) : this(stepSize, stepSize).
        explicit abstract_pattern(float step_size) : abstract_pattern(step_size, step_size)
        {
        }

        abstract_pattern(const abstract_pattern&) = default;
        abstract_pattern(abstract_pattern&&) = default;
        abstract_pattern& operator=(const abstract_pattern&) = default;
        abstract_pattern& operator=(abstract_pattern&&) = default;

    private:
        float width_;
        float height_;
        float step_x_;
        float step_y_;
    };
} // namespace maui::graphics
