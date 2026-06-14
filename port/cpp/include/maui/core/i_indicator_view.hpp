#pragma once
// maui::core::i_indicator_view  <=  Microsoft.Maui.IIndicatorView
//
// The virtual-view contract for the position indicator (dots): the count, the selected position
// (two-way — a native dot tap writes it back), the visible-count knobs (MaximumVisible / HideSingle),
// the indicator size, the two colors, and the dot shape. Ported from src/Core/src/Core/IIndicatorView.cs.
//
// Shape notes (deviations documented):
//   - C# IndicatorsShape is an IShape (Ellipse vs Rectangle) the IndicatorViewExtensions.IsCircleShape
//     helper duck-types by path-point count; the port carries the indicator_shape enum directly and
//     IsCircleShape collapses to `shape == circle` (reflection-free, same outcome).
//   - C# IndicatorColor / SelectedIndicatorColor are Paint? (the interface notes "We only support
//     SolidPaint color"); the port carries maui::graphics::color (the solid paint's color).
//   - set_position is the inbound channel: a native dot tap (UIPageControl ValueChanged) writes the
//     selected page back through SetValueFromHandler specificity (the C# IIndicatorView.Position setter
//     uses SetterSpecificity.FromHandler).

#include "maui/controls/indicator_shape.hpp"
#include "maui/core/i_view.hpp"
#include "maui/graphics/color.hpp"

namespace maui::core
{
    class i_indicator_view : public i_view
    {
    public:
        // The number of indicators.
        [[nodiscard]] virtual int count() const = 0;

        // The currently selected indicator index (two-way: the native tap writes it back via set_position).
        [[nodiscard]] virtual int position() const = 0;
        virtual void set_position(int value) = 0;

        // Maximum number of visible indicators (int max default).
        [[nodiscard]] virtual int maximum_visible() const = 0;

        // Whether the indicator hides when only one exists.
        [[nodiscard]] virtual bool hide_single() const = 0;

        // The size of each indicator.
        [[nodiscard]] virtual double indicator_size() const = 0;

        // Color of unselected / selected indicators.
        [[nodiscard]] virtual maui::graphics::color indicator_color() const = 0;
        [[nodiscard]] virtual maui::graphics::color selected_indicator_color() const = 0;

        // Shape of the indicators (circle or square).
        [[nodiscard]] virtual maui::controls::indicator_shape indicators_shape() const = 0;
    };
} // namespace maui::core
