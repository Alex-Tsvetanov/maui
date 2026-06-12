#pragma once
// maui::controls::shapes::geometry_helper  <=  Microsoft.Maui.Controls.Shapes.GeometryHelper
//
// Curve flattening helpers (free functions, like the C# static class): the elliptical-arc flattener
// the path_geometry's ArcSegment rendering uses (Petzold's ArcSegment mathematics), plus the cubic /
// quadratic Bezier flatteners. Ported from GeometryHelper.cs. FlattenGeometry (the Geometry →
// polyline-only PathGeometry rewriter) is deferred — nothing in the rendering pipeline consumes it
// (documented, not stubbed).
//
// Out-of-line definitions: src/controls/shapes/geometry_helper.cpp.

#include <vector>

#include "maui/graphics/point.hpp"

namespace maui::controls::shapes
{
    // C# GeometryHelper.FlattenCubicBezier(points, ptStart, ptCtrl1, ptCtrl2, ptEnd, tolerance).
    void flatten_cubic_bezier(std::vector<maui::graphics::point>& points, const maui::graphics::point& start,
                              const maui::graphics::point& control1, const maui::graphics::point& control2,
                              const maui::graphics::point& end, double tolerance);

    // C# GeometryHelper.FlattenQuadraticBezier(points, ptStart, ptCtrl, ptEnd, tolerance).
    void flatten_quadratic_bezier(std::vector<maui::graphics::point>& points, const maui::graphics::point& start,
                                  const maui::graphics::point& control, const maui::graphics::point& end,
                                  double tolerance);

    // C# GeometryHelper.FlattenArc(points, pt1, pt2, radiusX, radiusY, angleRotation, isLargeArc,
    // isCounterclockwise, tolerance) — flattens the elliptical arc from pt1 to pt2 into a polyline.
    void flatten_arc(std::vector<maui::graphics::point>& points, const maui::graphics::point& point1,
                     const maui::graphics::point& point2, double radius_x, double radius_y, double angle_rotation,
                     bool is_large_arc, bool is_counterclockwise, double tolerance);
} // namespace maui::controls::shapes
