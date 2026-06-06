---
title: "PlatformImage"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Graphics-Platform
aliases:
  - "Microsoft.Maui.Graphics.Platform.PlatformImage"
namespace: "Microsoft.Maui.Graphics.Platform"
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

# PlatformImage

> [!abstract] Class in `Microsoft.Maui.Graphics.Platform`
> Full name: `Microsoft.Maui.Graphics.Platform.PlatformImage`

Provides a platform-agnostic implementation of an image.

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
| [[PlatformImage.PlatformImage\|PlatformImage]] | Initializes a new instance of the `PlatformImage` class with the specified bytes and format. |

## Properties

| Name | Summary |
|---|---|
| [[PlatformImage.Bytes\|Bytes]] | Gets the raw image data. |
| [[PlatformImage.Height\|Height]] | Gets the height of the image. |
| [[PlatformImage.NativeRepresentation\|NativeRepresentation]] |  |
| [[PlatformImage.PlatformRepresentation\|PlatformRepresentation]] |  |
| [[PlatformImage.Width\|Width]] | Gets the width of the image. |

## Methods

| Name | Summary |
|---|---|
| [[PlatformImage.Dispose\|Dispose]] | Releases all resources used by the image. |
| [[PlatformImage.Downsize\|Downsize]] | Downsizes the image to fit within the specified maximum dimension while maintaining aspect ratio. |
| [[PlatformImage.Draw\|Draw]] |  |
| [[PlatformImage.FromStream\|FromStream]] |  |
| [[PlatformImage.Resize\|Resize]] | Resizes the image to the specified dimensions using the specified resize mode. |
| [[PlatformImage.Save\|Save]] | Saves the contents of this image to the provided `Stream` object. |
| [[PlatformImage.SaveAsync\|SaveAsync]] |  |
| [[PlatformImage.ToImage\|ToImage]] |  |
| [[PlatformImage.ToPlatformImage\|ToPlatformImage]] |  |

## See also

- [[_Microsoft.Maui.Graphics.Platform|Microsoft.Maui.Graphics.Platform namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.graphics.platform.platformimage)
