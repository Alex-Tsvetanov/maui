---
title: "SKGraphicsExtensions.AsRotatedAndroidPath"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.SKGraphicsExtensions.AsRotatedAndroidPath"
declaring_type: "SKGraphicsExtensions"
member_kind: method
---

# SKGraphicsExtensions.AsRotatedAndroidPath

> [!abstract] Method of [[SKGraphicsExtensions|SKGraphicsExtensions]]
> Namespace: `Microsoft.Maui.Graphics.Skia`

Converts a PathF to a SkiaSharp SKPath with a specified pixels-per-unit value, zoom level, and rotation angle.

## Signature

```csharp
SkiaSharp.SKPath static AsRotatedAndroidPath(this Microsoft.Maui.Graphics.PathF target, Microsoft.Maui.Graphics.PointF center, float ppu, float zoom, float angle)
```

## Parameters

| Parameter | Description |
|---|---|
| `target` | The PathF to convert. |
| `center` | The center point around which to rotate the path. |
| `ppu` | The pixels-per-unit value. |
| `zoom` | The zoom level. |
| `angle` | The rotation angle in degrees. |

## Returns

A SkiaSharp SKPath that corresponds to the specified PathF, scaled by the specified pixels-per-unit value and zoom level, and rotated around the specified center point.

## See also

- Declaring type: [[SKGraphicsExtensions|SKGraphicsExtensions]]
- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
