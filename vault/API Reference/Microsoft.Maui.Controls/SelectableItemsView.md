---
title: "SelectableItemsView"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.SelectableItemsView"
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

# SelectableItemsView

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.SelectableItemsView`

A structured items view that supports item selection.

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
| [[SelectableItemsView.SelectableItemsView\|SelectableItemsView]] | Initializes a new instance of the `SelectableItemsView` class. |

## Properties

| Name | Summary |
|---|---|
| [[SelectableItemsView.SelectedItem\|SelectedItem]] |  |
| [[SelectableItemsView.SelectedItems\|SelectedItems]] |  |
| [[SelectableItemsView.SelectionChangedCommand\|SelectionChangedCommand]] |  |
| [[SelectableItemsView.SelectionChangedCommandParameter\|SelectionChangedCommandParameter]] |  |
| [[SelectableItemsView.SelectionMode\|SelectionMode]] |  |

## Methods

| Name | Summary |
|---|---|
| [[SelectableItemsView.OnSelectionChanged\|OnSelectionChanged]] |  |
| [[SelectableItemsView.UpdateSelectedItems\|UpdateSelectedItems]] |  |

## Events

| Name | Summary |
|---|---|
| [[SelectableItemsView.SelectionChanged\|SelectionChanged]] |  |

## Fields

| Name | Summary |
|---|---|
| [[SelectableItemsView.SelectedItemProperty\|SelectedItemProperty]] | Bindable property for `SelectedItem`. |
| [[SelectableItemsView.SelectedItemsProperty\|SelectedItemsProperty]] | Bindable property for `SelectedItems`. |
| [[SelectableItemsView.SelectionChangedCommandParameterProperty\|SelectionChangedCommandParameterProperty]] | Bindable property for `SelectionChangedCommandParameter`. |
| [[SelectableItemsView.SelectionChangedCommandProperty\|SelectionChangedCommandProperty]] | Bindable property for `SelectionChangedCommand`. |
| [[SelectableItemsView.SelectionModeProperty\|SelectionModeProperty]] | Bindable property for `SelectionMode`. |

## Remarks

`SelectableItemsView` extends `StructuredItemsView` to add selection capabilities. Use `SelectionMode` to control whether single or multiple items can be selected. The `SelectedItem` and `SelectedItems` properties track the current selection, and the `SelectionChanged` event notifies when selection changes.

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.selectableitemsview)
