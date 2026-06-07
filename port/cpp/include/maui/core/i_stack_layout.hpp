#pragma once
// maui::core::i_stack_layout  <=  Microsoft.Maui.IStackLayout
//
// A layout that positions its children in a single line (vertical or horizontal), with uniform spacing
// between them. Ported from src/Core/src/Core/IStackLayout.cs.

#include "maui/core/i_layout.hpp"

namespace maui::core
{
    class i_stack_layout : public i_layout
    {
    public:
        // The space between adjacent (visible) children.
        [[nodiscard]] virtual double spacing() const = 0;
    };
} // namespace maui::core
