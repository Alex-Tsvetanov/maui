---
title: "SKGraphicsExtensions.GetPatternBitmap"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.SKGraphicsExtensions.GetPatternBitmap"
declaring_type: "SKGraphicsExtensions"
member_kind: method
---

# SKGraphicsExtensions.GetPatternBitmap

> [!abstract] Method of [[SKGraphicsExtensions|SKGraphicsExtensions]]
> Namespace: `Microsoft.Maui.Graphics.Skia`

Retrieves the bitmap pattern from a PatternPaint object.

## Signatures

```csharp
SkiaSharp.SKBitmap static GetPatternBitmap(this Microsoft.Maui.Graphics.PatternPaint patternPaint, float scale = 1)
SkiaSharp.SKBitmap static GetPatternBitmap(this Microsoft.Maui.Graphics.PatternPaint patternPaint, float scaleX, float scaleY, object currentFigure)
```

## Parameters

| Parameter | Description |
|---|---|
| `patternPaint` | The PatternPaint object. |
| `scale` | The scale factor for the bitmap. |

## Returns

The bitmap pattern.

## See also

- Declaring type: [[SKGraphicsExtensions|SKGraphicsExtensions]]
- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
