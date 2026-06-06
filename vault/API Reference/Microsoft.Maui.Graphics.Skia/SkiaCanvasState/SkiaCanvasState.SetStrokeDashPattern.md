---
title: "SkiaCanvasState.SetStrokeDashPattern"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.SkiaCanvasState.SetStrokeDashPattern"
declaring_type: "SkiaCanvasState"
member_kind: method
---

# SkiaCanvasState.SetStrokeDashPattern

> [!abstract] Method of [[SkiaCanvasState|SkiaCanvasState]]
> Namespace: `Microsoft.Maui.Graphics.Skia`

Gets or sets the stroke color.

## Signature

```csharp
void SetStrokeDashPattern(float[] pattern, float strokeDashOffset, float strokeSize)
```

## Parameters

| Parameter | Description |
|---|---|
| `pattern` | An array of values that specify the lengths of alternating dashes and gaps. |
| `strokeDashOffset` | The distance into the dash pattern to start the dash. |
| `strokeSize` | The stroke width to scale the pattern by. |

## Remarks

Setting this property resets any shader that might have been set.

## See also

- Declaring type: [[SkiaCanvasState|SkiaCanvasState]]
- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
