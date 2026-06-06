---
title: "PathF"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PathF"
namespace: "Microsoft.Maui.Graphics"
kind: class
platforms:
  - All platforms (.NET)
  - Android
  - iOS
  - Mac Catalyst
  - macOS
  - Windows
  - Tizen
  - .NET Standard
assemblies:
  - Graphics
---

# PathF

> [!abstract] Class in `Microsoft.Maui.Graphics`
> Full name: `Microsoft.Maui.Graphics.PathF`

Represents a geometric path consisting of lines, curves, and shapes using single-precision floating-point coordinates.

## Platforms

| Platform | Available |
|---|---|
| All platforms (.NET) | ✅ |
| Android | ✅ |
| iOS | ✅ |
| Mac Catalyst | ✅ |
| macOS | ✅ |
| Windows | ✅ |
| Tizen | ✅ |
| .NET Standard | ✅ |


## Constructors

| Name | Summary |
|---|---|
| [[PathF.PathF\|PathF]] | Initializes a new path by copying the segments, points, and arc metadata of another `PathF`. |

## Properties

| Name | Summary |
|---|---|
| [[PathF.Bounds\|Bounds]] |  |
| [[PathF.Closed\|Closed]] |  |
| [[PathF.Count\|Count]] | Gets the total number of points in the path. |
| [[PathF.FirstPoint\|FirstPoint]] |  |
| [[PathF.LastPoint\|LastPoint]] |  |
| [[PathF.LastPointIndex\|LastPointIndex]] |  |
| [[PathF.OperationCount\|OperationCount]] | Gets the number of segment operations (including move and close) in the path. |
| [[PathF.PlatformPath\|PlatformPath]] |  |
| [[PathF.Points\|Points]] |  |
| [[PathF.SegmentCountExcludingOpenAndClose\|SegmentCountExcludingOpenAndClose]] |  |
| [[PathF.SegmentTypes\|SegmentTypes]] |  |
| [[PathF.SubPathCount\|SubPathCount]] | Gets the number of sub-paths (contiguous sequences beginning with `Move`) in the path. |
| [[PathF.this[int index]\|this[int index]]] |  |

## Methods

| Name | Summary |
|---|---|
| [[PathF.AddArc\|AddArc]] | Adds an elliptical arc segment using coordinate values instead of points. |
| [[PathF.AppendCircle\|AppendCircle]] | Appends an approximated circle path centered at the specified point. |
| [[PathF.AppendEllipse\|AppendEllipse]] | Appends an approximated ellipse path inside the specified rectangle. |
| [[PathF.AppendRectangle\|AppendRectangle]] | Appends a rectangle path using the specified rectangle bounds. |
| [[PathF.AppendRoundedRectangle\|AppendRoundedRectangle]] | Appends a rounded rectangle using the specified rectangle bounds and uniform corner radius. |
| [[PathF.Close\|Close]] | Closes the current sub-path by appending a close segment if it is not already closed. |
| [[PathF.CurveTo\|CurveTo]] | Adds a cubic Bézier curve segment using coordinate values. |
| [[PathF.Dispose\|Dispose]] | Releases native resources associated with the path. |
| [[PathF.Equals\|Equals]] | Determines whether this path and another have equivalent geometry within a tolerance. |
| [[PathF.GetArcAngle\|GetArcAngle]] | Gets an arc angle value at the specified index (stored as degrees). |
| [[PathF.GetArcClockwise\|GetArcClockwise]] | Gets the stored clockwise flag for an arc segment at the specified index. |
| [[PathF.GetBoundsByFlattening\|GetBoundsByFlattening]] | Gets the axis-aligned bounding box of the path (cached until modified). |
| [[PathF.GetFlattenedPath\|GetFlattenedPath]] | Creates a new path consisting only of line segments approximating all curves and arcs. |
| [[PathF.GetHashCode\|GetHashCode]] |  |
| [[PathF.GetPointsForSegment\|GetPointsForSegment]] | Gets the points defining the segment at the specified index. |
| [[PathF.GetRotatedPoint\|GetRotatedPoint]] | Computes the position of a point in the path after rotation about a pivot. |
| [[PathF.GetSegmentForPoint\|GetSegmentForPoint]] | Determines which segment uses the point at a specified index. |
| [[PathF.GetSegmentInfo\|GetSegmentInfo]] | Retrieves segment metadata, returning the segment type and output indices pointing into internal collections. |
| [[PathF.GetSegmentPointIndex\|GetSegmentPointIndex]] | Computes the starting point index in the internal point list for a given segment index. |
| [[PathF.GetSegmentType\|GetSegmentType]] | Gets the count of segment operations excluding a leading `Move` and a trailing `Close`, if present. |
| [[PathF.InsertCurveTo\|InsertCurveTo]] | Inserts a cubic Bézier segment at a specific segment index. |
| [[PathF.InsertLineTo\|InsertLineTo]] | Inserts a line segment at a specific segment index. |
| [[PathF.InsertQuadTo\|InsertQuadTo]] | Inserts a quadratic Bézier segment at a specific segment index. |
| [[PathF.Invalidate\|Invalidate]] | Gets or sets a platform-specific native path object associated with this path. Setting a new value disposes the previous one if disposable. |
| [[PathF.IsSubPathClosed\|IsSubPathClosed]] | Indicates whether the specified sub-path is closed. |
| [[PathF.LineTo\|LineTo]] | Adds a straight line segment to the specified coordinates. |
| [[PathF.Move\|Move]] | Offsets every point in the path by the specified amounts. |
| [[PathF.MovePoint\|MovePoint]] | Offsets a single point by the specified deltas. |
| [[PathF.MoveTo\|MoveTo]] | Starts a new sub-path at the specified coordinates. |
| [[PathF.Open\|Open]] | Reopens a previously closed last sub-path by removing its closing segment. |
| [[PathF.QuadTo\|QuadTo]] | Adds a quadratic Bézier curve segment using coordinate values. |
| [[PathF.RemoveAllSegmentsAfter\|RemoveAllSegmentsAfter]] | Removes the specified segment and all segments that follow it. |
| [[PathF.RemoveSegment\|RemoveSegment]] | Removes a single segment, adjusting internal point and arc data accordingly. |
| [[PathF.Reverse\|Reverse]] | Creates a new path with the segment and point order reversed. |
| [[PathF.Rotate\|Rotate]] | Creates a new `PathF` representing this path rotated by the specified angle about a pivot point. |
| [[PathF.Separate\|Separate]] |  |
| [[PathF.SetArcAngle\|SetArcAngle]] | Sets an arc angle value (degrees) at the specified index. |
| [[PathF.SetArcClockwise\|SetArcClockwise]] | Sets the stored clockwise flag for an arc segment. |
| [[PathF.SetPoint\|SetPoint]] | Sets the coordinates of the point at the specified index. |
| [[PathF.Transform\|Transform]] | Applies a 2D affine transformation matrix to all points in the path in place. |

## Remarks

A path is composed of one or more sub-paths, each beginning with a Move operation and consisting of connected line segments, curves, and arcs. For fill operations to work reliably, paths should typically be closed using the `Close` method or by explicitly connecting the end point back to the starting point. When creating paths for filling, ensure proper path construction to avoid exceptions during rendering. Paths that start with `LineTo` operations will automatically create an initial MoveTo operation.

## See also

- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.graphics.pathf)
