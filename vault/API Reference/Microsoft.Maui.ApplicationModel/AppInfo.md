---
title: "AppInfo"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.AppInfo"
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

# AppInfo

> [!abstract] Class in `Microsoft.Maui.ApplicationModel`
> Full name: `Microsoft.Maui.ApplicationModel.AppInfo`

Gets the application package name or identifier.

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
| [[AppInfo.BuildString\|BuildString]] | Gets the application build number. |
| [[AppInfo.Current\|Current]] | Provides the default implementation for static usage of this API. |
| [[AppInfo.Name\|Name]] | Gets the application name. |
| [[AppInfo.PackageName\|PackageName]] | Gets the application package name or identifier. |
| [[AppInfo.PackagingModel\|PackagingModel]] | Gets the packaging model of this application. |
| [[AppInfo.RequestedLayoutDirection\|RequestedLayoutDirection]] | Gets the requested layout direction of the system or application. |
| [[AppInfo.RequestedTheme\|RequestedTheme]] | Gets the detected theme of the system or application. |
| [[AppInfo.Version\|Version]] | Gets the application version as a `Version` object. |
| [[AppInfo.VersionString\|VersionString]] | Gets the application version as a string representation. |

## Methods

| Name | Summary |
|---|---|
| [[AppInfo.ShowSettingsUI\|ShowSettingsUI]] | Open the settings menu or page for this application. |

## Remarks

On Android and iOS, this is the application package name. On Windows, this is the application GUID.

## See also

- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.applicationmodel.appinfo)
