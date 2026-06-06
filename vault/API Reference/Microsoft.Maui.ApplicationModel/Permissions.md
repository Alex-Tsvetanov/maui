---
title: "Permissions"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Permissions"
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

# Permissions

> [!abstract] Class in `Microsoft.Maui.ApplicationModel`
> Full name: `Microsoft.Maui.ApplicationModel.Permissions`

The Permissions API provides the ability to check and request runtime permissions.

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
| [[Permissions.LocationTimeout\|LocationTimeout]] | Gets or sets the timeout that is used when the location permission is requested. |

## Methods

| Name | Summary |
|---|---|
| [[Permissions.CheckStatusAsync{TPermission}\|CheckStatusAsync<TPermission>]] |  |
| [[Permissions.IsCapabilityDeclared\|IsCapabilityDeclared]] | Checks if the capability specified in `capabilityName` is declared in the application's AppxManifest.xml file. |
| [[Permissions.IsDeclaredInManifest\|IsDeclaredInManifest]] |  |
| [[Permissions.IsKeyDeclaredInInfoPlist\|IsKeyDeclaredInInfoPlist]] | Checks if the key specified in `usageKey` is declared in the application's Info.plist file. |
| [[Permissions.IsPrivilegeDeclared\|IsPrivilegeDeclared]] | Checks if the key specified in `tizenPrivilege` is declared in the application's tizen-manifest.xml file. |
| [[Permissions.RequestAsync{TPermission}\|RequestAsync<TPermission>]] |  |
| [[Permissions.ShouldShowRationale{TPermission}\|ShouldShowRationale<TPermission>]] |  |

## Guide

- 📖 Conceptual: [[permissions]]

## See also

- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.applicationmodel.permissions)
