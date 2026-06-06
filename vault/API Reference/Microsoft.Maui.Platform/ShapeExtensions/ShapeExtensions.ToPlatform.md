---
title: "ShapeExtensions.ToPlatform"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.ShapeExtensions.ToPlatform"
declaring_type: "ShapeExtensions"
member_kind: method
---

# ShapeExtensions.ToPlatform

> [!abstract] Method of [[ShapeExtensions|ShapeExtensions]]
> Namespace: `Microsoft.Maui.Platform`

Converts a MAUI shape into an Android Graphics.Path for the given bounds, stroke thickness, and density.

## Signature

```csharp
Android.Graphics.Path! static ToPlatform(this Microsoft.Maui.Graphics.IShape! shape, Microsoft.Maui.Graphics.Rect bounds, float strokeThickness, float density, bool innerPath = false)
```

## See also

- Declaring type: [[ShapeExtensions|ShapeExtensions]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
