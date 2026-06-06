---
title: "GraphicsExtensions (Platform).AsCanvasGeometry"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Platform
aliases:
  - "Microsoft.Maui.Graphics.Platform.GraphicsExtensions.AsCanvasGeometry"
declaring_type: "GraphicsExtensions (Platform)"
member_kind: method
---

# GraphicsExtensions (Platform).AsCanvasGeometry

> [!abstract] Method of [[GraphicsExtensions (Platform)|GraphicsExtensions (Platform)]]
> Namespace: `Microsoft.Maui.Graphics.Platform`

Converts the cross-platform path to a native Win2D canvas geometry.

## Signatures

```csharp
Microsoft.Graphics.Canvas.Geometry.CanvasGeometry static AsCanvasGeometry(this Microsoft.Maui.Graphics.PathF path, float ox, float oy, float fx, float fy, Microsoft.Graphics.Canvas.ICanvasResourceCreator creator, Microsoft.Graphics.Canvas.Geometry.CanvasFilledRegionDetermination fillMode = Microsoft.Graphics.Canvas.Geometry.CanvasFilledRegionDetermination.Winding)
Microsoft.Graphics.Canvas.Geometry.CanvasGeometry static AsCanvasGeometry(this Microsoft.Maui.Graphics.PathF path, Microsoft.Graphics.Canvas.ICanvasResourceCreator creator, Microsoft.Graphics.Canvas.Geometry.CanvasFilledRegionDetermination fillMode = Microsoft.Graphics.Canvas.Geometry.CanvasFilledRegionDetermination.Winding)
```

## See also

- Declaring type: [[GraphicsExtensions (Platform)|GraphicsExtensions (Platform)]]
- [[_Microsoft.Maui.Graphics.Platform|Microsoft.Maui.Graphics.Platform namespace]]
