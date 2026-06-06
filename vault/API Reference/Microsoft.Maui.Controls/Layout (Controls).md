---
title: "Layout (Controls)"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Layout"
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

# Layout (Controls)

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.Layout`

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
| [[Layout (Controls).Layout\|Layout]] |  |

## Properties

| Name | Summary |
|---|---|
| [[Layout (Controls).CascadeInputTransparent\|CascadeInputTransparent]] |  |
| [[Layout (Controls).Children\|Children]] |  |
| [[Layout (Controls).Count\|Count]] | Gets the child object count in this layout. |
| [[Layout (Controls).IgnoreSafeArea\|IgnoreSafeArea]] | Gets or sets the safe area edges to obey for this layout. The default value is SafeAreaEdges.Default (None - edge to edge). |
| [[Layout (Controls).IsClippedToBounds\|IsClippedToBounds]] |  |
| [[Layout (Controls).IsReadOnly\|IsReadOnly]] | Gets whether this layout is readonly. |
| [[Layout (Controls).Padding\|Padding]] |  |
| [[Layout (Controls).SafeAreaEdges\|SafeAreaEdges]] |  |
| [[Layout (Controls).this[int index]\|this[int index]]] |  |

## Methods

| Name | Summary |
|---|---|
| [[Layout (Controls).Add\|Add]] | Adds a child view to the end of this layout. |
| [[Layout (Controls).Clear\|Clear]] | Clears all child views from this layout. |
| [[Layout (Controls).Contains\|Contains]] | Determines if the specified child view is contained in this layout. |
| [[Layout (Controls).CopyTo\|CopyTo]] | Copies the child views to the specified array. |
| [[Layout (Controls).CreateLayoutManager\|CreateLayoutManager]] | Creates a manager object that can measure this layout and arrange its children. |
| [[Layout (Controls).CrossPlatformArrange\|CrossPlatformArrange]] |  |
| [[Layout (Controls).CrossPlatformMeasure\|CrossPlatformMeasure]] |  |
| [[Layout (Controls).GetEnumerator\|GetEnumerator]] |  |
| [[Layout (Controls).IndexOf\|IndexOf]] | Gets the index of a specified child view. |
| [[Layout (Controls).Insert\|Insert]] | Inserts a child view at the specified index. |
| [[Layout (Controls).InvalidateMeasureOverride\|InvalidateMeasureOverride]] |  |
| [[Layout (Controls).MapInputTransparent\|MapInputTransparent]] | Maps the abstract InputTransparent property to the platform-specific implementations. |
| [[Layout (Controls).OnAdd\|OnAdd]] | Invoked when `Add` is called and notifies the handler associated to this layout. |
| [[Layout (Controls).OnClear\|OnClear]] | Invoked when `Clear` is called and notifies the handler associated to this layout. |
| [[Layout (Controls).OnInsert\|OnInsert]] | Invoked when `RemoveAt` is called and notifies the handler associated to this layout. |
| [[Layout (Controls).OnRemove\|OnRemove]] | Invoked when `Insert` is called and notifies the handler associated to this layout. |
| [[Layout (Controls).OnUpdate\|OnUpdate]] | Invoked when `this[int]` is called and notifies the handler associated to this layout. |
| [[Layout (Controls).Remove\|Remove]] | Removes a child view. |
| [[Layout (Controls).RemoveAt\|RemoveAt]] | Removes a child view at the specified index. |

## Fields

| Name | Summary |
|---|---|
| [[Layout (Controls).CascadeInputTransparentProperty\|CascadeInputTransparentProperty]] | Bindable property for `CascadeInputTransparent`. |
| [[Layout (Controls).IsClippedToBoundsProperty\|IsClippedToBoundsProperty]] | Bindable property for `IsClippedToBounds`. |
| [[Layout (Controls).PaddingProperty\|PaddingProperty]] | Gets or sets a value which determines if the layout should clip its children to its bounds. The default value is `false`. |
| [[Layout (Controls).SafeAreaEdgesProperty\|SafeAreaEdgesProperty]] | Gets or sets the inner padding of the layout. The default value is a `Thickness` with all values set to 0. |
| [[Layout (Controls)._layoutManager\|_layoutManager]] |  |

## Guide

- 📖 Conceptual: [[layout]]

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.layout)
