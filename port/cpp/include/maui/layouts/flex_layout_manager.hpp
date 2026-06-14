#pragma once
// maui::layouts::flex_layout_manager  <=  Microsoft.Maui.Layouts.FlexLayoutManager
//
// The measure/arrange bridge for a FlexLayout: it runs the flex engine over the available area
// (i_flex_layout::layout), then reads each child's computed flex frame. Measure returns the union of the
// child frames (clamped by the layout's own size requests); arrange offsets each frame by padding +
// bounds origin and arranges the child. Ported from src/Core/src/Layouts/FlexLayoutManager.cs.
//
// In C# FlexLayoutManager : ILayoutManager (not LayoutManager). The port derives layout_manager to reuse
// the shared resolve_constraints helper (the same rule C# calls via LayoutManager.ResolveConstraints).

#include "maui/core/i_flex_layout.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/layouts/layout_manager.hpp"

namespace maui::layouts
{
    class flex_layout_manager : public layout_manager
    {
    public:
        explicit flex_layout_manager(maui::core::i_flex_layout& flex_layout);

        [[nodiscard]] maui::core::i_flex_layout& flex_layout() const
        {
            return *flex_layout_;
        }

        [[nodiscard]] maui::graphics::size measure(double width_constraint, double height_constraint) override;
        maui::graphics::size arrange_children(const maui::graphics::rect& bounds) override;

    private:
        maui::core::i_flex_layout* flex_layout_;
    };
} // namespace maui::layouts
