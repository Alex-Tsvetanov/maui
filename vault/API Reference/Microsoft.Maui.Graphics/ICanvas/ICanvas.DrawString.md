---
title: "ICanvas.DrawString"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.ICanvas.DrawString"
declaring_type: "ICanvas"
member_kind: method
---

# ICanvas.DrawString

> [!abstract] Method of [[ICanvas|ICanvas]]
> Namespace: `Microsoft.Maui.Graphics`

Draws a text string onto the canvas.

## Signatures

```csharp
void DrawString(string value, float x, float y, float width, float height, Microsoft.Maui.Graphics.HorizontalAlignment horizontalAlignment, Microsoft.Maui.Graphics.VerticalAlignment verticalAlignment, Microsoft.Maui.Graphics.TextFlow textFlow = Microsoft.Maui.Graphics.TextFlow.ClipBounds, float lineSpacingAdjustment = 0)
void DrawString(string value, float x, float y, Microsoft.Maui.Graphics.HorizontalAlignment horizontalAlignment)
```

## Parameters

| Parameter | Description |
|---|---|
| `value` | Text to be displayed. |
| `x` | Starting x coordinate. |
| `y` | Starting y coordinate. |
| `horizontalAlignment` | Horizontal alignment options to align the string. |

## Remarks

To draw attributed text, use `DrawText` instead.

## See also

- Declaring type: [[ICanvas|ICanvas]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
