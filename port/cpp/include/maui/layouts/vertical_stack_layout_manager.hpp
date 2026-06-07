#pragma once
// maui::layouts::vertical_stack_layout_manager  <=  Microsoft.Maui.Layouts.VerticalStackLayoutManager
//
// Stacks children top-to-bottom: measure sums child heights + inter-child spacing + padding (width is
// the widest child); arrange places each child full-width at the running vertical offset. Ported from
// src/Core/src/Layouts/VerticalStackLayoutManager.cs.

#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/layouts/stack_layout_manager.hpp"

namespace maui::core
{
    class i_stack_layout;
}

namespace maui::layouts
{
    class vertical_stack_layout_manager : public stack_layout_manager
    {
    public:
        explicit vertical_stack_layout_manager(maui::core::i_stack_layout& stack) : stack_layout_manager(stack)
        {
        }

        [[nodiscard]] maui::graphics::size measure(double width_constraint, double height_constraint) override;
        maui::graphics::size arrange_children(const maui::graphics::rect& bounds) override;
    };
} // namespace maui::layouts
