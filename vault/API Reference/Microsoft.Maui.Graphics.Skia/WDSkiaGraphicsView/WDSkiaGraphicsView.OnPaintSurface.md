---
title: "WDSkiaGraphicsView.OnPaintSurface"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.WDSkiaGraphicsView.OnPaintSurface"
declaring_type: "WDSkiaGraphicsView"
member_kind: method
---

# WDSkiaGraphicsView.OnPaintSurface

> [!abstract] Method of [[WDSkiaGraphicsView|WDSkiaGraphicsView]]
> Namespace: `Microsoft.Maui.Graphics.Skia`

Gets or sets the renderer used to handle drawing operations.

## Signature

```csharp
void override OnPaintSurface(SkiaSharp.Views.Desktop.SKPaintSurfaceEventArgs e)
```

## Parameters

| Parameter | Description |
|---|---|
| `e` | The event arguments containing the surface to paint on. |

## Returns

A new instance of `WDSkiaDirectRenderer`.

## Remarks

When a new renderer is set, the previous renderer is disposed, and the new renderer is configured with the current drawable and view dimensions.

## See also

- Declaring type: [[WDSkiaGraphicsView|WDSkiaGraphicsView]]
- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
