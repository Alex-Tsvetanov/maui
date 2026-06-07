#pragma once
// maui::layouts::i_layout_manager  <=  Microsoft.Maui.Layouts.ILayoutManager
//
// The cross-platform layout algorithm for one layout: measure computes the layout's desired size from
// its children; arrange_children positions the children within the given bounds and returns the actual
// size used. Ported from src/Core/src/Layouts/ILayoutManager.cs.

#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::layouts
{
    class i_layout_manager
    {
    public:
        virtual ~i_layout_manager() = default;

        [[nodiscard]] virtual maui::graphics::size measure(double width_constraint, double height_constraint) = 0;
        // Not [[nodiscard]]: arrange_children is usually invoked for its side effect (positioning the
        // children); the returned actual size is only sometimes consumed.
        virtual maui::graphics::size arrange_children(const maui::graphics::rect& bounds) = 0;

    protected:
        i_layout_manager() = default;
        i_layout_manager(const i_layout_manager&) = default;
        i_layout_manager(i_layout_manager&&) = default;
        i_layout_manager& operator=(const i_layout_manager&) = default;
        i_layout_manager& operator=(i_layout_manager&&) = default;
    };
} // namespace maui::layouts
