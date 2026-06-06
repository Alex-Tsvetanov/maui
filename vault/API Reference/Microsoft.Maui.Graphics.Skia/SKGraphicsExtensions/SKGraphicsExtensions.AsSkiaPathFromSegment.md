---
title: "SKGraphicsExtensions.AsSkiaPathFromSegment"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.SKGraphicsExtensions.AsSkiaPathFromSegment"
declaring_type: "SKGraphicsExtensions"
member_kind: method
---

# SKGraphicsExtensions.AsSkiaPathFromSegment

> [!abstract] Method of [[SKGraphicsExtensions|SKGraphicsExtensions]]
> Namespace: `Microsoft.Maui.Graphics.Skia`

Converts a segment of a PathF to a SkiaSharp SKPath with a specified pixels-per-unit value and zoom level.

## Signature

```csharp
SkiaSharp.SKPath static AsSkiaPathFromSegment(this Microsoft.Maui.Graphics.PathF target, int segmentIndex, float ppu, float zoom)
```

## Parameters

| Parameter | Description |
|---|---|
| `target` | The PathF containing the segment to convert. |
| `segmentIndex` | The index of the segment to convert. |
| `ppu` | The pixels-per-unit value. |
| `zoom` | The zoom level. |

## Returns

A SkiaSharp SKPath that corresponds to the specified segment of the PathF, scaled by the specified pixels-per-unit value and zoom level.

## See also

- Declaring type: [[SKGraphicsExtensions|SKGraphicsExtensions]]
- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
