#pragma once
// maui::layouts::absolute_layout_manager  <=  Microsoft.Maui.Layouts.AbsoluteLayoutManager
//
// The measure/arrange algorithm for an AbsoluteLayout: each child is positioned + sized from its
// per-child LayoutBounds, interpreted through its LayoutFlags (any of x/y/width/height may be a
// proportion of the available space). AutoSize (-1) defers to the child's own measured size. Ported
// from src/Core/src/Layouts/AbsoluteLayoutManager.cs.

#include "maui/core/i_absolute_layout.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/layouts/layout_manager.hpp"

namespace maui::layouts
{
    class absolute_layout_manager : public layout_manager
    {
    public:
        explicit absolute_layout_manager(maui::core::i_absolute_layout& absolute_layout);

        [[nodiscard]] maui::core::i_absolute_layout& absolute_layout() const
        {
            return *absolute_layout_;
        }

        [[nodiscard]] maui::graphics::size measure(double width_constraint, double height_constraint) override;
        maui::graphics::size arrange_children(const maui::graphics::rect& bounds) override;

    private:
        maui::core::i_absolute_layout* absolute_layout_;
    };
} // namespace maui::layouts
