---
title: "PlatformBitmapExportService (Skia).CreateContext"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.PlatformBitmapExportService.CreateContext"
declaring_type: "PlatformBitmapExportService (Skia)"
member_kind: method
---

# PlatformBitmapExportService (Skia).CreateContext

> [!abstract] Method of [[PlatformBitmapExportService (Skia)|PlatformBitmapExportService (Skia)]]
> Namespace: `Microsoft.Maui.Graphics.Skia`

Creates a new bitmap export context with the specified dimensions and display scale.

## Signature

```csharp
Microsoft.Maui.Graphics.BitmapExportContext CreateContext(int width, int height, float displayScale = 1)
```

## Parameters

| Parameter | Description |
|---|---|
| `width` | The width of the bitmap in pixels. |
| `height` | The height of the bitmap in pixels. |
| `displayScale` | The display scale factor to use. |

## Returns

A new `BitmapExportContext` instance for creating bitmap images.

## See also

- Declaring type: [[PlatformBitmapExportService (Skia)|PlatformBitmapExportService (Skia)]]
- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
