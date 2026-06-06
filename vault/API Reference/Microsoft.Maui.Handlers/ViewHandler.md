---
title: "ViewHandler"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Handlers
aliases:
  - "Microsoft.Maui.Handlers.ViewHandler"
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

# ViewHandler

> [!abstract] Class in `Microsoft.Maui.Handlers`
> Full name: `Microsoft.Maui.Handlers.ViewHandler`

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
| [[ViewHandler.ViewHandler\|ViewHandler]] | Initializes a new instance of the `ViewHandler` class. |

## Properties

| Name | Summary |
|---|---|
| [[ViewHandler.ContainerView\|ContainerView]] | Gets the view that acts as a container for the `PlatformView`. |
| [[ViewHandler.HasContainer\|HasContainer]] |  |
| [[ViewHandler.NeedsContainer\|NeedsContainer]] |  |
| [[ViewHandler.PlatformView\|PlatformView]] |  |
| [[ViewHandler.VirtualView\|VirtualView]] |  |

## Methods

| Name | Summary |
|---|---|
| [[ViewHandler.GetDesiredSize\|GetDesiredSize]] |  |
| [[ViewHandler.MapAnchorX\|MapAnchorX]] | Maps a view's abstract `AnchorX` property to the platform-specific implementations. |
| [[ViewHandler.MapAnchorY\|MapAnchorY]] | Maps a view's abstract `AnchorY` property to the platform-specific implementations. |
| [[ViewHandler.MapAutomationId\|MapAutomationId]] | Maps the abstract `AutomationId` property to the platform-specific implementations. |
| [[ViewHandler.MapBackground\|MapBackground]] | Maps the abstract `Background` property to the platform-specific implementations. |
| [[ViewHandler.MapBorderView\|MapBorderView]] | Maps the abstract `Border` property to the platform-specific implementations. |
| [[ViewHandler.MapClip\|MapClip]] | Maps the abstract `Clip` property to the platform-specific implementations. |
| [[ViewHandler.MapContainerView\|MapContainerView]] | Maps the abstract `ContainerView` property to the platform-specific implementations. |
| [[ViewHandler.MapContextFlyout\|MapContextFlyout]] | Maps the abstract `IView` to the platform-specific implementations of a `IContextFlyoutElement`. |
| [[ViewHandler.MapFlowDirection\|MapFlowDirection]] | Maps the abstract `FlowDirection` property to the platform-specific implementations. |
| [[ViewHandler.MapFocus\|MapFocus]] | Maps the abstract `Focus` property to the platform-specific implementations. |
| [[ViewHandler.MapFrame\|MapFrame]] | Maps the abstract `Frame` property to the platform-specific implementations. |
| [[ViewHandler.MapHeight\|MapHeight]] | Maps the abstract `Height` property to the platform-specific implementations. |
| [[ViewHandler.MapInputTransparent\|MapInputTransparent]] | Maps the abstract `InputTransparent` property to the platform-specific implementations. |
| [[ViewHandler.MapInvalidateMeasure\|MapInvalidateMeasure]] | Maps the abstract `InvalidateMeasure` method to the platform-specific implementations. |
| [[ViewHandler.MapIsEnabled\|MapIsEnabled]] | Maps the abstract `IsEnabled` property to the platform-specific implementations. |
| [[ViewHandler.MapMaximumHeight\|MapMaximumHeight]] | Maps the abstract `MaximumHeight` property to the platform-specific implementations. |
| [[ViewHandler.MapMaximumWidth\|MapMaximumWidth]] | Maps the abstract `MaximumWidth` property to the platform-specific implementations. |
| [[ViewHandler.MapMinimumHeight\|MapMinimumHeight]] | Maps the abstract `MinimumHeight` property to the platform-specific implementations. |
| [[ViewHandler.MapMinimumWidth\|MapMinimumWidth]] | Maps the abstract `MinimumWidth` property to the platform-specific implementations. |
| [[ViewHandler.MapOpacity\|MapOpacity]] | Maps the abstract `Opacity` property to the platform-specific implementations. |
| [[ViewHandler.MapRotation\|MapRotation]] | Maps a view's abstract `Rotation` property to the platform-specific implementations. |
| [[ViewHandler.MapRotationX\|MapRotationX]] | Maps a view's abstract `RotationX` property to the platform-specific implementations. |
| [[ViewHandler.MapRotationY\|MapRotationY]] | Maps a view's abstract `RotationY` property to the platform-specific implementations. |
| [[ViewHandler.MapScale\|MapScale]] | Maps a view's abstract `Scale` property to the platform-specific implementations. |
| [[ViewHandler.MapScaleX\|MapScaleX]] | Maps a view's abstract `ScaleX` property to the platform-specific implementations. |
| [[ViewHandler.MapScaleY\|MapScaleY]] | Maps a view's abstract `ScaleY` property to the platform-specific implementations. |
| [[ViewHandler.MapSemantics\|MapSemantics]] | Maps the abstract `Semantics` property to the platform-specific implementations. |
| [[ViewHandler.MapShadow\|MapShadow]] | Maps the abstract `Shadow` property to the platform-specific implementations. |
| [[ViewHandler.MapToolTip\|MapToolTip]] | Maps the abstract `ToolTip` property to the platform-specific implementations. |
| [[ViewHandler.MapToolbar\|MapToolbar]] |  |
| [[ViewHandler.MapTranslationX\|MapTranslationX]] | Maps a view's abstract `TranslationX` property to the platform-specific implementations. |
| [[ViewHandler.MapTranslationY\|MapTranslationY]] | Maps a view's abstract `TranslationY` property to the platform-specific implementations. |
| [[ViewHandler.MapUnfocus\|MapUnfocus]] | Maps the abstract `Unfocus` method to the platform-specific implementations. |
| [[ViewHandler.MapVisibility\|MapVisibility]] | Maps the abstract `Visibility` property to the platform-specific implementations. |
| [[ViewHandler.MapWidth\|MapWidth]] | Maps the abstract `Width` property to the platform-specific implementations. |
| [[ViewHandler.MapZIndex\|MapZIndex]] | Maps the abstract `ZIndex` property to the platform-specific implementations. |
| [[ViewHandler.OnFocused\|OnFocused]] |  |
| [[ViewHandler.OnPlatformViewDeleted\|OnPlatformViewDeleted]] |  |
| [[ViewHandler.OnUnfocused\|OnUnfocused]] |  |
| [[ViewHandler.PlatformArrange\|PlatformArrange]] |  |
| [[ViewHandler.RemoveContainer\|RemoveContainer]] | Deconstructs the `ContainerView` and removes `PlatformView` from its container. |
| [[ViewHandler.SetupContainer\|SetupContainer]] | Gets or sets a value that indicates whether the `PlatformView` is contained within a view. |

## Fields

| Name | Summary |
|---|---|
| [[ViewHandler.ViewCommandMapper\|ViewCommandMapper]] |  |
| [[ViewHandler.ViewMapper\|ViewMapper]] |  |

## Remarks

Handlers map virtual views (.NET MAUI layer) to controls on each platform (iOS, Android, Windows, macOS, etc.), which are known as platform views. Handlers are also responsible for instantiating the underlying platform view, and mapping the cross-platform control API to the platform view API.

## See also

- [[_Microsoft.Maui.Handlers|Microsoft.Maui.Handlers namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.handlers.viewhandler)
