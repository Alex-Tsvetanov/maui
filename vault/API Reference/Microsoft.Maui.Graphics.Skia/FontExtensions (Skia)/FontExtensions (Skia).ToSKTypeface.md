---
title: "FontExtensions (Skia).ToSKTypeface"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.FontExtensions.ToSKTypeface"
declaring_type: "FontExtensions (Skia)"
member_kind: method
---

# FontExtensions (Skia).ToSKTypeface

> [!abstract] Method of [[FontExtensions (Skia)|FontExtensions (Skia)]]
> Namespace: `Microsoft.Maui.Graphics.Skia`

Converts a .NET MAUI Graphics font to a SkiaSharp typeface.

## Signature

```csharp
SkiaSharp.SKTypeface static ToSKTypeface(this Microsoft.Maui.Graphics.IFont font)
```

## Parameters

| Parameter | Description |
|---|---|
| `font` | The font to convert. |

## Returns

A SkiaSharp typeface that corresponds to the specified font.

## Remarks

If the font name is not found as a family name, this method will attempt to load it as a file. If the font is null or has an empty name, the default typeface will be returned.

## See also

- Declaring type: [[FontExtensions (Skia)|FontExtensions (Skia)]]
- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
