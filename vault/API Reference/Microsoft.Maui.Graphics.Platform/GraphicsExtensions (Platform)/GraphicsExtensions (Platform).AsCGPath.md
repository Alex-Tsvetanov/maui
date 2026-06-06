---
title: "GraphicsExtensions (Platform).AsCGPath"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Platform
aliases:
  - "Microsoft.Maui.Graphics.Platform.GraphicsExtensions.AsCGPath"
declaring_type: "GraphicsExtensions (Platform)"
member_kind: method
---

# GraphicsExtensions (Platform).AsCGPath

> [!abstract] Method of [[GraphicsExtensions (Platform)|GraphicsExtensions (Platform)]]
> Namespace: `Microsoft.Maui.Graphics.Platform`

Converts the cross-platform path to a native Core Graphics path, optionally applying the specified offset and scale.

## Signatures

```csharp
CoreGraphics.CGPath static AsCGPath(this Microsoft.Maui.Graphics.PathF target, float ox, float oy, float fx, float fy)
CoreGraphics.CGPath static AsCGPath(this Microsoft.Maui.Graphics.PathF target, float scale, float zoom)
CoreGraphics.CGPath static AsCGPath(this Microsoft.Maui.Graphics.PathF target)
```

## See also

- Declaring type: [[GraphicsExtensions (Platform)|GraphicsExtensions (Platform)]]
- [[_Microsoft.Maui.Graphics.Platform|Microsoft.Maui.Graphics.Platform namespace]]
