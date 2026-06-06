---
title: "IImageSourcePartSetter"
tags:
  - api
  - kind/interface
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.IImageSourcePartSetter"
namespace: "Microsoft.Maui.Platform"
kind: interface
platforms:
  - All platforms (.NET)
  - Android
  - iOS
  - Mac Catalyst
  - Windows
  - Tizen
  - .NET Standard
  - .NET Standard 2.0
assemblies:
  - src
---

# IImageSourcePartSetter

> [!abstract] Interface in `Microsoft.Maui.Platform`
> Full name: `Microsoft.Maui.Platform.IImageSourcePartSetter`

This represents a object that knows what the desired image is and how to apply a loaded version of the image to a platform view.

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
| .NET Standard 2.0 | ✅ |


## Properties

| Name | Summary |
|---|---|
| [[IImageSourcePartSetter.Handler\|Handler]] |  |
| [[IImageSourcePartSetter.ImageSourcePart\|ImageSourcePart]] |  |

## Methods

| Name | Summary |
|---|---|
| [[IImageSourcePartSetter.SetImageSource\|SetImageSource]] |  |

## Remarks

If a handler has multiple image parts, then multiple `IImageSourcePartSetter` instances can be used for each image part. The handler should not implement this interface itself as is breaks re-use of mappers and/or handlers.

## See also

- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.platform.iimagesourcepartsetter)
