#pragma once
// maui::layouts::layout_manager  <=  Microsoft.Maui.Layouts.LayoutManager
//
// Abstract base for every layout manager: holds the layout it manages and provides the shared
// constraint-resolution rule. Concrete managers implement measure/arrange_children. Ported from
// src/Core/src/Layouts/LayoutManager.cs.

#include "maui/core/dimension.hpp"
#include "maui/core/i_layout.hpp"
#include "maui/layouts/i_layout_manager.hpp"

namespace maui::layouts
{
    class layout_manager : public i_layout_manager
    {
    public:
        explicit layout_manager(maui::core::i_layout& layout) : layout_(&layout)
        {
        }

        [[nodiscard]] maui::core::i_layout& layout() const
        {
            return *layout_;
        }

        // C# LayoutManager.ResolveConstraints: pick the explicit length if set (else the measured one),
        // clamp into [min, max], then cap by the external constraint.
        [[nodiscard]] static double resolve_constraints(double external_constraint, double explicit_length,
                                                        double measured_length,
                                                        double min = maui::core::dimension::minimum,
                                                        double max = maui::core::dimension::maximum);

        // C# ViewHandlerExtensions.ResolveConstraints(measured, exact, min, max) — the PER-CHILD size
        // resolution that runs in a view's handler-measure (GetDesiredSizeFromHandler): pick the exact
        // length if explicitly set (else the measured one), then clamp into [resolve_minimum(min), max]
        // with max winning over exact and min winning over both. Unlike the layout-level resolve_constraints
        // above, there is NO external-constraint cap — this resolves a single child against its OWN
        // Width/Minimum*/Maximum* requests. Used by view<>::measure (the leaf-control measure seam).
        [[nodiscard]] static double resolve_size_request(double measured, double exact, double min, double max);

    private:
        maui::core::i_layout* layout_;
    };
} // namespace maui::layouts
