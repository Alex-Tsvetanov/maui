---
title: "SKGraphicsExtensions.AsSKColorMultiplyAlpha"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.SKGraphicsExtensions.AsSKColorMultiplyAlpha"
declaring_type: "SKGraphicsExtensions"
member_kind: method
---

# SKGraphicsExtensions.AsSKColorMultiplyAlpha

> [!abstract] Method of [[SKGraphicsExtensions|SKGraphicsExtensions]]
> Namespace: `Microsoft.Maui.Graphics.Skia`

Converts a .NET MAUI Graphics color to a SkiaSharp color with a multiplied alpha value.

## Signature

```csharp
SkiaSharp.SKColor static AsSKColorMultiplyAlpha(this Microsoft.Maui.Graphics.Color target, float alpha)
```

## Parameters

| Parameter | Description |
|---|---|
| `target` | The color to convert. |
| `alpha` | The alpha multiplier to apply (0-1). |

## Returns

A SkiaSharp color with the alpha value multiplied by the specified factor.

## See also

- Declaring type: [[SKGraphicsExtensions|SKGraphicsExtensions]]
- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
