// absolute_layout_manager — the AbsoluteLayout measure/arrange algorithm. A faithful port of
// src/Core/src/Layouts/AbsoluteLayoutManager.cs. See absolute_layout_manager.hpp.

#include "maui/layouts/absolute_layout_manager.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

#include "maui/core/i_absolute_layout.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/layouts/absolute_layout_flags.hpp"
#include "maui/layouts/layout_manager.hpp"

namespace maui::layouts
{
    namespace
    {
        constexpr double auto_size = -1;

        // C# ResolveDimension: default to the bounds value; if proportional (and the available space is
        // finite) it is a fraction of the available space; otherwise AutoSize defers to the measured size.
        double resolve_dimension(bool is_proportional, double from_bounds, double available, double measured)
        {
            double value = from_bounds;
            if (is_proportional && !std::isinf(available))
            {
                value *= available;
            }
            else if (value == auto_size)
            {
                value = measured;
            }
            return value;
        }

        // C# ResolveChildMeasureConstraint: a negative (unset) bounds value lets the child auto-size; a
        // proportional value scales the constraint; an absolute value is used as-is.
        double resolve_child_measure_constraint(double bounds_value, bool proportional, double constraint)
        {
            if (bounds_value < 0)
            {
                return std::numeric_limits<double>::infinity();
            }
            if (proportional)
            {
                return bounds_value * constraint;
            }
            return bounds_value;
        }
    } // namespace

    absolute_layout_manager::absolute_layout_manager(maui::core::i_absolute_layout& absolute_layout)
        : layout_manager(absolute_layout), absolute_layout_(&absolute_layout)
    {
    }

    maui::graphics::size absolute_layout_manager::measure(double width_constraint, double height_constraint)
    {
        const maui::core::thickness padding = absolute_layout_->padding();

        const double available_width = width_constraint - padding.horizontal_thickness();
        const double available_height = height_constraint - padding.vertical_thickness();

        double measured_height = 0;
        double measured_width = 0;

        for (int n = 0; n < absolute_layout_->count(); ++n)
        {
            maui::core::i_view& child = absolute_layout_->at(n);

            if (child.visibility() == maui::core::visibility::collapsed)
            {
                continue;
            }

            const maui::graphics::rect bounds = absolute_layout_->get_layout_bounds(child);
            const absolute_layout_flags flags = absolute_layout_->get_layout_flags(child);
            const bool is_width_proportional = has_flag(flags, absolute_layout_flags::width_proportional);
            const bool is_height_proportional = has_flag(flags, absolute_layout_flags::height_proportional);

            const double measure_width =
                resolve_child_measure_constraint(bounds.width, is_width_proportional, width_constraint);
            const double measure_height =
                resolve_child_measure_constraint(bounds.height, is_height_proportional, height_constraint);

            const maui::graphics::size measured = child.measure(measure_width, measure_height);

            const double width =
                resolve_dimension(is_width_proportional, bounds.width, available_width, measured.width);
            const double height =
                resolve_dimension(is_height_proportional, bounds.height, available_height, measured.height);

            measured_height = std::max(measured_height, bounds.top() + height);
            measured_width = std::max(measured_width, bounds.left() + width);
        }

        const double final_height =
            resolve_constraints(height_constraint, absolute_layout_->height(), measured_height,
                                absolute_layout_->minimum_height(), absolute_layout_->maximum_height());
        const double final_width =
            resolve_constraints(width_constraint, absolute_layout_->width(), measured_width,
                                absolute_layout_->minimum_width(), absolute_layout_->maximum_width());

        return {final_width, final_height};
    }

    maui::graphics::size absolute_layout_manager::arrange_children(const maui::graphics::rect& bounds)
    {
        const maui::core::thickness padding = absolute_layout_->padding();

        const double top = padding.top + bounds.top();
        const double left = padding.left + bounds.left();
        const double available_width = bounds.width - padding.horizontal_thickness();
        const double available_height = bounds.height - padding.vertical_thickness();

        for (int n = 0; n < absolute_layout_->count(); ++n)
        {
            maui::core::i_view& child = absolute_layout_->at(n);

            if (child.visibility() == maui::core::visibility::collapsed)
            {
                continue;
            }

            maui::graphics::rect destination = absolute_layout_->get_layout_bounds(child);
            const absolute_layout_flags flags = absolute_layout_->get_layout_flags(child);

            const bool is_width_proportional = has_flag(flags, absolute_layout_flags::width_proportional);
            const bool is_height_proportional = has_flag(flags, absolute_layout_flags::height_proportional);

            destination.width = resolve_dimension(is_width_proportional, destination.width, available_width,
                                                  child.desired_size().width);
            destination.height = resolve_dimension(is_height_proportional, destination.height, available_height,
                                                   child.desired_size().height);

            if (has_flag(flags, absolute_layout_flags::x_proportional))
            {
                destination.x = (available_width - destination.width) * destination.x;
            }

            if (has_flag(flags, absolute_layout_flags::y_proportional))
            {
                destination.y = (available_height - destination.height) * destination.y;
            }

            destination.x += left;
            destination.y += top;

            child.arrange(destination);
        }

        return {available_width, available_height};
    }
} // namespace maui::layouts
