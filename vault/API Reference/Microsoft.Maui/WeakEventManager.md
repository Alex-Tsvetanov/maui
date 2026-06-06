---
title: "WeakEventManager"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.WeakEventManager"
namespace: "Microsoft.Maui"
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

# WeakEventManager

> [!abstract] Class in `Microsoft.Maui`
> Full name: `Microsoft.Maui.WeakEventManager`

Manages weak event subscriptions, preventing memory leaks by maintaining weak references to handlers.

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
| [[WeakEventManager.WeakEventManager\|WeakEventManager]] |  |

## Methods

| Name | Summary |
|---|---|
| [[WeakEventManager.AddEventHandler\|AddEventHandler]] | Adds an event handler for the specified event, storing a weak reference to the handler's target. |
| [[WeakEventManager.AddEventHandler{TEventArgs}\|AddEventHandler<TEventArgs>]] |  |
| [[WeakEventManager.HandleEvent\|HandleEvent]] | Invokes the handlers registered for the specified event. Removes handlers whose targets have been garbage collected. |
| [[WeakEventManager.RemoveEventHandler\|RemoveEventHandler]] | Removes a previously added event handler for the specified event. |
| [[WeakEventManager.RemoveEventHandler{TEventArgs}\|RemoveEventHandler<TEventArgs>]] |  |

## See also

- [[_Microsoft.Maui|Microsoft.Maui namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.weakeventmanager)
