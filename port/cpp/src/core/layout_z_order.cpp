// order_by_z_index / get_layout_handler_index — see layout_z_order.hpp. Ports of
// Microsoft.Maui.Handlers.LayoutExtensions (src/Core/src/Handlers/Layout/LayoutExtensions.cs).

#include "maui/core/layout_z_order.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

#include "maui/core/i_layout.hpp"
#include "maui/core/i_view.hpp"

namespace maui::core
{
    std::vector<i_view*> order_by_z_index(const i_layout& layout)
    {
        // C# `layout.OrderBy(v => v.ZIndex)`: LINQ OrderBy is a STABLE sort, so equal z-indices keep their
        // add (logical) order. Sort the child INDICES (by z-index, stable) rather than the pointers — the
        // ordering depends only on z-index + add order, never on pointer identity — then map to pointers.
        const int count = std::max(layout.count(), 0);
        std::vector<int> order(static_cast<std::size_t>(count));
        for (int n = 0; n < count; ++n)
        {
            order[static_cast<std::size_t>(n)] = n;
        }
        std::ranges::stable_sort(
            order, [&layout](int lhs, int rhs) { return layout.at(lhs).z_index() < layout.at(rhs).z_index(); });

        std::vector<i_view*> ordered;
        ordered.reserve(static_cast<std::size_t>(count));
        for (const int index : order)
        {
            ordered.push_back(&layout.at(index));
        }
        return ordered;
    }

    int get_layout_handler_index(const i_layout& layout, const i_view& view)
    {
        // A faithful port of C# GetLayoutHandlerIndex: the index `view` lands at when the children are
        // z-ordered. `lower_views` counts siblings that sort strictly before `view` — a lower z-index, or an
        // equal z-index encountered BEFORE `view` in add order (the `!found` half preserves add order on
        // ties). Returns -1 when `view` is not a child.
        const int count = layout.count();
        switch (count)
        {
            case 0:
                return -1;
            case 1:
                return &layout.at(0) == &view ? 0 : -1;
            default: {
                bool found = false;
                const int z_index = view.z_index();
                int lower_views = 0;

                for (int i = 0; i < count; ++i)
                {
                    const i_view& child = layout.at(i);
                    const int child_z_index = child.z_index();

                    if (&child == &view)
                    {
                        found = true;
                    }

                    if (child_z_index < z_index || (!found && child_z_index == z_index))
                    {
                        ++lower_views;
                    }
                }

                return found ? lower_views : -1;
            }
        }
    }
} // namespace maui::core
