---
title: "SKGraphicsExtensions.AsSkiaPath"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.SKGraphicsExtensions.AsSkiaPath"
declaring_type: "SKGraphicsExtensions"
member_kind: method
---

# SKGraphicsExtensions.AsSkiaPath

> [!abstract] Method of [[SKGraphicsExtensions|SKGraphicsExtensions]]
> Namespace: `Microsoft.Maui.Graphics.Skia`

Converts a PathF to a SkiaSharp SKPath.

## Signatures

```csharp
SkiaSharp.SKPath static AsSkiaPath(this Microsoft.Maui.Graphics.PathF path, float ppu, float ox, float oy, float fx, float fy)
SkiaSharp.SKPath static AsSkiaPath(this Microsoft.Maui.Graphics.PathF path, float ppu, float zoom)
SkiaSharp.SKPath static AsSkiaPath(this Microsoft.Maui.Graphics.PathF path, float ppu)
SkiaSharp.SKPath static AsSkiaPath(this Microsoft.Maui.Graphics.PathF target)
```

## Parameters

| Parameter | Description |
|---|---|
| `target` | The PathF to convert. |

## Returns

A SkiaSharp SKPath that corresponds to the specified PathF.

## See also

- Declaring type: [[SKGraphicsExtensions|SKGraphicsExtensions]]
- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
