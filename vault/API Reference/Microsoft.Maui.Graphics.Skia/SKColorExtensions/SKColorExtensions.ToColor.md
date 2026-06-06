---
title: "SKColorExtensions.ToColor"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.SKColorExtensions.ToColor"
declaring_type: "SKColorExtensions"
member_kind: method
---

# SKColorExtensions.ToColor

> [!abstract] Method of [[SKColorExtensions|SKColorExtensions]]
> Namespace: `Microsoft.Maui.Graphics.Skia`

Converts a .NET MAUI Graphics color to a SkiaSharp color.

## Signature

```csharp
SkiaSharp.SKColor static ToColor(this Microsoft.Maui.Graphics.Color target, float alpha = 1)
```

## Parameters

| Parameter | Description |
|---|---|
| `target` | The color to convert. |
| `alpha` | An optional alpha multiplier to apply to the resulting color (0-1). |

## Returns

A SkiaSharp color that corresponds to the specified color.

## See also

- Declaring type: [[SKColorExtensions|SKColorExtensions]]
- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
