---
title: "IMauiInitializeScopedService"
tags:
  - api
  - kind/interface
  - ns/Microsoft-Maui-Hosting
aliases:
  - "Microsoft.Maui.Hosting.IMauiInitializeScopedService"
namespace: "Microsoft.Maui.Hosting"
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

# IMauiInitializeScopedService

> [!abstract] Interface in `Microsoft.Maui.Hosting`
> Full name: `Microsoft.Maui.Hosting.IMauiInitializeScopedService`

Represents a service that is initialized during the window construction.

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


## Methods

| Name | Summary |
|---|---|
| [[IMauiInitializeScopedService.Initialize\|Initialize]] |  |

## Remarks

This service is initialized during the creation of a window. It is executed once per window using the window-scoped service provider.

## See also

- [[_Microsoft.Maui.Hosting|Microsoft.Maui.Hosting namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.hosting.imauiinitializescopedservice)
