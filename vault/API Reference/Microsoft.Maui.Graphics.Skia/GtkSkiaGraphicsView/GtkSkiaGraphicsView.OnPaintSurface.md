---
title: "GtkSkiaGraphicsView.OnPaintSurface"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.GtkSkiaGraphicsView.OnPaintSurface"
declaring_type: "GtkSkiaGraphicsView"
member_kind: method
---

# GtkSkiaGraphicsView.OnPaintSurface

> [!abstract] Method of [[GtkSkiaGraphicsView|GtkSkiaGraphicsView]]
> Namespace: `Microsoft.Maui.Graphics.Skia`

Gets or sets the Skia graphics renderer used to draw content on this view.

## Signature

```csharp
void override OnPaintSurface(SkiaSharp.Views.Desktop.SKPaintSurfaceEventArgs e)
```

## Parameters

| Parameter | Description |
|---|---|
| `e` | The event arguments containing the surface to paint on. |

## Returns

A new instance of `GtkSkiaDirectRenderer`.

## Remarks

When changing the renderer, the previous renderer will be disposed.

## See also

- Declaring type: [[GtkSkiaGraphicsView|GtkSkiaGraphicsView]]
- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
