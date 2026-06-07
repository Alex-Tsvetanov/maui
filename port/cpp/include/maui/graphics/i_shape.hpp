#pragma once
// maui::graphics::i_shape  <=  Microsoft.Maui.Graphics.IShape
//
// The shape contract used as a view's Clip geometry: it produces a path_f for a given view-bounds rect.
// Ported from src/Core/src/Graphics/IShape.cs (IShape { PathF PathForBounds(Rect bounds); }). An abstract
// class (PROFILE §11 — runtime polymorphism: the view_mapper pushes an i_shape* through the platform base).
//
// The IRoundRectangle / IVersionedShape internal refinements (inner paths, change versioning) are not
// ported — documented here, not stubbed. The concrete shapes live under graphics/shapes/.
//
// Ownership: the control owns the clip shape via a shared_ptr (controls/view.hpp); i_view::clip() returns
// a raw borrow and the view_platform_base mirror is a non-owning const pointer.

#include "maui/graphics/path_f.hpp"
#include "maui/graphics/rect.hpp"

namespace maui::graphics
{
    class i_shape
    {
    public:
        virtual ~i_shape() = default;

        // C# IShape.PathForBounds(Rect bounds): the path describing the shape fitted to the given bounds.
        [[nodiscard]] virtual maui::graphics::path_f path_for_bounds(const maui::graphics::rect& bounds) const = 0;

    protected:
        i_shape() = default;
        i_shape(const i_shape&) = default;
        i_shape(i_shape&&) = default;
        i_shape& operator=(const i_shape&) = default;
        i_shape& operator=(i_shape&&) = default;
    };
} // namespace maui::graphics
