---
title: "BitmapExportContext"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.BitmapExportContext"
namespace: "Microsoft.Maui.Graphics"
kind: class
platforms:
  - All platforms (.NET)
  - Android
  - iOS
  - Mac Catalyst
  - macOS
  - Windows
  - Tizen
  - .NET Standard
assemblies:
  - Graphics
---

# BitmapExportContext

> [!abstract] Class in `Microsoft.Maui.Graphics`
> Full name: `Microsoft.Maui.Graphics.BitmapExportContext`

Provides an abstract base class for bitmap export operations.

## Platforms

| Platform | Available |
|---|---|
| All platforms (.NET) | ✅ |
| Android | ✅ |
| iOS | ✅ |
| Mac Catalyst | ✅ |
| macOS | ✅ |
| Windows | ✅ |
| Tizen | ✅ |
| .NET Standard | ✅ |


## Constructors

| Name | Summary |
|---|---|
| [[BitmapExportContext.BitmapExportContext\|BitmapExportContext]] | Initializes a new instance of the `BitmapExportContext` class with the specified dimensions and resolution. |

## Properties

| Name | Summary |
|---|---|
| [[BitmapExportContext.Canvas\|Canvas]] |  |
| [[BitmapExportContext.Dpi\|Dpi]] | Gets the resolution (dots per inch) of the bitmap. |
| [[BitmapExportContext.Height\|Height]] | Gets the height of the bitmap in pixels. |
| [[BitmapExportContext.Image\|Image]] |  |
| [[BitmapExportContext.Width\|Width]] | Gets the width of the bitmap in pixels. |

## Methods

| Name | Summary |
|---|---|
| [[BitmapExportContext.Dispose\|Dispose]] | Releases all resources used by the `BitmapExportContext`. |
| [[BitmapExportContext.WriteToStream\|WriteToStream]] |  |

## Remarks

This class encapsulates the context for exporting graphics to a bitmap. Platform-specific implementations will handle the actual drawing and image generation.

## See also

- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.graphics.bitmapexportcontext)
