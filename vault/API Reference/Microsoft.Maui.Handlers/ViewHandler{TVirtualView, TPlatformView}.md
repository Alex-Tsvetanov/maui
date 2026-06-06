---
title: "ViewHandler<TVirtualView, TPlatformView>"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Handlers
aliases:
  - "Microsoft.Maui.Handlers.ViewHandler<TVirtualView, TPlatformView>"
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

# ViewHandler<TVirtualView, TPlatformView>

> [!abstract] Class in `Microsoft.Maui.Handlers`
> Full name: `Microsoft.Maui.Handlers.ViewHandler<TVirtualView, TPlatformView>`

Base class for handlers that manage views which implement `IView`.

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
| [[ViewHandler{TVirtualView, TPlatformView}.ViewHandler\|ViewHandler]] | Initializes a new instance of the `ViewHandler` class. |

## Properties

| Name | Summary |
|---|---|
| [[ViewHandler{TVirtualView, TPlatformView}.ContainerView\|ContainerView]] | Gets the view that acts as a container for the `PlatformView`. |
| [[ViewHandler{TVirtualView, TPlatformView}.Context\|Context]] |  |
| [[ViewHandler{TVirtualView, TPlatformView}.NeedsContainer\|NeedsContainer]] |  |
| [[ViewHandler{TVirtualView, TPlatformView}.PlatformView\|PlatformView]] |  |
| [[ViewHandler{TVirtualView, TPlatformView}.PlatformViewFactory\|PlatformViewFactory]] |  |
| [[ViewHandler{TVirtualView, TPlatformView}.ViewController\|ViewController]] |  |
| [[ViewHandler{TVirtualView, TPlatformView}.VirtualView\|VirtualView]] |  |

## Methods

| Name | Summary |
|---|---|
| [[ViewHandler{TVirtualView, TPlatformView}.ConnectHandler\|ConnectHandler]] |  |
| [[ViewHandler{TVirtualView, TPlatformView}.CreatePlatformView\|CreatePlatformView]] |  |
| [[ViewHandler{TVirtualView, TPlatformView}.DisconnectHandler\|DisconnectHandler]] |  |
| [[ViewHandler{TVirtualView, TPlatformView}.Dispose\|Dispose]] |  |
| [[ViewHandler{TVirtualView, TPlatformView}.GetDesiredSize\|GetDesiredSize]] |  |
| [[ViewHandler{TVirtualView, TPlatformView}.Measure\|Measure]] |  |
| [[ViewHandler{TVirtualView, TPlatformView}.OnPlatformViewDeleted\|OnPlatformViewDeleted]] |  |
| [[ViewHandler{TVirtualView, TPlatformView}.PlatformArrange\|PlatformArrange]] |  |
| [[ViewHandler{TVirtualView, TPlatformView}.RemoveContainer\|RemoveContainer]] | Deconstructs the `ContainerView` and removes `PlatformView` from its container. |
| [[ViewHandler{TVirtualView, TPlatformView}.SetVirtualView\|SetVirtualView]] |  |
| [[ViewHandler{TVirtualView, TPlatformView}.SetupContainer\|SetupContainer]] | Gets or sets a value that indicates whether the `PlatformView` is contained within a view. |
| [[ViewHandler{TVirtualView, TPlatformView}.~ViewHandler\|~ViewHandler]] |  |

## Remarks

Handlers map virtual views (.NET MAUI layer) to controls on each platform (iOS, Android, Windows, macOS, etc.), which are known as platform views. Handlers are also responsible for instantiating the underlying platform view, and mapping the cross-platform control API to the platform view API.

## See also

- [[_Microsoft.Maui.Handlers|Microsoft.Maui.Handlers namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.handlers.viewhandler)
