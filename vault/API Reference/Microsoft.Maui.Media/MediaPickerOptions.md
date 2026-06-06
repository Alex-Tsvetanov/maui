---
title: "MediaPickerOptions"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Media
aliases:
  - "Microsoft.Maui.Media.MediaPickerOptions"
namespace: "Microsoft.Maui.Media"
kind: class
platforms:
  - All platforms (.NET)
  - Android
  - iOS
  - Mac Catalyst
  - Windows
  - Tizen
  - .NET Standard
assemblies:
  - src
---

# MediaPickerOptions

> [!abstract] Class in `Microsoft.Maui.Media`
> Full name: `Microsoft.Maui.Media.MediaPickerOptions`

Pick options for picking media from the device.

## Platforms

| Platform | Available |
|---|---|
| All platforms (.NET) | ✅ |
| Android | ✅ |
| iOS | ✅ |
| Mac Catalyst | ✅ |
| Windows | ✅ |
| Tizen | ✅ |
| .NET Standard | ✅ |


## Constructors

| Name | Summary |
|---|---|
| [[MediaPickerOptions.MediaPickerOptions\|MediaPickerOptions]] |  |

## Properties

| Name | Summary |
|---|---|
| [[MediaPickerOptions.CompressionQuality\|CompressionQuality]] |  |
| [[MediaPickerOptions.MaximumHeight\|MaximumHeight]] | Gets or sets the maximum height for image resizing. When set, images will be resized to fit within this height while preserving aspect ratio. A value of 0 or… |
| [[MediaPickerOptions.MaximumWidth\|MaximumWidth]] | Gets or sets the compression quality for picked media. The value should be between 0 and 100, where 0 is the lowest quality (most compression) and 100 is the… |
| [[MediaPickerOptions.PreserveMetaData\|PreserveMetaData]] | Gets or sets whether to preserve metadata (including EXIF data) when processing images. When true, metadata from the original image will be preserved in the … |
| [[MediaPickerOptions.RotateImage\|RotateImage]] | Gets or sets whether to automatically rotate the image based on EXIF orientation data. When true, the image will be rotated to the correct orientation. Defau… |
| [[MediaPickerOptions.SelectionLimit\|SelectionLimit]] | Gets or sets the maximum number of items that can be selected. Default value is 1. |
| [[MediaPickerOptions.Title\|Title]] | Gets or sets the title that is displayed when picking media. |

## See also

- [[_Microsoft.Maui.Media|Microsoft.Maui.Media namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.media.mediapickeroptions)
