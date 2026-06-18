#pragma once
// maui::controls::stack_layout_manager  <=  Microsoft.Maui.Controls.StackLayoutManager
//
// The orientation-dispatching layout manager for the generic stack_layout control. Unlike the
// fixed-orientation VerticalStackLayout/HorizontalStackLayout (each of which hard-wires its single
// Core manager), the generic StackLayout can flip Orientation at runtime, so its manager picks the
// concrete algorithm on EACH measure/arrange by reading the layout's current orientation:
//   - vertical   -> vertical_stack_layout_manager
//   - horizontal -> horizontal_stack_layout_manager
// The sub-managers are created lazily and cached (the ??= pattern from C# SelectLayoutManager) — they
// hold only a reference to the stack, so reuse across passes is safe even after an orientation flip.
//
// Ported from src/Controls/src/Core/Layout/StackLayoutManager.cs.
//
// DEFERRED (// deferred): C# also dispatches to AndExpandLayoutManager when any child uses
// LayoutOptions.Expands (the deprecated StackLayout expansion). The port has no LayoutOptions/Expands
// surface on its views (i_view exposes only horizontal_/vertical_layout_alignment, no Expands bit), so
// the AndExpand branch is omitted here pending a separate Expands port. SelectLayoutManager() documents
// exactly where the UsesExpansion(...) check would slot in. See the unit report's deviation note.

#include <memory>

#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/layouts/horizontal_stack_layout_manager.hpp"
#include "maui/layouts/i_layout_manager.hpp"
#include "maui/layouts/vertical_stack_layout_manager.hpp"

namespace maui::controls
{
    class stack_layout; // the concrete control (orientation lives here, not on i_stack_layout)

    class stack_layout_manager : public maui::layouts::i_layout_manager
    {
    public:
        explicit stack_layout_manager(stack_layout& stack) : stack_(&stack)
        {
        }

        [[nodiscard]] maui::graphics::size measure(double width_constraint, double height_constraint) override;
        maui::graphics::size arrange_children(const maui::graphics::rect& bounds) override;

    private:
        // C# SelectLayoutManager: pick the manager matching the current orientation (lazily created and
        // cached). The AndExpand branch is the documented deferral (see the header note above).
        [[nodiscard]] maui::layouts::i_layout_manager& select_layout_manager();

        stack_layout* stack_;
        std::unique_ptr<maui::layouts::vertical_stack_layout_manager> vertical_;
        std::unique_ptr<maui::layouts::horizontal_stack_layout_manager> horizontal_;
    };
} // namespace maui::controls
