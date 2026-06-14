#pragma once
// maui::core::i_absolute_layout  <=  Microsoft.Maui.IAbsoluteLayout
//
// A layout that positions + sizes children using explicit per-child layout bounds and flags. Ported from
// src/Core/src/Core/IAbsoluteLayout.cs (IAbsoluteLayout : ILayout). The per-child LayoutBounds (a rect)
// and LayoutFlags (which of x/y/width/height are proportional) are exposed as get_* queries keyed on the
// child view, consumed by the absolute_layout_manager.

#include "maui/core/i_layout.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/layouts/absolute_layout_flags.hpp"

namespace maui::core
{
    class i_absolute_layout : public i_layout
    {
    public:
        // The child's LayoutBounds (x, y, width/height; width/height may be AutoSize == -1).
        [[nodiscard]] virtual maui::graphics::rect get_layout_bounds(const i_view& view) const = 0;
        // Which of the bounds components are proportional (a fraction of the available space).
        [[nodiscard]] virtual maui::layouts::absolute_layout_flags get_layout_flags(const i_view& view) const = 0;
    };
} // namespace maui::core
