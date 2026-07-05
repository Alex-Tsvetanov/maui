#pragma once
// maui::controls::shapes::geometry  <=  Microsoft.Maui.Controls.Shapes.Geometry  (+ IGeometry.cs)
//
// The base of all geometry objects describing 2D shapes: appends itself onto a path_f, and is an
// i_shape (C# Geometry : BindableObject, IGeometry : IShape) so a geometry can also serve as a view
// clip. path_for_bounds ignores the bounds, exactly like Geometry's explicit IShape.PathForBounds
// (the geometry is in absolute coordinates).
//
// bindable_object base (XAML registration, 2026-07): C# Geometry : BindableObject, IGeometry — ported
// faithfully so RectangleGeometry/EllipseGeometry/GeometryGroup/PathGeometry can be
// xaml_type_registry::register_type'd (which requires a bindable_object) and therefore used from
// markup as <Image.Clip><RectangleGeometry .../></Image.Clip> and as GeometryGroup's nested
// [ContentProperty("Children")] geometry elements. Previously a plain i_shape-only object (documented
// PORT COLLAPSE below); the bindable_object base only adds the change-notification surface (unused by
// the geometry family here) — the property-value model stays the plain snake_case accessors.
//
// PORT COLLAPSE (documented, not stubbed; the transform.hpp precedent): the C# family's
// PropertyChanged/InvalidateGeometryRequested resubscription (so a mutated geometry re-renders its
// owning Path) is not modeled — the port's geometries have plain snake_case accessors — re-set
// path::set_data (or call path::invalidate_data) after mutating to retrigger the mapper.
//
// Ownership (PROFILE §8): a path control / geometry_group owns its geometries via shared_ptr.

#include "maui/core/bindable_object.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/rect.hpp"

namespace maui::controls::shapes
{
    class geometry : public maui::core::bindable_object, public maui::graphics::i_shape
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
