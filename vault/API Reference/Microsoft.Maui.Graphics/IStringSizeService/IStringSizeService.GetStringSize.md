---
title: "IStringSizeService.GetStringSize"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.IStringSizeService.GetStringSize"
declaring_type: "IStringSizeService"
member_kind: method
---

# IStringSizeService.GetStringSize

> [!abstract] Method of [[IStringSizeService|IStringSizeService]]
> Namespace: `Microsoft.Maui.Graphics`

Gets the size of a string when rendered with the specified font and font size.

## Signatures

```csharp
Microsoft.Maui.Graphics.SizeF GetStringSize(string value, Microsoft.Maui.Graphics.IFont font, float fontSize, Microsoft.Maui.Graphics.HorizontalAlignment horizontalAlignment, Microsoft.Maui.Graphics.VerticalAlignment verticalAlignment)
Microsoft.Maui.Graphics.SizeF GetStringSize(string value, Microsoft.Maui.Graphics.IFont font, float fontSize)
```

## Returns

The size of the string in device-independent units.

## Parameters

| Parameter | Description |
|---|---|
| `value` | The string to measure. |
| `font` | The font to use for measurement. |
| `fontSize` | The font size in points. |

## See also

- Declaring type: [[IStringSizeService|IStringSizeService]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
