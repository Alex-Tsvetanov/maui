---
title: "SkiaImage"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Graphics-Skia
aliases:
  - "Microsoft.Maui.Graphics.Skia.SkiaImage"
namespace: "Microsoft.Maui.Graphics.Skia"
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

# SkiaImage

> [!abstract] Class in `Microsoft.Maui.Graphics.Skia`
> Full name: `Microsoft.Maui.Graphics.Skia.SkiaImage`

Represents an image implementation using SkiaSharp.

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
| [[SkiaImage.SkiaImage\|SkiaImage]] | Initializes a new instance of the `SkiaImage` class with the specified SkiaSharp bitmap. |

## Properties

| Name | Summary |
|---|---|
| [[SkiaImage.Height\|Height]] | Gets the height of the image in pixels. |
| [[SkiaImage.PlatformRepresentation\|PlatformRepresentation]] | Gets the underlying SkiaSharp bitmap that this image wraps. |
| [[SkiaImage.Width\|Width]] | Gets the width of the image in pixels. |

## Methods

| Name | Summary |
|---|---|
| [[SkiaImage.Dispose\|Dispose]] | Releases all resources used by this image. |
| [[SkiaImage.Downsize\|Downsize]] | Creates a new image by downsizing this image to fit within the specified maximum dimension. |
| [[SkiaImage.Draw\|Draw]] | Draws this image on the specified canvas within the specified rectangle. |
| [[SkiaImage.FromStream\|FromStream]] | Creates a new image from a stream. |
| [[SkiaImage.Resize\|Resize]] | Creates a new image by resizing this image to the specified dimensions. |
| [[SkiaImage.Save\|Save]] | Saves the image to a stream in the specified format. |
| [[SkiaImage.SaveAsync\|SaveAsync]] | Asynchronously saves the image to a stream in the specified format. |
| [[SkiaImage.ToPlatformImage\|ToPlatformImage]] | Creates a platform-specific image from this image. |

## See also

- [[_Microsoft.Maui.Graphics.Skia|Microsoft.Maui.Graphics.Skia namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.graphics.skia.skiaimage)
