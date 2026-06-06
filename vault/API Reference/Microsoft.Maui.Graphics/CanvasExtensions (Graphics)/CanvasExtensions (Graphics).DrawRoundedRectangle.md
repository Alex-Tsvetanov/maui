---
title: "CanvasExtensions (Graphics).DrawRoundedRectangle"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.CanvasExtensions.DrawRoundedRectangle"
declaring_type: "CanvasExtensions (Graphics)"
member_kind: method
---

# CanvasExtensions (Graphics).DrawRoundedRectangle

> [!abstract] Method of [[CanvasExtensions (Graphics)|CanvasExtensions (Graphics)]]
> Namespace: `Microsoft.Maui.Graphics`

Draws the outline of a rounded rectangle on the canvas with the specified corner radii.

## Signatures

```csharp
void static DrawRoundedRectangle(this Microsoft.Maui.Graphics.ICanvas target, float x, float y, float width, float height, float topLeftCornerRadius, float topRightCornerRadius, float bottomLeftCornerRadius, float bottomRightCornerRadius)
void static DrawRoundedRectangle(this Microsoft.Maui.Graphics.ICanvas target, Microsoft.Maui.Graphics.Rect rect, double cornerRadius)
void static DrawRoundedRectangle(this Microsoft.Maui.Graphics.ICanvas target, Microsoft.Maui.Graphics.Rect rect, double topLeftCornerRadius, double topRightCornerRadius, double bottomLeftCornerRadius, double bottomRightCornerRadius)
void static DrawRoundedRectangle(this Microsoft.Maui.Graphics.ICanvas target, Microsoft.Maui.Graphics.RectF rect, float cornerRadius)
void static DrawRoundedRectangle(this Microsoft.Maui.Graphics.ICanvas target, Microsoft.Maui.Graphics.RectF rect, float topLeftCornerRadius, float topRightCornerRadius, float bottomLeftCornerRadius, float bottomRightCornerRadius)
void static DrawRoundedRectangle(this Microsoft.Maui.Graphics.ICanvas target, Microsoft.Maui.Graphics.RectF rect, float xRadius, float yRadius)
```

## See also

- Declaring type: [[CanvasExtensions (Graphics)|CanvasExtensions (Graphics)]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
