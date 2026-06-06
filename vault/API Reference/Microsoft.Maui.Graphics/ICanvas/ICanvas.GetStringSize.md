---
title: "ICanvas.GetStringSize"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.ICanvas.GetStringSize"
declaring_type: "ICanvas"
member_kind: method
---

# ICanvas.GetStringSize

> [!abstract] Method of [[ICanvas|ICanvas]]
> Namespace: `Microsoft.Maui.Graphics`

Calculates the area a string would occupy if drawn on the canvas.

## Signatures

```csharp
Microsoft.Maui.Graphics.SizeF GetStringSize(string value, Microsoft.Maui.Graphics.IFont font, float fontSize, Microsoft.Maui.Graphics.HorizontalAlignment horizontalAlignment, Microsoft.Maui.Graphics.VerticalAlignment verticalAlignment)
Microsoft.Maui.Graphics.SizeF GetStringSize(string value, Microsoft.Maui.Graphics.IFont font, float fontSize)
```

## Parameters

| Parameter | Description |
|---|---|
| `value` | String to calculate the size on. |
| `font` | The string's font type. |
| `fontSize` | The string's font size. |

## Returns

The area the string would occupy on the canvas.

## See also

- Declaring type: [[ICanvas|ICanvas]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
