---
title: "ApplicationHandler"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Handlers
aliases:
  - "Microsoft.Maui.Handlers.ApplicationHandler"
namespace: "Microsoft.Maui.Handlers"
kind: class
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

# ApplicationHandler

> [!abstract] Class in `Microsoft.Maui.Handlers`
> Full name: `Microsoft.Maui.Handlers.ApplicationHandler`

Represents the view handler for the abstract `IApplication` view and its platform-specific implementation.

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


## Constructors

| Name | Summary |
|---|---|
| [[ApplicationHandler.ApplicationHandler\|ApplicationHandler]] |  |

## Methods

| Name | Summary |
|---|---|
| [[ApplicationHandler.CreatePlatformElement\|CreatePlatformElement]] |  |
| [[ApplicationHandler.MapActivateWindow\|MapActivateWindow]] | Maps the abstract `ActivateWindow` command to the platform-specific implementations. |
| [[ApplicationHandler.MapCloseWindow\|MapCloseWindow]] | Maps the abstract `CloseWindow` command to the platform-specific implementations. |
| [[ApplicationHandler.MapOpenWindow\|MapOpenWindow]] | Maps the abstract `OpenWindow` command to the platform-specific implementations. |
| [[ApplicationHandler.MapTerminate\|MapTerminate]] | Maps the abstract "Terminate" command to the platform-specific implementations. |

## Fields

| Name | Summary |
|---|---|
| [[ApplicationHandler.CommandMapper\|CommandMapper]] |  |
| [[ApplicationHandler.Mapper\|Mapper]] |  |

## See also

- [[_Microsoft.Maui.Handlers|Microsoft.Maui.Handlers namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.handlers.applicationhandler)
