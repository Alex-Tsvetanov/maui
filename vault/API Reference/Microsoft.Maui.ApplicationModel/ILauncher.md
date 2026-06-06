---
title: "ILauncher"
tags:
  - api
  - kind/interface
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.ILauncher"
namespace: "Microsoft.Maui.ApplicationModel"
kind: interface
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

# ILauncher

> [!abstract] Interface in `Microsoft.Maui.ApplicationModel`
> Full name: `Microsoft.Maui.ApplicationModel.ILauncher`

The Launcher API enables an application to open a URI by the system. This is often used when deep linking into another application's custom URI schemes.

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


## Methods

| Name | Summary |
|---|---|
| [[ILauncher.CanOpenAsync\|CanOpenAsync]] |  |
| [[ILauncher.OpenAsync\|OpenAsync]] |  |
| [[ILauncher.TryOpenAsync\|TryOpenAsync]] |  |

## Remarks

If you are looking to open the browser to a website then you should refer to the `IBrowser` API. On iOS 9+, you will have to specify the LSApplicationQueriesSchemes key in the info.plist file with URI schemes you want to query from your app.

## See also

- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.applicationmodel.ilauncher)
