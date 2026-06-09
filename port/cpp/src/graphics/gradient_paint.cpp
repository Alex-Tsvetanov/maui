// maui::graphics::gradient_paint — out-of-line definitions. See gradient_paint.hpp. Ported from
// src/Graphics/src/Graphics/GradientPaint.cs (the stop store, get_color_at interpolation, start/end colors,
// sorted stops, IsTransparent). The geometry helpers GetFactor / GetLinearValue / Epsilon are ported inline
// here from src/Graphics/src/Graphics/GeometryUtil.cs (only the gradient-relevant subset; the full
// GeometryUtil is not yet ported).

#include "maui/graphics/gradient_paint.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/gradient_stop.hpp"

namespace maui::graphics
{
    namespace
    {
        // GeometryUtil.Epsilon — a small value for float comparisons.
        constexpr float k_epsilon = 0.0000000001F;

        // GeometryUtil.GetFactor(aMin, aMax, aValue): the normalized position of aValue in [aMin, aMax].
        float get_factor(float a_min, float a_max, float a_value)
        {
            const float adjusted_value = a_value - a_min;
            const float range = a_max - a_min;

            if (std::fabs(adjusted_value - range) < k_epsilon)
            {
                return 1;
            }

            return adjusted_value / range;
        }

        // GeometryUtil.GetLinearValue(aMin, aMax, aFactor): the linear interpolation aMin + (aMax-aMin)*factor.
        float get_linear_value(float a_min, float a_max, float a_factor)
        {
            float d = a_max - a_min;
            d *= a_factor;
            return a_min + d;
        }

        // The C# default stop set: white at 0 and white at 1.
        std::vector<gradient_stop> default_stops()
        {
            return {gradient_stop(0, colors::white), gradient_stop(1, colors::white)};
        }
    } // namespace

    gradient_paint::gradient_paint() : gradient_stops_(default_stops())
    {
    }

    const std::vector<gradient_stop>& gradient_paint::gradient_stops() const
    {
        return gradient_stops_;
    }

    void gradient_paint::set_gradient_stops(std::vector<gradient_stop> value)
    {
        gradient_stops_ = std::move(value);

        // C#: a null or zero-length array restores the default white-to-white pair.
        if (gradient_stops_.empty())
        {
            gradient_stops_ = default_stops();
        }
    }

    int gradient_paint::start_color_index() const
    {
        // C# StartColorIndex: the index of the stop with the lowest offset (<= a shrinking threshold from 1).
        int index = -1;
        float offset = 1;

        for (int i = 0; std::cmp_less(i, gradient_stops_.size()); i++)
        {
            if (gradient_stops_[static_cast<std::size_t>(i)].offset() <= offset)
            {
                index = i;
                offset = gradient_stops_[static_cast<std::size_t>(i)].offset();
            }
        }

        return index >= 0 ? index : 0;
    }

    int gradient_paint::end_color_index() const
    {
        // C# EndColorIndex: the index of the stop with the highest offset (>= a growing threshold from 0).
        int index = -1;
        float offset = 0;

        for (int i = 0; std::cmp_less(i, gradient_stops_.size()); i++)
        {
            if (gradient_stops_[static_cast<std::size_t>(i)].offset() >= offset)
            {
                index = i;
                offset = gradient_stops_[static_cast<std::size_t>(i)].offset();
            }
        }

        return index >= 0 ? index : static_cast<int>(gradient_stops_.size()) - 1;
    }

    maui::graphics::color gradient_paint::start_color() const
    {
        return gradient_stops_[static_cast<std::size_t>(start_color_index())].color();
    }

    void gradient_paint::set_start_color(maui::graphics::color value)
    {
        gradient_stops_[static_cast<std::size_t>(start_color_index())].set_color(value);
    }

    maui::graphics::color gradient_paint::end_color() const
    {
        return gradient_stops_[static_cast<std::size_t>(end_color_index())].color();
    }

    void gradient_paint::set_end_color(maui::graphics::color value)
    {
        gradient_stops_[static_cast<std::size_t>(end_color_index())].set_color(value);
    }

    std::vector<gradient_stop> gradient_paint::get_sorted_stops() const
    {
        // C# GetSortedStops: a copy sorted by ascending offset (Array.Sort uses PaintGradientStop.CompareTo,
        // which orders by Offset). stable_sort keeps equal-offset stops in input order. Project on offset so
        // the comparison is over the float key (matches gradient_stop::operator<, the CompareTo analog).
        std::vector<gradient_stop> sorted = gradient_stops_;
        std::ranges::stable_sort(sorted, {}, &gradient_stop::offset);
        return sorted;
    }

    void gradient_paint::set_gradient_stops(const std::vector<float>& offsets,
                                            const std::vector<maui::graphics::color>& colors)
    {
        // C# SetGradientStops: pair the two arrays up to the shorter length (Math.Min).
        const std::size_t stop_count = std::min(colors.size(), offsets.size());
        std::vector<gradient_stop> stops;
        stops.reserve(stop_count);
        for (std::size_t p = 0; p < stop_count; p++)
        {
            stops.emplace_back(offsets[p], colors[p]);
        }
        // Assign directly (C#'s SetGradientStops sets the field without the empty-restores-default guard;
        // an empty input here leaves an empty stop list, matching C# exactly).
        gradient_stops_ = std::move(stops);
    }

    void gradient_paint::add_offset(float offset)
    {
        add_offset(offset, get_color_at(offset));
    }

    void gradient_paint::add_offset(float offset, maui::graphics::color color)
    {
        gradient_stops_.emplace_back(offset, color);
    }

    void gradient_paint::remove_offset(int index)
    {
        // C# RemoveOffset: an out-of-range index is a no-op.
        if (index < 0 || std::cmp_greater_equal(index, gradient_stops_.size()))
        {
            return;
        }
        gradient_stops_.erase(gradient_stops_.begin() + index);
    }

    maui::graphics::color gradient_paint::get_color_at(float offset) const
    {
        // C# GetColorAt: a single stop returns its color directly.
        if (gradient_stops_.size() == 1)
        {
            return gradient_stops_[0].color();
        }

        float before = std::numeric_limits<float>::max();
        int before_index = -1;
        float after = std::numeric_limits<float>::max();
        int after_index = -1;

        for (int i = 0; std::cmp_less(i, gradient_stops_.size()); i++)
        {
            const float current_offset = gradient_stops_[static_cast<std::size_t>(i)].offset();

            if (std::fabs(current_offset - offset) < k_epsilon)
            {
                return gradient_stops_[static_cast<std::size_t>(i)].color();
            }

            if (current_offset < offset)
            {
                const float dx = offset - current_offset;
                if (dx < before)
                {
                    before = current_offset;
                    before_index = i;
                }
            }
            else if (current_offset > offset)
            {
                const float dx = current_offset - offset;
                if (dx < after)
                {
                    after = current_offset;
                    after_index = i;
                }
            }
        }

        if (after_index == -1)
        {
            return end_color();
        }

        if (before_index == -1)
        {
            return start_color();
        }

        const float f = get_factor(before, after, offset);
        return blend_start_and_end_colors(gradient_stops_[static_cast<std::size_t>(before_index)].color(),
                                          gradient_stops_[static_cast<std::size_t>(after_index)].color(), f);
    }

    maui::graphics::color gradient_paint::blend_start_and_end_colors() const
    {
        // C#: fewer than 2 stops blends to white.
        if (gradient_stops_.size() < 2)
        {
            return colors::white;
        }
        return blend_start_and_end_colors(start_color(), end_color(), .5F);
    }

    maui::graphics::color gradient_paint::blend_start_and_end_colors(maui::graphics::color start_color,
                                                                     maui::graphics::color end_color, float factor)
    {
        // C# BlendStartAndEndColors(start, end, factor): per-channel linear interpolation. (Our colors are
        // value types and never null, so C#'s `?? Colors.White` coalescing is a no-op here.)
        const float r = get_linear_value(start_color.red, end_color.red, factor);
        const float g = get_linear_value(start_color.green, end_color.green, factor);
        const float b = get_linear_value(start_color.blue, end_color.blue, factor);
        const float a = get_linear_value(start_color.alpha, end_color.alpha, factor);

        return {r, g, b, a};
    }

    maui::graphics::color gradient_paint::background_color() const
    {
        // Port decision (see header): a gradient has no single fill color; return the start/end blend so the
        // abstract paint::background_color() contract is satisfied with a representative color.
        return blend_start_and_end_colors();
    }

    bool gradient_paint::is_transparent() const
    {
        // C# GradientPaint.IsTransparent: true if any stop's color has alpha < 1.
        return std::ranges::any_of(gradient_stops_, [](const gradient_stop& stop) { return stop.color().alpha < 1; });
    }
} // namespace maui::graphics
