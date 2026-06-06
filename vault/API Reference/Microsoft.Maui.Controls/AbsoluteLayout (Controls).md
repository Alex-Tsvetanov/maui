---
title: "AbsoluteLayout (Controls)"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.AbsoluteLayout"
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

# AbsoluteLayout (Controls)

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.AbsoluteLayout`

Positions child elements at absolute positions.

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
| [[AbsoluteLayout (Controls).AbsoluteLayout\|AbsoluteLayout]] |  |

## Methods

| Name | Summary |
|---|---|
| [[AbsoluteLayout (Controls).CreateLayoutManager\|CreateLayoutManager]] |  |
| [[AbsoluteLayout (Controls).GetLayoutBounds\|GetLayoutBounds]] | Gets the layout bounds of a view that will be used to interpret the layout bounds set on it when it is added to the layout. |
| [[AbsoluteLayout (Controls).GetLayoutFlags\|GetLayoutFlags]] | Gets the layout flags of a view that will be used to interpret the layout bounds set on it when it is added to the layout. |
| [[AbsoluteLayout (Controls).OnAdd\|OnAdd]] |  |
| [[AbsoluteLayout (Controls).OnClear\|OnClear]] |  |
| [[AbsoluteLayout (Controls).OnInsert\|OnInsert]] |  |
| [[AbsoluteLayout (Controls).OnRemove\|OnRemove]] |  |
| [[AbsoluteLayout (Controls).OnUpdate\|OnUpdate]] |  |
| [[AbsoluteLayout (Controls).SetLayoutBounds\|SetLayoutBounds]] | Sets the layout bounds of a view that will be used to interpret the layout bounds set on it when it is added to the layout. |
| [[AbsoluteLayout (Controls).SetLayoutFlags\|SetLayoutFlags]] | Sets the layout flags of a view that will be used to interpret the layout bounds set on it when it is added to the layout. |

## Fields

| Name | Summary |
|---|---|
| [[AbsoluteLayout (Controls).AutoSize\|AutoSize]] | A value that indicates that the width or height of the child should be sized to that child's native size. |
| [[AbsoluteLayout (Controls).LayoutBoundsProperty\|LayoutBoundsProperty]] | Bindable property for attached property LayoutBounds . |
| [[AbsoluteLayout (Controls).LayoutFlagsProperty\|LayoutFlagsProperty]] | Bindable property for attached property LayoutFlags . |

## Remarks

Application developers can control the placement of child elements by providing proportional coordinates, device coordinates, or a combination of both, depending on the `AbsoluteLayoutFlags` values that are passed to `SetLayoutFlags` method. When one of the proportional `AbsoluteLayoutFlags` enumeration values is provided, the corresponding X, or Y arguments that range between 0.0 and 1.0 will always cause the child to be displayed completely on screen. That is, you do not need to subtract or add the height or width of a child in order to display it flush with the left, right, top, or bottom of the `AbsoluteLayout`. For width, height, X, or Y values that are not specified proportionally, application developers use device-dependent units to locate and size the child element.

## Guide

- 📖 Conceptual: [[absolutelayout]]

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.absolutelayout)
