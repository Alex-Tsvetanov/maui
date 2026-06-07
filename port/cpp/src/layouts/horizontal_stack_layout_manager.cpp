// horizontal_stack_layout_manager — left-to-right stacking. See the header + HorizontalStackLayoutManager.cs.

#include "maui/layouts/horizontal_stack_layout_manager.hpp"

#include <algorithm>
#include <limits>

#include "maui/core/i_stack_layout.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    // C# HorizontalStackLayoutManager.ArrangeChild: place the child full-height at x; return its width.
    double arrange_child(maui::core::i_view& child, double height, double top, double x)
    {
        const maui::graphics::rect destination(x, top, child.desired_size().width, height);
        child.arrange(destination);
        return destination.width;
    }
} // namespace

namespace maui::layouts
{
    maui::graphics::size horizontal_stack_layout_manager::measure(double width_constraint, double height_constraint)
    {
        const maui::core::i_stack_layout& stack_layout = stack();
        const maui::core::thickness padding = stack_layout.padding();

        double measured_width = 0;
        double measured_height = 0;
        int spacing_count = 0;

        for (int n = 0; n < stack_layout.count(); ++n)
        {
            maui::core::i_view& child = stack_layout.at(n);
            if (child.visibility() == maui::core::visibility::collapsed)
            {
                continue;
            }

            ++spacing_count;
            const maui::graphics::size measured = child.measure(std::numeric_limits<double>::infinity(),
                                                                height_constraint - padding.vertical_thickness());
            measured_width += measured.width;
            measured_height = std::max(measured_height, measured.height);
        }

        measured_width += measure_spacing(stack_layout.spacing(), spacing_count);
        measured_width += padding.horizontal_thickness();
        measured_height += padding.vertical_thickness();

        const double final_height = resolve_constraints(height_constraint, stack_layout.height(), measured_height,
                                                        stack_layout.minimum_height(), stack_layout.maximum_height());
        const double final_width = resolve_constraints(width_constraint, stack_layout.width(), measured_width,
                                                       stack_layout.minimum_width(), stack_layout.maximum_width());

        return {final_width, final_height};
    }

    maui::graphics::size horizontal_stack_layout_manager::arrange_children(const maui::graphics::rect& bounds)
    {
        const maui::core::i_stack_layout& stack_layout = stack();
        const maui::core::thickness padding = stack_layout.padding();
        const double spacing = stack_layout.spacing();
        const int child_count = stack_layout.count();

        const double top = padding.top + bounds.top();
        const double height = std::max(0.0, bounds.height - padding.vertical_thickness());
        double x_position = padding.left + bounds.left();

        for (int n = 0; n < stack_layout.count(); ++n)
        {
            maui::core::i_view& child = stack_layout.at(n);
            if (child.visibility() == maui::core::visibility::collapsed)
            {
                continue;
            }

            x_position += arrange_child(child, height, top, x_position);

            if (n < child_count - 1)
            {
                // Add spacing after every child except the last.
                x_position += spacing;
            }
        }

        return {bounds.width, bounds.height};
    }
} // namespace maui::layouts
