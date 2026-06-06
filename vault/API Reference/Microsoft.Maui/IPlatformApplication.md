---
title: "IPlatformApplication"
tags:
  - api
  - kind/interface
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.IPlatformApplication"
namespace: "Microsoft.Maui"
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

# IPlatformApplication

> [!abstract] Interface in `Microsoft.Maui`
> Full name: `Microsoft.Maui.IPlatformApplication`

Represents the platform-specific application instance that hosts a .NET MAUI application.

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
| [[IPlatformApplication.Application\|Application]] | Gets the .NET MAUI application instance. |
| [[IPlatformApplication.Current\|Current]] | Gets or sets the current platform application instance. |
| [[IPlatformApplication.Services\|Services]] | Gets the dependency injection service provider for the platform application. |

## Remarks

This interface provides access to platform-specific services and the main application instance. Each platform (Android, iOS, Windows, etc.) provides its own implementation of this interface. Use the IPlatformApplication.Current property to access the current platform application instance.

## See also

- [[_Microsoft.Maui|Microsoft.Maui namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.iplatformapplication)
