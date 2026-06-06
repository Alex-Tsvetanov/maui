---
title: "ReorderableItemsView"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ReorderableItemsView"
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

# ReorderableItemsView

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.ReorderableItemsView`

A `GroupableItemsView` that supports reordering of items through user interaction.

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
| [[ReorderableItemsView.ReorderableItemsView\|ReorderableItemsView]] |  |

## Properties

| Name | Summary |
|---|---|
| [[ReorderableItemsView.CanMixGroups\|CanMixGroups]] |  |
| [[ReorderableItemsView.CanReorderItems\|CanReorderItems]] |  |

## Methods

| Name | Summary |
|---|---|
| [[ReorderableItemsView.SendReorderCompleted\|SendReorderCompleted]] | Gets or sets a value indicating whether items in the collection can be reordered by the user. |

## Events

| Name | Summary |
|---|---|
| [[ReorderableItemsView.ReorderCompleted\|ReorderCompleted]] | Occurs when a reorder operation has been completed. |

## Fields

| Name | Summary |
|---|---|
| [[ReorderableItemsView.CanMixGroupsProperty\|CanMixGroupsProperty]] | Bindable property for `CanMixGroups`. |
| [[ReorderableItemsView.CanReorderItemsProperty\|CanReorderItemsProperty]] | Gets or sets a value indicating whether items from different groups can be mixed together during reordering. |

## Remarks

This class extends `GroupableItemsView` to provide reordering capabilities. Use `CanReorderItems` to enable or disable reordering functionality. When items are grouped, use `CanMixGroups` to control whether items can be moved between groups.

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.reorderableitemsview)
