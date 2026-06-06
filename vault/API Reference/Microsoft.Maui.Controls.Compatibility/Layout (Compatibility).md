---
title: "Layout (Compatibility)"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls-Compatibility
aliases:
  - "Microsoft.Maui.Controls.Compatibility.Layout"
namespace: "Microsoft.Maui.Controls.Compatibility"
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
  - Controls
---

# Layout (Compatibility)

> [!abstract] Class in `Microsoft.Maui.Controls.Compatibility`
> Full name: `Microsoft.Maui.Controls.Compatibility.Layout`

Base class for layouts that allow you to arrange and group UI controls in your application.

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


## Constructors

| Name | Summary |
|---|---|
| [[Layout (Compatibility).Layout\|Layout]] |  |

## Properties

| Name | Summary |
|---|---|
| [[Layout (Compatibility).CascadeInputTransparent\|CascadeInputTransparent]] |  |
| [[Layout (Compatibility).Children\|Children]] |  |
| [[Layout (Compatibility).IsClippedToBounds\|IsClippedToBounds]] |  |
| [[Layout (Compatibility).Padding\|Padding]] |  |

## Methods

| Name | Summary |
|---|---|
| [[Layout (Compatibility).ArrangeOverride\|ArrangeOverride]] |  |
| [[Layout (Compatibility).CrossPlatformArrange\|CrossPlatformArrange]] |  |
| [[Layout (Compatibility).CrossPlatformMeasure\|CrossPlatformMeasure]] |  |
| [[Layout (Compatibility).ForceLayout\|ForceLayout]] | Forces a layout cycle on the element and all of its descendants. |
| [[Layout (Compatibility).InvalidateLayout\|InvalidateLayout]] | Invalidates the current layout. |
| [[Layout (Compatibility).LayoutChildIntoBoundingRegion\|LayoutChildIntoBoundingRegion]] | Positions a child element into a bounding region while respecting the child elements `HorizontalOptions` and `VerticalOptions`. |
| [[Layout (Compatibility).LayoutChildren\|LayoutChildren]] | Positions and sizes the children of a layout. |
| [[Layout (Compatibility).LowerChild\|LowerChild]] | Sends a child to the back of the visual stack. |
| [[Layout (Compatibility).Measure\|Measure]] |  |
| [[Layout (Compatibility).MeasureOverride\|MeasureOverride]] |  |
| [[Layout (Compatibility).OnChildAdded\|OnChildAdded]] |  |
| [[Layout (Compatibility).OnChildMeasureInvalidated\|OnChildMeasureInvalidated]] | Invoked whenever a child of the layout has emitted `MeasureInvalidated`. Implement this method to add class handling for this event. |
| [[Layout (Compatibility).OnChildRemoved\|OnChildRemoved]] |  |
| [[Layout (Compatibility).OnSizeAllocated\|OnSizeAllocated]] |  |
| [[Layout (Compatibility).RaiseChild\|RaiseChild]] | Sends a child to the front of the visual stack. |
| [[Layout (Compatibility).ShouldInvalidateOnChildAdded\|ShouldInvalidateOnChildAdded]] | When implemented, should return `true` if `child` should call `InvalidateMeasure` when added, and should return `false` if it should not call `InvalidateMeas… |
| [[Layout (Compatibility).ShouldInvalidateOnChildRemoved\|ShouldInvalidateOnChildRemoved]] | When implemented, should return `true` if `child` should call `InvalidateMeasure` when removed, and should return `false` if it should not call `InvalidateMe… |
| [[Layout (Compatibility).UpdateChildrenLayout\|UpdateChildrenLayout]] | Instructs the layout to relayout all of its children. |

## Events

| Name | Summary |
|---|---|
| [[Layout (Compatibility).LayoutChanged\|LayoutChanged]] | Occurs at the end of a layout cycle if any of the child element's `Bounds` have changed. |

## Fields

| Name | Summary |
|---|---|
| [[Layout (Compatibility).CascadeInputTransparentProperty\|CascadeInputTransparentProperty]] | Bindable property for `CascadeInputTransparent`. |
| [[Layout (Compatibility).IsClippedToBoundsProperty\|IsClippedToBoundsProperty]] | Bindable property for `IsClippedToBounds`. |
| [[Layout (Compatibility).PaddingProperty\|PaddingProperty]] | Bindable property for `Padding`. |

## See also

- [[_Microsoft.Maui.Controls.Compatibility|Microsoft.Maui.Controls.Compatibility namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.compatibility.layout)
