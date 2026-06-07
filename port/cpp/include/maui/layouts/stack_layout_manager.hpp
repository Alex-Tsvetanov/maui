#pragma once
// maui::layouts::stack_layout_manager  <=  Microsoft.Maui.Layouts.StackLayoutManager
//
// Abstract base for the vertical/horizontal stack managers: holds the typed i_stack_layout and the
// shared spacing computation. Ported from src/Core/src/Layouts/StackLayoutManager.cs.

#include "maui/core/i_stack_layout.hpp"
#include "maui/layouts/layout_manager.hpp"

namespace maui::layouts
{
    class stack_layout_manager : public layout_manager
    {
    public:
        explicit stack_layout_manager(maui::core::i_stack_layout& stack) : layout_manager(stack), stack_(&stack)
        {
        }

        [[nodiscard]] maui::core::i_stack_layout& stack() const
        {
            return *stack_;
        }

    protected:
        // Total inter-child spacing for `child_count` visible children (none below two children).
        [[nodiscard]] static double measure_spacing(double spacing, int child_count);

    private:
        maui::core::i_stack_layout* stack_;
    };
} // namespace maui::layouts
