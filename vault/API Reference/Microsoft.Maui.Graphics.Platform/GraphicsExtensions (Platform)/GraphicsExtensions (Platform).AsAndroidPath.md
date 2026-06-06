---
title: "GraphicsExtensions (Platform).AsAndroidPath"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Platform
aliases:
  - "Microsoft.Maui.Graphics.Platform.GraphicsExtensions.AsAndroidPath"
declaring_type: "GraphicsExtensions (Platform)"
member_kind: method
---

# GraphicsExtensions (Platform).AsAndroidPath

> [!abstract] Method of [[GraphicsExtensions (Platform)|GraphicsExtensions (Platform)]]
> Namespace: `Microsoft.Maui.Graphics.Platform`

Converts the cross-platform path to a native Android path, optionally applying the specified offset and scale.

## Signatures

```csharp
Android.Graphics.Path static AsAndroidPath(this Microsoft.Maui.Graphics.PathF path, float offsetX = 0, float offsetY = 0, float scaleX = 1, float scaleY = 1)
Android.Graphics.Path static AsAndroidPath(this Microsoft.Maui.Graphics.PathF path, float ppu, float zoom)
```

## See also

- Declaring type: [[GraphicsExtensions (Platform)|GraphicsExtensions (Platform)]]
- [[_Microsoft.Maui.Graphics.Platform|Microsoft.Maui.Graphics.Platform namespace]]
