---
title: "IBitmapExportService.CreateContext"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.IBitmapExportService.CreateContext"
declaring_type: "IBitmapExportService"
member_kind: method
---

# IBitmapExportService.CreateContext

> [!abstract] Method of [[IBitmapExportService|IBitmapExportService]]
> Namespace: `Microsoft.Maui.Graphics`

Creates a context for exporting graphics to a bitmap with the specified dimensions and scale.

## Signature

```csharp
Microsoft.Maui.Graphics.BitmapExportContext CreateContext(int width, int height, float displayScale = 1)
```

## Returns

A new `BitmapExportContext` that can be used to draw content for export.

## Parameters

| Parameter | Description |
|---|---|
| `width` | The width of the bitmap in pixels. |
| `height` | The height of the bitmap in pixels. |
| `displayScale` | The scaling factor to apply (default is 1). Use values greater than 1 for higher-resolution exports. |

## See also

- Declaring type: [[IBitmapExportService|IBitmapExportService]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
