#pragma once
// maui::core::i_layout_handler  <=  Microsoft.Maui.ILayoutHandler
//
// The handler contract for layouts (ILayout). Extends i_view_handler with the child-management seam: the
// cross-platform layout control calls these so the native panel can keep its subview list in sync with
// the control's logical children. Ported from src/Core/src/Handlers/Layout/ILayoutHandler.cs.
//
// In MAUI the layout invokes these through the CommandMapper ("Add"/"Remove"/…), carrying a
// LayoutHandlerUpdate (Index + IView) payload; the mapper's MapAdd/MapRemove/… simply forward to these
// methods. The C++ port keeps both: the command_mapper dispatches the named commands, and those land on
// these typed methods.

#include "maui/core/i_view_handler.hpp"

namespace maui::core
{
    class i_view;

    // i_view_handler is a *virtual* base (matched by view_handler) so the concrete layout_handler —
    // which is both a view_handler<…> and an i_layout_handler — has a single i_view_handler subobject.
    class i_layout_handler : public virtual i_view_handler
    {
    public:
        ~i_layout_handler() override = default;

        // Append `child` as the last subview (C# ILayoutHandler.Add).
        virtual void add(i_view& child) = 0;
        // Remove `child`'s subview, if present (C# ILayoutHandler.Remove).
        virtual void remove(i_view& child) = 0;
        // Remove every subview (C# ILayoutHandler.Clear).
        virtual void clear() = 0;
        // Insert `child`'s subview at the logical `index` (C# ILayoutHandler.Insert).
        virtual void insert(int index, i_view& child) = 0;
        // Replace the subview at `index` with `child`'s (C# ILayoutHandler.Update).
        virtual void update(int index, i_view& child) = 0;
        // Re-order `child`'s subview to match its z-index (C# ILayoutHandler.UpdateZIndex).
        virtual void update_z_index(i_view& child) = 0;

    protected:
        i_layout_handler() = default;
        i_layout_handler(const i_layout_handler&) = default;
        i_layout_handler(i_layout_handler&&) = default;
        i_layout_handler& operator=(const i_layout_handler&) = default;
        i_layout_handler& operator=(i_layout_handler&&) = default;
    };
} // namespace maui::core
