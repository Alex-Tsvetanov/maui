---
title: "WDSkiaDirectRenderer"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.WDSkiaDirectRenderer"
namespace: "Microsoft.Maui.Graphics.Skia"
kind: class
platforms:
  - Windows
assemblies:
  - Graphics
---

# WDSkiaDirectRenderer

> [!abstract] Class in `Microsoft.Maui.Graphics.Skia`
> Full name: `Microsoft.Maui.Graphics.Skia.WDSkiaDirectRenderer`

A renderer that directly uses SkiaSharp to render graphics in a WPF application.

## Platforms

| Platform | Available |
|---|---|
| Windows | ✅ |


## Constructors

| Name | Summary |
|---|---|
| [[WDSkiaDirectRenderer.WDSkiaDirectRenderer\|WDSkiaDirectRenderer]] | Initializes a new instance of the `WDSkiaDirectRenderer` class. |

## Properties

| Name | Summary |
|---|---|
| [[WDSkiaDirectRenderer.BackgroundColor\|BackgroundColor]] |  |
| [[WDSkiaDirectRenderer.Canvas\|Canvas]] | Gets the canvas used for drawing operations. |
| [[WDSkiaDirectRenderer.Drawable\|Drawable]] |  |
| [[WDSkiaDirectRenderer.GraphicsView\|GraphicsView]] |  |

## Methods

| Name | Summary |
|---|---|
| [[WDSkiaDirectRenderer.Detached\|Detached]] | Notifies the renderer that it has been detached from its view. |
| [[WDSkiaDirectRenderer.Dispose\|Dispose]] | Releases all resources used by the renderer. |
| [[WDSkiaDirectRenderer.Draw\|Draw]] | Gets or sets the drawable that provides the content to be rendered. |
| [[WDSkiaDirectRenderer.Invalidate\|Invalidate]] | Invalidates the entire drawing surface, causing a redraw. |
| [[WDSkiaDirectRenderer.SizeChanged\|SizeChanged]] | Notifies the renderer that the size of the drawing surface has changed. |

## Remarks

This class implements `ISkiaGraphicsRenderer` to provide drawing capabilities using SkiaSharp in WPF applications.

## See also

- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.graphics.skia.wdskiadirectrenderer)
