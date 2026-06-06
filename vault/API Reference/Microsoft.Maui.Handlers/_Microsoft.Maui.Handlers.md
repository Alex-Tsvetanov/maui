---
title: "Microsoft.Maui.Handlers"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Handlers
---

# Microsoft.Maui.Handlers

> [!info] Namespace
> `Microsoft.Maui.Handlers` — 98 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.handlers)

## Overview

`Microsoft.Maui.Handlers` contains the **handlers** that bridge .NET MAUI's abstract, cross-platform views to their concrete native controls on each platform. In MAUI's architecture a control is described by a platform-agnostic interface (for example `IButton`, `ILabel`, or `IDatePicker`), and a handler is the object responsible for creating, configuring, and updating the real platform widget that backs it. This decoupling lets the cross-platform layer stay free of platform-specific code while each handler owns the mapping to UIKit, Android views, WinUI, and so on.

The hierarchy is rooted in [[ElementHandler]] and its generic form [[ElementHandler{TVirtualView, TPlatformView}]], which establish the connection between a *virtual view* (the MAUI abstraction) and a *platform view* (the native control). [[ViewHandler]] and its generic counterpart [[ViewHandler{TVirtualView, TPlatformView}]] build on this base to manage anything implementing `IView`, adding layout, measurement, and property-mapping infrastructure. Most concrete handlers in this namespace derive from these bases.

From there the namespace provides a dedicated handler for nearly every built-in control: [[ButtonHandler]], [[LabelHandler]], [[EntryHandler]], [[EditorHandler]], [[ImageHandler]], [[DatePickerHandler]], [[CheckBoxHandler]], [[LayoutHandler]], [[ScrollViewHandler]], [[WebViewHandler]], and many more. Each handler typically has a matching interface (such as `IButtonHandler`) so consumers can depend on the abstraction. Application- and window-level pieces like [[ApplicationHandler]], [[WindowHandler]], and [[PageHandler]] complete the surface, making this namespace the central extension point for customizing or replacing native rendering across MAUI.

> [!tip] To customize how a control renders natively, register a custom handler or add to an existing handler's property and command mappers rather than subclassing the native view directly.

## Key types

- [[ElementHandler]] — Non-generic base for handlers that connect a virtual view to its platform view.
- [[ViewHandler]] — Base class for handlers that manage views which implement `IView`.
- [[ViewHandler{TVirtualView, TPlatformView}]] — Strongly typed base mapping a specific virtual view to a specific platform view.
- [[ButtonHandler]] — Handler for the cross-platform `IButton` control.
- [[LabelHandler]] — Handler for the cross-platform `ILabel` control.
- [[EntryHandler]] — Handler for the single-line text `IEntry` control.
- [[ImageHandler]] — Handler for the `IImage` control.
- [[LayoutHandler]] — View handler for the abstract `ILayout` view and its platform implementation.
- [[ScrollViewHandler]] — Handler for the scrollable `IScrollView` control.
- [[PageHandler]] — Handler for MAUI page content.
- [[WindowHandler]] — Handler that backs an `IWindow` with the platform window.
- [[ApplicationHandler]] — View handler for the abstract `IApplication` and its platform implementation.

## Related guides

- [[_API Reference]]


## Classes

| Type | Summary |
|---|---|
| [[ActivityIndicatorHandler\|ActivityIndicatorHandler]] | Represents the view handler for the abstract `IActivityIndicator` view and its platform-specific implementation. |
| [[ApplicationHandler\|ApplicationHandler]] | Represents the view handler for the abstract `IApplication` view and its platform-specific implementation. |
| [[BorderHandler\|BorderHandler]] | Represents the view handler for the abstract `IBorderView` view and its platform-specific implementation. |
| [[ButtonHandler\|ButtonHandler]] |  |
| [[CheckBoxHandler\|CheckBoxHandler]] | Represents the view handler for the abstract `ICheckBox` view and its platform-specific implementation. |
| [[ContentViewHandler\|ContentViewHandler]] | Represents the view handler for the abstract `IContentView` view and its platform-specific implementation. |
| [[DataFlowDirection\|DataFlowDirection]] |  |
| [[DatePickerHandler\|DatePickerHandler]] | Represents the view handler for the abstract `IDatePicker` view and its platform-specific implementation. |
| [[EditorHandler\|EditorHandler]] |  |
| [[ElementHandler\|ElementHandler]] |  |
| [[ElementHandler{TVirtualView, TPlatformView}\|ElementHandler<TVirtualView, TPlatformView>]] |  |
| [[EntryHandler\|EntryHandler]] |  |
| [[FlyoutViewHandler\|FlyoutViewHandler]] |  |
| [[GraphicsViewHandler\|GraphicsViewHandler]] |  |
| [[HybridWebViewHandler\|HybridWebViewHandler]] |  |
| [[ImageButtonHandler\|ImageButtonHandler]] |  |
| [[ImageHandler\|ImageHandler]] |  |
| [[IndicatorViewHandler\|IndicatorViewHandler]] |  |
| [[LabelHandler\|LabelHandler]] |  |
| [[LayoutHandler\|LayoutHandler]] | Represents the view handler for the abstract `ILayout` view and its platform-specific implementation. |
| [[MenuBarHandler\|MenuBarHandler]] |  |
| [[MenuBarItemHandler\|MenuBarItemHandler]] |  |
| [[MenuFlyoutHandler\|MenuFlyoutHandler]] |  |
| [[MenuFlyoutItemHandler\|MenuFlyoutItemHandler]] |  |
| [[MenuFlyoutSeparatorHandler\|MenuFlyoutSeparatorHandler]] |  |
| [[MenuFlyoutSubItemHandler\|MenuFlyoutSubItemHandler]] |  |
| [[NavigationViewHandler\|NavigationViewHandler]] |  |
| [[OpenWindowRequest\|OpenWindowRequest]] |  |
| [[PageHandler\|PageHandler]] |  |
| [[PickerHandler\|PickerHandler]] |  |
| [[PickerSource\|PickerSource]] |  |
| [[ProgressBarHandler\|ProgressBarHandler]] |  |
| [[RadioButtonHandler\|RadioButtonHandler]] |  |
| [[RefreshViewHandler\|RefreshViewHandler]] |  |
| [[ScrollViewHandler\|ScrollViewHandler]] |  |
| [[SearchBarHandler\|SearchBarHandler]] |  |
| [[ShapeViewHandler\|ShapeViewHandler]] |  |
| [[SliderHandler\|SliderHandler]] |  |
| [[StepperHandler\|StepperHandler]] |  |
| [[SwipeItemButton\|SwipeItemButton]] |  |
| [[SwipeItemMenuItemHandler\|SwipeItemMenuItemHandler]] |  |
| [[SwipeItemViewHandler\|SwipeItemViewHandler]] |  |
| [[SwipeViewHandler\|SwipeViewHandler]] |  |
| [[SwitchHandler\|SwitchHandler]] |  |
| [[TabbedViewHandler\|TabbedViewHandler]] |  |
| [[TimePickerHandler\|TimePickerHandler]] |  |
| [[ToolbarHandler\|ToolbarHandler]] |  |
| [[ViewHandler\|ViewHandler]] | Base class for handlers that manage views which implement `IView`. |
| [[ViewHandler{TVirtualView, TPlatformView}\|ViewHandler<TVirtualView, TPlatformView>]] | Base class for handlers that manage views which implement `IView`. |
| [[WebViewHandler\|WebViewHandler]] |  |
| [[WindowHandler\|WindowHandler]] |  |

## Interfaces

| Type | Summary |
|---|---|
| [[IActivityIndicatorHandler\|IActivityIndicatorHandler]] |  |
| [[IBorderHandler\|IBorderHandler]] |  |
| [[IButtonHandler\|IButtonHandler]] |  |
| [[ICheckBoxHandler\|ICheckBoxHandler]] |  |
| [[IContentViewHandler\|IContentViewHandler]] |  |
| [[IDatePickerHandler\|IDatePickerHandler]] |  |
| [[IEditorHandler\|IEditorHandler]] |  |
| [[IEntryHandler\|IEntryHandler]] |  |
| [[IFlyoutViewHandler\|IFlyoutViewHandler]] |  |
| [[IGraphicsViewHandler\|IGraphicsViewHandler]] |  |
| [[IHybridPlatformWebView\|IHybridPlatformWebView]] |  |
| [[IHybridWebViewHandler\|IHybridWebViewHandler]] |  |
| [[IImageButtonHandler\|IImageButtonHandler]] |  |
| [[IImageHandler\|IImageHandler]] |  |
| [[IIndicatorViewHandler\|IIndicatorViewHandler]] |  |
| [[ILabelHandler\|ILabelHandler]] |  |
| [[IMenuBarHandler\|IMenuBarHandler]] |  |
| [[IMenuBarItemHandler\|IMenuBarItemHandler]] |  |
| [[IMenuFlyoutHandler\|IMenuFlyoutHandler]] |  |
| [[IMenuFlyoutItemHandler\|IMenuFlyoutItemHandler]] |  |
| [[IMenuFlyoutSeparatorHandler\|IMenuFlyoutSeparatorHandler]] |  |
| [[IMenuFlyoutSubItemHandler\|IMenuFlyoutSubItemHandler]] |  |
| [[INavigationViewHandler\|INavigationViewHandler]] |  |
| [[IPageHandler\|IPageHandler]] |  |
| [[IPickerHandler\|IPickerHandler]] |  |
| [[IProgressBarHandler\|IProgressBarHandler]] |  |
| [[IRadioButtonHandler\|IRadioButtonHandler]] |  |
| [[IRefreshViewHandler\|IRefreshViewHandler]] |  |
| [[IScrollViewHandler\|IScrollViewHandler]] |  |
| [[ISearchBarHandler\|ISearchBarHandler]] |  |
| [[IShapeViewHandler\|IShapeViewHandler]] |  |
| [[ISliderHandler\|ISliderHandler]] |  |
| [[IStepperHandler\|IStepperHandler]] |  |
| [[ISwipeItemMenuItemHandler\|ISwipeItemMenuItemHandler]] |  |
| [[ISwipeItemViewHandler\|ISwipeItemViewHandler]] |  |
| [[ISwipeViewHandler\|ISwipeViewHandler]] |  |
| [[ISwitchHandler\|ISwitchHandler]] |  |
| [[ITabbedViewHandler\|ITabbedViewHandler]] |  |
| [[ITimePickerHandler\|ITimePickerHandler]] |  |
| [[IToolbarHandler\|IToolbarHandler]] |  |
| [[IWebViewHandler\|IWebViewHandler]] |  |
| [[IWindowHandler\|IWindowHandler]] |  |

## See also

- [[_API Reference]]
