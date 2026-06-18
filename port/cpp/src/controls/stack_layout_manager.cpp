// maui::controls::stack_layout_manager — orientation dispatch. See the header + the C# oracle
// src/Controls/src/Core/Layout/StackLayoutManager.cs.

#include "maui/controls/stack_layout_manager.hpp"

#include <memory>

#include "maui/controls/stack_layout.hpp"
#include "maui/controls/stack_orientation.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/layouts/horizontal_stack_layout_manager.hpp"
#include "maui/layouts/i_layout_manager.hpp"
#include "maui/layouts/vertical_stack_layout_manager.hpp"

namespace maui::controls
{
    maui::layouts::i_layout_manager& stack_layout_manager::select_layout_manager()
    {
        // C# SelectLayoutManager checks UsesExpansion(_stackLayout) FIRST and returns the
        // AndExpandLayoutManager when any child uses LayoutOptions.Expands. That branch is the
        // documented deferral (no Expands surface on the port's views) — it would slot in here:
        //   if (uses_expansion(*stack_)) { return *(and_expand_ ??= ...); }   // deferred
        // The orientation dispatch below is the faithful remainder.
        if (stack_->orientation() == stack_orientation::vertical)
        {
            if (!vertical_)
            {
                vertical_ = std::make_unique<maui::layouts::vertical_stack_layout_manager>(*stack_);
            }
            return *vertical_;
        }

        if (!horizontal_)
        {
            horizontal_ = std::make_unique<maui::layouts::horizontal_stack_layout_manager>(*stack_);
        }
        return *horizontal_;
    }

    maui::graphics::size stack_layout_manager::measure(double width_constraint, double height_constraint)
    {
        return select_layout_manager().measure(width_constraint, height_constraint);
    }

    maui::graphics::size stack_layout_manager::arrange_children(const maui::graphics::rect& bounds)
    {
        return select_layout_manager().arrange_children(bounds);
    }
} // namespace maui::controls
