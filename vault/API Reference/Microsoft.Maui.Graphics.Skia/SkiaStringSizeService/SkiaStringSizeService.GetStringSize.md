---
title: "SkiaStringSizeService.GetStringSize"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.SkiaStringSizeService.GetStringSize"
declaring_type: "SkiaStringSizeService"
member_kind: method
---

# SkiaStringSizeService.GetStringSize

> [!abstract] Method of [[SkiaStringSizeService|SkiaStringSizeService]]
> Namespace: `Microsoft.Maui.Graphics.Skia`

Gets the size of a string when rendered with the specified font and font size.

## Signatures

```csharp
Microsoft.Maui.Graphics.SizeF GetStringSize(string value, Microsoft.Maui.Graphics.IFont font, float fontSize, Microsoft.Maui.Graphics.HorizontalAlignment horizontalAlignment, Microsoft.Maui.Graphics.VerticalAlignment verticalAlignment)
Microsoft.Maui.Graphics.SizeF GetStringSize(string value, Microsoft.Maui.Graphics.IFont font, float fontSize)
```

## Parameters

| Parameter | Description |
|---|---|
| `value` | The string to measure. |
| `font` | The font to use when measuring the string. |
| `fontSize` | The font size to use when measuring the string. |

## Returns

The size of the string in device-independent units.

## See also

- Declaring type: [[SkiaStringSizeService|SkiaStringSizeService]]
- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
