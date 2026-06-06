---
title: "TemplatedView"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.TemplatedView"
namespace: "Microsoft.Maui.Controls"
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

# TemplatedView

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.TemplatedView`

A view that displays content with a control template, and the base class for `ContentView`.

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
| [[TemplatedView.TemplatedView\|TemplatedView]] |  |

## Properties

| Name | Summary |
|---|---|
| [[TemplatedView.CascadeInputTransparent\|CascadeInputTransparent]] |  |
| [[TemplatedView.Children\|Children]] |  |
| [[TemplatedView.ControlTemplate\|ControlTemplate]] |  |
| [[TemplatedView.IsClippedToBounds\|IsClippedToBounds]] |  |
| [[TemplatedView.Padding\|Padding]] |  |

## Methods

| Name | Summary |
|---|---|
| [[TemplatedView.ArrangeOverride\|ArrangeOverride]] |  |
| [[TemplatedView.ComputeConstraintForView\|ComputeConstraintForView]] |  |
| [[TemplatedView.GetTemplateChild\|GetTemplateChild]] |  |
| [[TemplatedView.InvalidateLayout\|InvalidateLayout]] | Invalidates the current layout. |
| [[TemplatedView.LayoutChildren\|LayoutChildren]] |  |
| [[TemplatedView.LowerChild\|LowerChild]] | Sends a child to the back of the visual stack. |
| [[TemplatedView.MeasureOverride\|MeasureOverride]] |  |
| [[TemplatedView.OnApplyTemplate\|OnApplyTemplate]] |  |
| [[TemplatedView.OnChildMeasureInvalidated\|OnChildMeasureInvalidated]] | Invoked whenever a child of the layout has emitted `MeasureInvalidated`. Implement this method to add class handling for this event. |
| [[TemplatedView.OnChildRemoved\|OnChildRemoved]] |  |
| [[TemplatedView.OnMeasure\|OnMeasure]] |  |
| [[TemplatedView.OnSizeAllocated\|OnSizeAllocated]] |  |
| [[TemplatedView.RaiseChild\|RaiseChild]] | Sends a child to the front of the visual stack. |
| [[TemplatedView.ResolveControlTemplate\|ResolveControlTemplate]] | Resolves and returns the `ControlTemplate` associated with this instance. |
| [[TemplatedView.ShouldInvalidateOnChildAdded\|ShouldInvalidateOnChildAdded]] | If you want to influence invalidation override InvalidateMeasureOverride. This method will no longer work on .NET 10 and later. |
| [[TemplatedView.ShouldInvalidateOnChildRemoved\|ShouldInvalidateOnChildRemoved]] | If you want to influence invalidation override InvalidateMeasureOverride. This method will no longer work on .NET 10 and later. |
| [[TemplatedView.UpdateChildrenLayout\|UpdateChildrenLayout]] | Use InvalidateMeasure depending on your scenario. This method will no longer work on .NET 10 and later. |

## Fields

| Name | Summary |
|---|---|
| [[TemplatedView.CascadeInputTransparentProperty\|CascadeInputTransparentProperty]] | Gets or sets a value which determines if the layout should clip its children to its bounds. The default value is `false`. |
| [[TemplatedView.ControlTemplateProperty\|ControlTemplateProperty]] | Bindable property for `ControlTemplate`. |
| [[TemplatedView.IsClippedToBoundsProperty\|IsClippedToBoundsProperty]] | Bindable property for `IsClippedToBounds`. |
| [[TemplatedView.PaddingProperty\|PaddingProperty]] | Gets or sets a value that controls whether child elements inherit the input transparency of this layout when the transparency is `true`. |

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.templatedview)
