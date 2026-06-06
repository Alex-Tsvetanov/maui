---
title: "Microsoft.Maui.Controls.Shapes"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Controls-Shapes
---

# Microsoft.Maui.Controls.Shapes

> [!info] Namespace
> `Microsoft.Maui.Controls.Shapes` — 49 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.shapes)

## Overview

`Microsoft.Maui.Controls.Shapes` provides the vector-drawing primitives for .NET MAUI. It lets you render resolution-independent 2D graphics — lines, ellipses, rectangles, polygons, and arbitrarily complex paths — directly in your UI and in XAML, without dropping down to a custom canvas. Every concrete shape derives from [[Shape|Shape]], a `View` that exposes stroke, fill, and outline properties, so shapes participate in layout and styling like any other control.

The namespace is organized into three cooperating groups. **Shapes** ([[Ellipse|Ellipse]], [[Line|Line]], [[Rectangle|Rectangle]], [[Polygon (Shapes)|Polygon]], [[Polyline (Shapes)|Polyline]], and [[Path|Path]]) are the visible elements you place in a view. **Geometries** ([[Geometry|Geometry]], [[PathGeometry|PathGeometry]], [[EllipseGeometry|EllipseGeometry]], [[LineGeometry|LineGeometry]], [[RectangleGeometry|RectangleGeometry]], and groupings like [[GeometryGroup|GeometryGroup]]) describe abstract 2D outlines that a `Path` renders. A geometry is built from one or more [[PathFigure|PathFigure]] objects, each composed of [[PathSegment|PathSegment]] pieces such as [[LineSegment|LineSegment]], [[ArcSegment|ArcSegment]], [[BezierSegment|BezierSegment]], and [[QuadraticBezierSegment|QuadraticBezierSegment]].

The third group, **transforms**, applies coordinate manipulation to shapes and geometries. [[Transform|Transform]] is the base type, with concrete operations including [[RotateTransform|RotateTransform]], [[ScaleTransform|ScaleTransform]], [[SkewTransform|SkewTransform]], [[TranslateTransform|TranslateTransform]], and [[MatrixTransform|MatrixTransform]]; multiple transforms can be combined via [[TransformGroup|TransformGroup]] or [[CompositeTransform|CompositeTransform]]. Supporting enums such as [[FillRule|FillRule]], [[PenLineCap|PenLineCap]], and [[PenLineJoin|PenLineJoin]] control how interiors are filled and how strokes are capped and joined. A family of type converters (for example [[PathGeometryConverter|PathGeometryConverter]]) enables the concise path-markup syntax used in XAML.

## Key types

- [[Shape|Shape]] — Base class for shape elements such as Ellipse, Line, Polygon, Polyline, and Rectangle.
- [[Path|Path]] — A shape that draws complex geometries defined by a `PathGeometry`.
- [[Ellipse|Ellipse]] — A shape that draws an ellipse or circle.
- [[Rectangle|Rectangle]] — A shape that draws a rectangle, optionally with rounded corners via `RadiusX` and `RadiusY`.
- [[Line|Line]] — A shape that draws a straight line between two points.
- [[Polygon (Shapes)|Polygon]] — A shape that draws a closed polygon from a series of connected lines.
- [[Polyline (Shapes)|Polyline]] — A shape that draws connected straight lines that, unlike a polygon, are not automatically closed.
- [[Geometry|Geometry]] — The base class for all geometry objects that describe 2D shapes.
- [[PathGeometry|PathGeometry]] — A complex geometry composed of `PathFigure` objects, used to render a `Path`.
- [[PathFigure|PathFigure]] — A subsection of a geometry containing a collection of path segments.
- [[PathSegment|PathSegment]] — The base class for all segment types that define a portion of a `PathFigure`.
- [[Transform|Transform]] — Base class for all transforms (rotate, scale, skew, translate, matrix) applied to shapes.


## Classes

| Type | Summary |
|---|---|
| [[ArcSegment\|ArcSegment]] | Represents a path segment that draws an elliptical arc between two points. |
| [[BezierSegment\|BezierSegment]] | Represents a path segment that draws a cubic Bezier curve defined by three points. |
| [[CompositeTransform\|CompositeTransform]] | A transform that combines multiple transform operations (scale, skew, rotate, translate) into a single transform. |
| [[Ellipse\|Ellipse]] | A shape that draws an ellipse or circle. |
| [[EllipseGeometry\|EllipseGeometry]] | Represents the geometry of an ellipse or circle. |
| [[Geometry\|Geometry]] | The base class for all geometry objects that describe 2D shapes. |
| [[GeometryCollection\|GeometryCollection]] | A collection of `Geometry` objects. |
| [[GeometryGroup\|GeometryGroup]] | A composite `Geometry` that combines multiple `Geometry` objects into a single shape. |
| [[GeometryHelper\|GeometryHelper]] | Provides helper methods for geometry operations such as flattening curves into polylines. |
| [[Line\|Line]] | A shape that draws a straight line between two points. |
| [[LineGeometry\|LineGeometry]] | Represents the geometry of a line. |
| [[LineSegment\|LineSegment]] | A path segment that draws a straight line to a specified point. |
| [[MatrixExtensions\|MatrixExtensions]] |  |
| [[MatrixTransform\|MatrixTransform]] | A transform that uses a `Matrix` to perform arbitrary linear transformations. |
| [[MatrixTypeConverter\|MatrixTypeConverter]] | A type converter that converts strings to `Matrix` objects. |
| [[Path\|Path]] | A shape that can draw complex geometries defined by a `PathGeometry`. |
| [[PathFigure\|PathFigure]] | Represents a subsection of a geometry, containing a collection of path segments. |
| [[PathFigureCollection\|PathFigureCollection]] | A collection of `PathFigure` objects that make up a `PathGeometry`. |
| [[PathFigureCollectionConverter\|PathFigureCollectionConverter]] | A type converter that converts path markup syntax strings to `PathFigureCollection` objects. |
| [[PathGeometry\|PathGeometry]] | Represents a complex geometry composed of `PathFigure` objects. |
| [[PathGeometryConverter\|PathGeometryConverter]] | A type converter that converts path markup syntax strings to `Geometry` objects. |
| [[PathSegment\|PathSegment]] | The base class for all path segment types that define a portion of a `PathFigure`. |
| [[PathSegmentCollection\|PathSegmentCollection]] | A collection of `PathSegment` objects that define the geometry of a `PathFigure`. |
| [[PointCollectionConverter\|PointCollectionConverter]] | A type converter that converts strings to `PointCollection` objects. |
| [[PolyBezierSegment\|PolyBezierSegment]] | A path segment that draws one or more connected cubic Bezier curves. |
| [[PolyLineSegment\|PolyLineSegment]] | A path segment that draws a series of connected straight lines. |
| [[PolyQuadraticBezierSegment\|PolyQuadraticBezierSegment]] | A path segment that defines one or more connected quadratic Bezier curves. |
| [[Polygon (Shapes)\|Polygon (Shapes)]] | A shape that draws a closed polygon from a series of connected lines. |
| [[Polyline (Shapes)\|Polyline (Shapes)]] | A shape that draws a series of connected straight lines. Unlike `Polygon`, a polyline is not automatically closed. |
| [[QuadraticBezierSegment\|QuadraticBezierSegment]] | Represents a path segment that draws a quadratic Bezier curve. |
| [[Rectangle\|Rectangle]] | A `Shape` that draws a rectangle, optionally with rounded corners via `RadiusX` and `RadiusY`. |
| [[RectangleGeometry\|RectangleGeometry]] | Represents the geometry of a rectangle. |
| [[RotateTransform\|RotateTransform]] | A transform that rotates an element around a specified center point. |
| [[RoundRectangle\|RoundRectangle]] |  |
| [[RoundRectangleGeometry\|RoundRectangleGeometry]] | Represents a geometry that describes a rounded rectangle. |
| [[ScaleTransform\|ScaleTransform]] | A transform that scales an element horizontally and/or vertically from a specified center point. |
| [[Shape\|Shape]] | Base class for shape elements, such as `Ellipse`, `Line`, `Polygon`, `Polyline`, and `Rectangle`. |
| [[SkewTransform\|SkewTransform]] | A transform that skews (shears) an element by the specified angles. |
| [[StrokeShapeTypeConverter\|StrokeShapeTypeConverter]] |  |
| [[Transform\|Transform]] | Base class for all transforms that can be applied to shapes. |
| [[TransformCollection\|TransformCollection]] | A collection of `Transform` objects. |
| [[TransformGroup\|TransformGroup]] | Represents a composite `Transform` composed of multiple transforms applied in sequence. |
| [[TransformTypeConverter\|TransformTypeConverter]] | Converts a string representation of a matrix into a `Transform` object. |
| [[TranslateTransform\|TranslateTransform]] | A transform that translates (moves) an element by a specified offset. |

## Interfaces

| Type | Summary |
|---|---|
| [[IGeometry\|IGeometry]] | Enable you to describe the geometry of a 2D shape. |

## Structs

| Type | Summary |
|---|---|
| [[Matrix\|Matrix]] | Represents a 3x3 affine transformation matrix used for 2D transformations such as rotation, scaling, skewing, and translation. |

## Enums

| Type | Summary |
|---|---|
| [[FillRule\|FillRule]] | Specifies how the interior of a shape is determined. |
| [[PenLineCap\|PenLineCap]] | Specifies the shape at the end of a line or segment. |
| [[PenLineJoin\|PenLineJoin]] | Specifies the shape at the vertices where two lines meet. |

## See also

- [[_API Reference]]
