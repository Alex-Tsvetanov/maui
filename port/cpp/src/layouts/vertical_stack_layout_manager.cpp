// vertical_stack_layout_manager — top-to-bottom stacking. See the header + VerticalStackLayoutManager.cs.

#include "maui/layouts/vertical_stack_layout_manager.hpp"

#include <algorithm>
#include <limits>

#include "maui/core/i_stack_layout.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::layouts
{
    maui::graphics::size vertical_stack_layout_manager::measure(double width_constraint, double height_constraint)
    {
        const maui::core::i_stack_layout& stack_layout = stack();
        const maui::core::thickness padding = stack_layout.padding();

        double measured_height = 0;
        double measured_width = 0;
        const double child_width_constraint = width_constraint - padding.horizontal_thickness();
        int spacing_count = 0;

        for (int n = 0; n < stack_layout.count(); ++n)
        {
            maui::core::i_view& child = stack_layout.at(n);
            if (child.visibility() == maui::core::visibility::collapsed)
            {
                continue;
            }

            ++spacing_count;
            const maui::graphics::size measured =
                child.measure(child_width_constraint, std::numeric_limits<double>::infinity());
            measured_height += measured.height;
            measured_width = std::max(measured_width, measured.width);
        }

        measured_height += measure_spacing(stack_layout.spacing(), spacing_count);
        measured_height += padding.vertical_thickness();
        measured_width += padding.horizontal_thickness();

        const double final_height = resolve_constraints(height_constraint, stack_layout.height(), measured_height,
                                                        stack_layout.minimum_height(), stack_layout.maximum_height());
        const double final_width = resolve_constraints(width_constraint, stack_layout.width(), measured_width,
                                                       stack_layout.minimum_width(), stack_layout.maximum_width());

        return {final_width, final_height};
    }

    maui::graphics::size vertical_stack_layout_manager::arrange_children(const maui::graphics::rect& bounds)
    {
        const maui::core::i_stack_layout& stack_layout = stack();
        const maui::core::thickness padding = stack_layout.padding();

        double stack_height = padding.top + bounds.y;
        const double left = padding.left + bounds.x;
        const double width = std::max(0.0, bounds.width - padding.horizontal_thickness());

        for (int n = 0; n < stack_layout.count(); ++n)
        {
            maui::core::i_view& child = stack_layout.at(n);
            if (child.visibility() == maui::core::visibility::collapsed)
            {
                continue;
            }

            const maui::graphics::rect destination(left, stack_height, width, child.desired_size().height);
            child.arrange(destination);
            stack_height += destination.height + stack_layout.spacing();
        }

        return {bounds.width, bounds.height};
    }
} // namespace maui::layouts
