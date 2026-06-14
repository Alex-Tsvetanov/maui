// flex_layout_manager — the FlexLayout measure/arrange bridge. A faithful port of
// src/Core/src/Layouts/FlexLayoutManager.cs. See flex_layout_manager.hpp.

#include "maui/layouts/flex_layout_manager.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "maui/core/i_flex_layout.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/layouts/layout_manager.hpp"

namespace maui::layouts
{
    flex_layout_manager::flex_layout_manager(maui::core::i_flex_layout& flex_layout)
        : layout_manager(flex_layout), flex_layout_(&flex_layout)
    {
    }

    maui::graphics::size flex_layout_manager::measure(double width_constraint, double height_constraint)
    {
        const maui::core::thickness padding = flex_layout_->padding();

        const double available_width = width_constraint - padding.horizontal_thickness();
        const double available_height = height_constraint - padding.vertical_thickness();

        double measured_height = 0;
        double measured_width = 0;

        flex_layout_->layout(available_width, available_height);

        for (int n = 0; n < flex_layout_->count(); ++n)
        {
            const maui::core::i_view& child = flex_layout_->at(n);
            if (child.visibility() == maui::core::visibility::collapsed)
            {
                continue;
            }

            const maui::graphics::rect frame = flex_layout_->get_flex_frame(child);
            measured_height = std::max(measured_height, frame.bottom());
            measured_width = std::max(measured_width, frame.right());
        }

        const double final_height = resolve_constraints(height_constraint, flex_layout_->height(),
                                                        measured_height + padding.vertical_thickness(),
                                                        flex_layout_->minimum_height(), flex_layout_->maximum_height());

        const double final_width = resolve_constraints(width_constraint, flex_layout_->width(),
                                                       measured_width + padding.horizontal_thickness(),
                                                       flex_layout_->minimum_width(), flex_layout_->maximum_width());

        return {final_width, final_height};
    }

    maui::graphics::size flex_layout_manager::arrange_children(const maui::graphics::rect& bounds)
    {
        const maui::core::thickness padding = flex_layout_->padding();

        const double top = padding.top + bounds.top();
        const double left = padding.left + bounds.left();
        const double available_width = bounds.width - padding.horizontal_thickness();
        const double available_height = bounds.height - padding.vertical_thickness();

        flex_layout_->layout(available_width, available_height);

        for (int n = 0; n < flex_layout_->count(); ++n)
        {
            maui::core::i_view& child = flex_layout_->at(n);

            maui::graphics::rect frame = flex_layout_->get_flex_frame(child);
            if (std::isnan(frame.x) || std::isnan(frame.y) || std::isnan(frame.width) || std::isnan(frame.height))
            {
                throw std::logic_error("flex frame contains NaN — something is deeply wrong");
            }

            frame = frame.offset(left, top);
            child.arrange(frame);
        }

        return bounds.size();
    }
} // namespace maui::layouts
