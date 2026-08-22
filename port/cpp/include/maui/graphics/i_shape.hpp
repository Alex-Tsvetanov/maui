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

        // Does path_for_bounds ALREADY apply C# Shape.TransformPathForBounds' half-stroke deflate
        // (`viewBounds.X += StrokeThickness / 2; Width -= StrokeThickness` — Shape.cs:312-323)?
        //
        // false for the shapes under graphics/shapes/, which omit it deliberately (see their headers) and
        // rely on maui::core::shape_self_inset in the border handlers to supply it. TRUE for
        // maui::controls::shapes::shape, which ports TransformPathForBounds faithfully and therefore
        // deflates on its own — feeding one of those to shape_self_inset counts the SAME C# step twice.
        //
        // Both kinds reach the same border handler: the code-first builder hands it a graphics shape,
        // while the XAML loader hands it a CONTROLS shape for <Ellipse>/<Rectangle>/<Polygon>
        // (xaml_visitors.cpp:1955 — only <RoundRectangle> is minted as a graphics shape). Measured on
        // border_resize_content/ios, the ellipse row: MAUI and the builder column both span 100.0 pt
        // (x 168..467 at y=385), the loader column 98.67 pt (x 170..465) — exactly one extra 0.5 pt per
        // side. It is invisible on Polygon only because Polygon's Aspect is Stretch.None, where the
        // deflated bounds change nothing but a translation that already fits.
        [[nodiscard]] virtual bool applies_own_stroke_inset() const
        {
            return false;
        }

    protected:
        i_shape() = default;
        i_shape(const i_shape&) = default;
        i_shape(i_shape&&) = default;
        i_shape& operator=(const i_shape&) = default;
        i_shape& operator=(i_shape&&) = default;
    };
} // namespace maui::graphics
