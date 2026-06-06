---
title: "W2DExtensions.AsPath"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Win2D
aliases:
  - "Microsoft.Maui.Graphics.Win2D.W2DExtensions.AsPath"
declaring_type: "W2DExtensions"
member_kind: method
---

# W2DExtensions.AsPath

> [!abstract] Method of [[W2DExtensions|W2DExtensions]]
> Namespace: `Microsoft.Maui.Graphics.Win2D`

Converts the cross-platform path to a Win2D canvas geometry using the specified offsets, scale, resource creator, and fill mode.

## Signatures

```csharp
Microsoft.Graphics.Canvas.Geometry.CanvasGeometry static AsPath(this Microsoft.Maui.Graphics.PathF path, float ox, float oy, float fx, float fy, Microsoft.Graphics.Canvas.ICanvasResourceCreator creator, Microsoft.Graphics.Canvas.Geometry.CanvasFilledRegionDetermination fillMode = Microsoft.Graphics.Canvas.Geometry.CanvasFilledRegionDetermination.Winding)
Microsoft.Graphics.Canvas.Geometry.CanvasGeometry static AsPath(this Microsoft.Maui.Graphics.PathF path, Microsoft.Graphics.Canvas.ICanvasResourceCreator creator, Microsoft.Graphics.Canvas.Geometry.CanvasFilledRegionDetermination fillMode = Microsoft.Graphics.Canvas.Geometry.CanvasFilledRegionDetermination.Winding)
```

## See also

- Declaring type: [[W2DExtensions|W2DExtensions]]
- [[_Microsoft.Maui.Graphics.Win2D|Microsoft.Maui.Graphics.Win2D namespace]]
