---
title: "Microsoft.Maui.Graphics.Skia"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Graphics-Skia
---

# Microsoft.Maui.Graphics.Skia

> [!info] Namespace
> `Microsoft.Maui.Graphics.Skia` — 20 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.graphics.skia)

## Overview

`Microsoft.Maui.Graphics.Skia` is the [SkiaSharp](https://github.com/mono/SkiaSharp) backend for .NET MAUI Graphics. It supplies a concrete implementation of the cross-platform 2D drawing abstractions (such as `ICanvas`, `IImage`, and the bitmap-export services) on top of Skia, so the same `IDrawable` drawing code can be rendered through Skia rather than a native platform canvas. This is what powers drawing on desktop targets and any host where a Skia surface is available.

The centerpiece is [[SkiaCanvas|SkiaCanvas]], a full `ICanvas` implementation that translates MAUI Graphics drawing commands into SkiaSharp calls. Its mutable drawing state (colors, fonts, transforms, stroke settings) is captured by [[SkiaCanvasState|SkiaCanvasState]] and produced through [[SkiaCanvasStateService|SkiaCanvasStateService]], enabling the standard save/restore stack. Text rendering and measurement are handled by [[SkiaTextLayout|SkiaTextLayout]], [[SkiaStringSizeService|SkiaStringSizeService]], and the [[TextLine|TextLine]] value type, while [[SkiaImage|SkiaImage]] and [[SkiaImageLoadingService (Skia)|SkiaImageLoadingService]] cover image loading and representation.

For producing raster output, [[SkiaBitmapExportContext|SkiaBitmapExportContext]] and [[PlatformBitmapExportService (Skia)|PlatformBitmapExportService]] let you draw into an off-screen Skia bitmap and export it. A set of conversion helpers — [[SKColorExtensions|SKColorExtensions]], [[SKFontExtensions|SKFontExtensions]], [[SKPaintExtensions|SKPaintExtensions]], [[SKGraphicsExtensions|SKGraphicsExtensions]], and [[FontExtensions (Skia)|FontExtensions]] — bridge MAUI Graphics types to their SkiaSharp counterparts.

The namespace also ships host-specific surfaces and renderers: [[GtkSkiaGraphicsView|GtkSkiaGraphicsView]] with [[GtkSkiaDirectRenderer|GtkSkiaDirectRenderer]] for GTK, and [[WDSkiaGraphicsView|WDSkiaGraphicsView]] with [[WDSkiaDirectRenderer|WDSkiaDirectRenderer]] and the [[ISkiaGraphicsRenderer|ISkiaGraphicsRenderer]] interface for WPF.

## Key types

- [[SkiaCanvas|SkiaCanvas]] — `ICanvas` implementation that renders graphics through SkiaSharp.
- [[SkiaCanvasState|SkiaCanvasState]] — Drawing state (colors, fonts, transforms) for a `SkiaCanvas`.
- [[SkiaCanvasStateService|SkiaCanvasStateService]] — Creates and manages `SkiaCanvasState` instances.
- [[SkiaImage|SkiaImage]] — Image implementation backed by SkiaSharp.
- [[SkiaImageLoadingService (Skia)|SkiaImageLoadingService]] — Loads images using SkiaSharp.
- [[SkiaTextLayout|SkiaTextLayout]] — Lays out text using SkiaSharp.
- [[SkiaStringSizeService|SkiaStringSizeService]] — Measures string dimensions using SkiaSharp.
- [[SkiaBitmapExportContext|SkiaBitmapExportContext]] — Context for exporting bitmaps via SkiaSharp.
- [[PlatformBitmapExportService (Skia)|PlatformBitmapExportService]] — Creates bitmap export contexts using SkiaSharp.
- [[GtkSkiaGraphicsView|GtkSkiaGraphicsView]] — GTK drawing area that renders content via SkiaSharp.
- [[WDSkiaGraphicsView|WDSkiaGraphicsView]] — WPF control that renders graphics via SkiaSharp.
- [[ISkiaGraphicsRenderer|ISkiaGraphicsRenderer]] — Renderer abstraction that draws graphics with SkiaSharp.

## Related guides

- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics]]


## Classes

| Type | Summary |
|---|---|
| [[FontExtensions (Skia)\|FontExtensions (Skia)]] | Provides extension methods for converting between .NET MAUI Graphics font types and SkiaSharp font types. |
| [[GtkSkiaDirectRenderer\|GtkSkiaDirectRenderer]] | A Skia graphics renderer that directly renders to a GTK surface. |
| [[GtkSkiaGraphicsView\|GtkSkiaGraphicsView]] | A GTK drawing area that renders content using SkiaSharp with MAUI Graphics integration. |
| [[PlatformBitmapExportService (Skia)\|PlatformBitmapExportService (Skia)]] | Provides functionality for creating bitmap export contexts using SkiaSharp. |
| [[SKColorExtensions\|SKColorExtensions]] | Provides extension methods for converting between .NET MAUI Graphics colors and SkiaSharp colors. |
| [[SKFontExtensions\|SKFontExtensions]] | Provides extension methods for SkiaSharp font objects. |
| [[SKGraphicsExtensions\|SKGraphicsExtensions]] | Provides extension methods for working with SkiaSharp graphics objects. |
| [[SKPaintExtensions\|SKPaintExtensions]] | Provides extension methods for SkiaSharp paint objects. |
| [[SkiaBitmapExportContext\|SkiaBitmapExportContext]] | Provides a context for exporting bitmaps using SkiaSharp. |
| [[SkiaCanvas\|SkiaCanvas]] | Implements a canvas that uses SkiaSharp for rendering graphics. |
| [[SkiaCanvasState\|SkiaCanvasState]] | Represents the state of a `SkiaCanvas`, maintaining properties like colors, fonts, and transformations. |
| [[SkiaCanvasStateService\|SkiaCanvasStateService]] | Provides services for creating and managing `SkiaCanvasState` instances. |
| [[SkiaImage\|SkiaImage]] | Represents an image implementation using SkiaSharp. |
| [[SkiaImageLoadingService (Skia)\|SkiaImageLoadingService (Skia)]] | Provides image loading functionality using SkiaSharp. |
| [[SkiaStringSizeService\|SkiaStringSizeService]] | Provides functionality for measuring string dimensions using SkiaSharp. |
| [[SkiaTextLayout\|SkiaTextLayout]] | Provides functionality for laying out text using SkiaSharp. |
| [[TextLine\|TextLine]] | Represents a line of text with its measured width. |
| [[WDSkiaDirectRenderer\|WDSkiaDirectRenderer]] | A renderer that directly uses SkiaSharp to render graphics in a WPF application. |
| [[WDSkiaGraphicsView\|WDSkiaGraphicsView]] | A WPF control that renders graphics using SkiaSharp. |

## Interfaces

| Type | Summary |
|---|---|
| [[ISkiaGraphicsRenderer\|ISkiaGraphicsRenderer]] | Defines a renderer that uses SkiaSharp to render graphics in a WPF application. |

## See also

- [[_API Reference]]
