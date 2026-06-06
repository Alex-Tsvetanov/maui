---
title: "GroupableItemsView"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.GroupableItemsView"
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

# GroupableItemsView

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.GroupableItemsView`

A selectable items view that supports grouping of items.

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
| [[GroupableItemsView.GroupableItemsView\|GroupableItemsView]] |  |

## Properties

| Name | Summary |
|---|---|
| [[GroupableItemsView.GroupFooterTemplate\|GroupFooterTemplate]] |  |
| [[GroupableItemsView.GroupHeaderTemplate\|GroupHeaderTemplate]] |  |
| [[GroupableItemsView.IsGrouped\|IsGrouped]] |  |

## Fields

| Name | Summary |
|---|---|
| [[GroupableItemsView.GroupFooterTemplateProperty\|GroupFooterTemplateProperty]] | Gets or sets the `DataTemplate` used to display the header for each group. |
| [[GroupableItemsView.GroupHeaderTemplateProperty\|GroupHeaderTemplateProperty]] | Gets or sets a value indicating whether items should be displayed in groups. |
| [[GroupableItemsView.IsGroupedProperty\|IsGroupedProperty]] | Bindable property for `IsGrouped`. |

## Remarks

`GroupableItemsView` extends `SelectableItemsView` to add support for displaying items in groups. When `IsGrouped` is `true`, items are organized into sections with optional headers and footers. Use `GroupHeaderTemplate` and `GroupFooterTemplate` to customize the appearance of group sections.

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.groupableitemsview)
