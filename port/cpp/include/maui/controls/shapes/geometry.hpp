#pragma once
// maui::controls::shapes::geometry  <=  Microsoft.Maui.Controls.Shapes.Geometry  (+ IGeometry.cs)
//
// The base of all geometry objects describing 2D shapes: appends itself onto a path_f, and is an
// i_shape (C# Geometry : BindableObject, IGeometry : IShape) so a geometry can also serve as a view
// clip. path_for_bounds ignores the bounds, exactly like Geometry's explicit IShape.PathForBounds
// (the geometry is in absolute coordinates).
//
// PORT COLLAPSE (documented, not stubbed; the transform.hpp precedent): the C# family is a
// BindableObject tree with PropertyChanged/InvalidateGeometryRequested resubscription so a mutated
// geometry re-renders its Path. The port's geometries are plain objects with snake_case accessors —
// re-set path::set_data (or call path::invalidate_data) after mutating to retrigger the mapper.
//
// Ownership (PROFILE §8): a path control / geometry_group owns its geometries via shared_ptr.

#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/rect.hpp"

namespace maui::controls::shapes
{
    class geometry : public maui::graphics::i_shape
    {
    public:
        // C# Geometry.AppendPath(PathF) — append this geometry's figures onto the path.
        virtual void append_path(maui::graphics::path_f& path) const = 0;

        // C# Geometry's explicit IShape.PathForBounds: a fresh path with the geometry appended
        // (bounds unused — geometries carry absolute coordinates).
        [[nodiscard]] maui::graphics::path_f path_for_bounds(const maui::graphics::rect& /*bounds*/) const override
        {
            maui::graphics::path_f path;
            append_path(path);
            return path;
        }
    };
} // namespace maui::controls::shapes
