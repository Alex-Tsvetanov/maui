---
title: "PathArcExtensions.SVGArcTo"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PathArcExtensions.SVGArcTo"
declaring_type: "PathArcExtensions"
member_kind: method
---

# PathArcExtensions.SVGArcTo

> [!abstract] Method of [[PathArcExtensions|PathArcExtensions]]
> Namespace: `Microsoft.Maui.Graphics`

Adds an SVG arc segment to the path.

## Signature

```csharp
void static SVGArcTo(this Microsoft.Maui.Graphics.PathF aTarget, float rx, float ry, float angle, bool largeArcFlag, bool sweepFlag, float x, float y, float lastPointX, float lastPointY)
```

## Parameters

| Parameter | Description |
|---|---|
| `aTarget` | The path to which the arc segment is added. |
| `rx` | The x-radius of the ellipse. |
| `ry` | The y-radius of the ellipse. |
| `angle` | The rotation angle of the ellipse in degrees. |
| `largeArcFlag` | Determines whether the arc should be greater than or less than 180 degrees. |
| `sweepFlag` | Determines whether the arc should be swept in a positive or negative angle direction. |
| `x` | The x-coordinate of the end point of the arc. |
| `y` | The y-coordinate of the end point of the arc. |
| `lastPointX` | The x-coordinate of the start point of the arc (the current point in the path). |
| `lastPointY` | The y-coordinate of the start point of the arc (the current point in the path). |

## See also

- Declaring type: [[PathArcExtensions|PathArcExtensions]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
