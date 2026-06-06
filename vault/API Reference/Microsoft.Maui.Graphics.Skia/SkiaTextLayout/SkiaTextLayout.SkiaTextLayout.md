---
title: "SkiaTextLayout.SkiaTextLayout"
tags:
  - api
  - member/constructor
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.SkiaTextLayout.SkiaTextLayout"
declaring_type: "SkiaTextLayout"
member_kind: constructor
---

# SkiaTextLayout.SkiaTextLayout

> [!abstract] Constructor of [[SkiaTextLayout|SkiaTextLayout]]
> Namespace: `Microsoft.Maui.Graphics.Skia`

Initializes a new instance of the `SkiaTextLayout` class.

## Signatures

```csharp
void SkiaTextLayout(string value, Microsoft.Maui.Graphics.RectF rect, Microsoft.Maui.Graphics.ITextAttributes textAttributes, Microsoft.Maui.Graphics.LayoutLine callback, Microsoft.Maui.Graphics.TextFlow textFlow = Microsoft.Maui.Graphics.TextFlow.ClipBounds, SkiaSharp.SKFont font = null)
void SkiaTextLayout(string value, Microsoft.Maui.Graphics.RectF rect, Microsoft.Maui.Graphics.ITextAttributes textAttributes, Microsoft.Maui.Graphics.LayoutLine callback, Microsoft.Maui.Graphics.TextFlow textFlow = Microsoft.Maui.Graphics.TextFlow.ClipBounds, SkiaSharp.SKPaint paint = null)
```

## Parameters

| Parameter | Description |
|---|---|
| `value` | The text to layout. |
| `rect` | The rectangle in which to layout the text. |
| `textAttributes` | The text attributes to apply. |
| `callback` | The callback to invoke for each line of text. |
| `textFlow` | The text flow behavior. |
| `paint` | The SkiaSharp paint object to use. This method is obsolete. |

## See also

- Declaring type: [[SkiaTextLayout|SkiaTextLayout]]
- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
