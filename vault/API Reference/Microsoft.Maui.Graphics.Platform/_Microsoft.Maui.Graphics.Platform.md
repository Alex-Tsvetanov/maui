---
title: "Microsoft.Maui.Graphics.Platform"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Graphics-Platform
---

# Microsoft.Maui.Graphics.Platform

> [!info] Namespace
> `Microsoft.Maui.Graphics.Platform` — 27 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.graphics.platform)

## Overview

`Microsoft.Maui.Graphics.Platform` is the platform-binding layer of the .NET MAUI Graphics drawing system. While `Microsoft.Maui.Graphics` defines the abstract, cross-platform drawing surface (canvas, paths, colors, fonts, images), the types in this namespace map those abstractions onto each operating system's native 2D graphics stack — Core Graphics / UIKit / AppKit on Apple platforms, and the equivalent rendering primitives on other targets. This is where the portable API actually gets turned into pixels.

The centerpiece is [[PlatformCanvas|PlatformCanvas]], the native-backed implementation of the drawing canvas, supported by [[PlatformCanvasState|PlatformCanvasState]] and [[PlatformCanvasStateService|PlatformCanvasStateService]] for managing transform/clip/style state during draw operations. Rendering is driven through [[IGraphicsRenderer|IGraphicsRenderer]] and its [[DirectRenderer|DirectRenderer]] implementation, and surfaced to the UI tree through [[PlatformGraphicsView|PlatformGraphicsView]]. Images are handled by [[PlatformImage|PlatformImage]], a platform-agnostic image implementation, together with [[PlatformImageLoadingService|PlatformImageLoadingService]] for decoding and [[PlatformBitmapExportContext|PlatformBitmapExportContext]] / [[PlatformBitmapExportService (Platform)|PlatformBitmapExportService]] for producing bitmaps.

The remainder of the namespace is a collection of interop extension classes that bridge MAUI Graphics types to native types: color converters ([[CGColorExtensions|CGColorExtensions]], [[NSColorExtensions|NSColorExtensions]], [[UIColorExtensions|UIColorExtensions]], [[ColorExtensions (Microsoft.Maui.Graphics.Platform)|ColorExtensions]]), image converters ([[NSImageExtensions|NSImageExtensions]], [[UIImageExtensions|UIImageExtensions]], [[ImageExtensions (Platform)|ImageExtensions]]), text and font helpers, and Core Graphics / UIKit / AppKit glue. Most app developers consume these indirectly through the cross-platform API; they are primarily of interest when writing custom platform-specific drawing or interop code.

## Key types

- [[PlatformCanvas|PlatformCanvas]] — Native-backed implementation of the MAUI Graphics drawing canvas.
- [[PlatformGraphicsView|PlatformGraphicsView]] — Platform view that hosts and displays drawn graphics content in the UI.
- [[IGraphicsRenderer|IGraphicsRenderer]] — Abstraction for rendering drawing commands onto a platform surface.
- [[DirectRenderer|DirectRenderer]] — Renderer implementation that draws directly to the native graphics context.
- [[PlatformImage|PlatformImage]] — Provides a platform-agnostic implementation of an image.
- [[PlatformImageLoadingService|PlatformImageLoadingService]] — Service for loading and decoding images into platform image instances.
- [[PlatformBitmapExportContext|PlatformBitmapExportContext]] — Drawing context used to render into and export a bitmap.
- [[PlatformBitmapExportService (Platform)|PlatformBitmapExportService]] — Service that creates bitmap export contexts.
- [[PlatformCanvasState|PlatformCanvasState]] — Captures transform, clip, and style state for a platform canvas.
- [[PlatformStringSizeService|PlatformStringSizeService]] — Measures text dimensions using native font metrics.
- [[PlatformCanvasStateService|PlatformCanvasStateService]] — Manages creation and restoration of canvas state.

| Type | Summary |
|---|---|
| [[AppKitConstants]] |  |
| [[AttributedTextExtensions (Platform)]] |  |
| [[CGColorExtensions]] |  |
| [[CgContextExtensions]] |  |
| [[ColorExtensions (Microsoft.Maui.Graphics.Platform)]] |  |
| [[CoreGraphicsExtensions (Microsoft.Maui.Graphics.Platform)]] |  |
| [[DirectRenderer]] |  |
| [[FontExtensions (Microsoft.Maui.Graphics.Platform)]] |  |
| [[GraphicsExtensions (Platform)]] |  |
| [[ImageExtensions (Platform)]] |  |
| [[NSAttributedStringExtension]] |  |
| [[NSColorExtensions]] |  |
| [[NSImageExtensions]] |  |
| [[PlatformBitmapExportContext]] |  |
| [[PlatformBitmapExportService (Platform)]] |  |
| [[PlatformCanvas]] |  |
| [[PlatformCanvasState]] |  |
| [[PlatformCanvasStateService]] |  |
| [[PlatformGraphicsView]] |  |
| [[PlatformImage]] | Provides a platform-agnostic implementation of an image. |
| [[PlatformImageLoadingService]] |  |
| [[PlatformStringSizeService]] |  |
| [[UIColorExtensions]] |  |
| [[UIImageExtensions]] |  |
| [[UIKitExtensions]] |  |
| [[UIViewExtensions (Platform)]] |  |


## Classes

| Type | Summary |
|---|---|
| [[AppKitConstants\|AppKitConstants]] |  |
| [[AttributedTextExtensions (Platform)\|AttributedTextExtensions (Platform)]] |  |
| [[CGColorExtensions\|CGColorExtensions]] |  |
| [[CgContextExtensions\|CgContextExtensions]] |  |
| [[ColorExtensions (Microsoft.Maui.Graphics.Platform)\|ColorExtensions (Microsoft.Maui.Graphics.Platform)]] |  |
| [[CoreGraphicsExtensions (Microsoft.Maui.Graphics.Platform)\|CoreGraphicsExtensions (Microsoft.Maui.Graphics.Platform)]] |  |
| [[DirectRenderer\|DirectRenderer]] |  |
| [[FontExtensions (Microsoft.Maui.Graphics.Platform)\|FontExtensions (Microsoft.Maui.Graphics.Platform)]] |  |
| [[GraphicsExtensions (Platform)\|GraphicsExtensions (Platform)]] |  |
| [[ImageExtensions (Platform)\|ImageExtensions (Platform)]] |  |
| [[NSAttributedStringExtension\|NSAttributedStringExtension]] |  |
| [[NSColorExtensions\|NSColorExtensions]] |  |
| [[NSImageExtensions\|NSImageExtensions]] |  |
| [[PlatformBitmapExportContext\|PlatformBitmapExportContext]] |  |
| [[PlatformBitmapExportService (Platform)\|PlatformBitmapExportService (Platform)]] |  |
| [[PlatformCanvas\|PlatformCanvas]] |  |
| [[PlatformCanvasState\|PlatformCanvasState]] |  |
| [[PlatformCanvasStateService\|PlatformCanvasStateService]] |  |
| [[PlatformGraphicsView\|PlatformGraphicsView]] |  |
| [[PlatformImage\|PlatformImage]] | Provides a platform-agnostic implementation of an image. |
| [[PlatformImageLoadingService\|PlatformImageLoadingService]] |  |
| [[PlatformStringSizeService\|PlatformStringSizeService]] |  |
| [[UIColorExtensions\|UIColorExtensions]] |  |
| [[UIImageExtensions\|UIImageExtensions]] |  |
| [[UIKitExtensions\|UIKitExtensions]] |  |
| [[UIViewExtensions (Platform)\|UIViewExtensions (Platform)]] |  |

## Interfaces

| Type | Summary |
|---|---|
| [[IGraphicsRenderer\|IGraphicsRenderer]] |  |

## See also

- [[_API Reference]]
