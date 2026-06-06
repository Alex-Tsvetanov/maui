---
title: "StructuredItemsView"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.StructuredItemsView"
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

# StructuredItemsView

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.StructuredItemsView`

An items view that supports headers, footers, and configurable item layouts.

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
| [[StructuredItemsView.StructuredItemsView\|StructuredItemsView]] |  |

## Properties

| Name | Summary |
|---|---|
| [[StructuredItemsView.Footer\|Footer]] |  |
| [[StructuredItemsView.FooterTemplate\|FooterTemplate]] |  |
| [[StructuredItemsView.Header\|Header]] |  |
| [[StructuredItemsView.HeaderTemplate\|HeaderTemplate]] |  |
| [[StructuredItemsView.ItemSizingStrategy\|ItemSizingStrategy]] |  |
| [[StructuredItemsView.ItemsLayout\|ItemsLayout]] |  |

## Fields

| Name | Summary |
|---|---|
| [[StructuredItemsView.FooterProperty\|FooterProperty]] | Gets or sets the `DataTemplate` used to render the header. |
| [[StructuredItemsView.FooterTemplateProperty\|FooterTemplateProperty]] | Gets or sets the object to display as the footer of the items view. |
| [[StructuredItemsView.HeaderProperty\|HeaderProperty]] | Bindable property for `Header`. |
| [[StructuredItemsView.HeaderTemplateProperty\|HeaderTemplateProperty]] | Gets or sets the object to display as the header of the items view. |
| [[StructuredItemsView.ItemSizingStrategyProperty\|ItemSizingStrategyProperty]] | Gets or sets the layout strategy used to arrange items in the view. |
| [[StructuredItemsView.ItemsLayoutProperty\|ItemsLayoutProperty]] | Gets or sets the `DataTemplate` used to render the footer. |

## Remarks

`StructuredItemsView` extends `ItemsView` to add structural elements like headers and footers, as well as control over how items are laid out through the `ItemsLayout` property. This class serves as the base for views like `SelectableItemsView` and ultimately `CollectionView`.

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.structureditemsview)
