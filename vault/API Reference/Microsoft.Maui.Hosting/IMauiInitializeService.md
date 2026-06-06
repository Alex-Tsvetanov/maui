---
title: "IMauiInitializeService"
tags:
  - api
  - kind/interface
  - ns/Microsoft-Maui-Hosting
aliases:
  - "Microsoft.Maui.Hosting.IMauiInitializeService"
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

# IMauiInitializeService

> [!abstract] Interface in `Microsoft.Maui.Hosting`
> Full name: `Microsoft.Maui.Hosting.IMauiInitializeService`

Represents a service that is initialized during the application construction.

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
| [[IMauiInitializeService.Initialize\|Initialize]] |  |

## Remarks

This service is initialized during the MauiAppBuilder.Build() method. It is executed once per application using the root service provider.

## See also

- [[_Microsoft.Maui.Hosting|Microsoft.Maui.Hosting namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.hosting.imauiinitializeservice)
