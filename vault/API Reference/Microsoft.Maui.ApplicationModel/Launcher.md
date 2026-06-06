---
title: "Launcher"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Launcher"
namespace: "Microsoft.Maui.ApplicationModel"
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

# Launcher

> [!abstract] Class in `Microsoft.Maui.ApplicationModel`
> Full name: `Microsoft.Maui.ApplicationModel.Launcher`

Queries if the device supports opening the given URI scheme.

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


## Properties

| Name | Summary |
|---|---|
| [[Launcher.Default\|Default]] | Provides the default implementation for static usage of this API. |

## Methods

| Name | Summary |
|---|---|
| [[Launcher.CanOpenAsync\|CanOpenAsync]] |  |
| [[Launcher.OpenAsync\|OpenAsync]] |  |
| [[Launcher.TryOpenAsync\|TryOpenAsync]] |  |

## Remarks

If you are looking to open the browser to a website then you should refer to the `IBrowser` API. On iOS 9+, you will have to specify the LSApplicationQueriesSchemes key in the info.plist file with URI schemes you want to query from your app.

## Guide

- 📖 Conceptual: [[launcher]]

## See also

- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.applicationmodel.launcher)
