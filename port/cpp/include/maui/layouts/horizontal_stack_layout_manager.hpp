#pragma once
// maui::layouts::horizontal_stack_layout_manager  <=  Microsoft.Maui.Layouts.HorizontalStackLayoutManager
//
// Stacks children left-to-right: measure sums child widths + inter-child spacing + padding (height is
// the tallest child); arrange places each child full-height at the running horizontal offset. Ported
// from src/Core/src/Layouts/HorizontalStackLayoutManager.cs.

#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/layouts/stack_layout_manager.hpp"

namespace maui::core
{
    class i_stack_layout;
}

namespace maui::layouts
{
    class horizontal_stack_layout_manager : public stack_layout_manager
    {
    public:
        explicit horizontal_stack_layout_manager(maui::core::i_stack_layout& stack) : stack_layout_manager(stack)
        {
        }

        [[nodiscard]] maui::graphics::size measure(double width_constraint, double height_constraint) override;
        maui::graphics::size arrange_children(const maui::graphics::rect& bounds) override;
    };
} // namespace maui::layouts
