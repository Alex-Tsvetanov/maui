---
title: "GridItemsLayout"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.GridItemsLayout"
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

# GridItemsLayout

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.GridItemsLayout`

An items layout that arranges items in a grid with configurable columns or rows.

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
| [[GridItemsLayout.GridItemsLayout\|GridItemsLayout]] | Gets or sets the number of columns (for vertical orientation) or rows (for horizontal orientation) in the grid. |

## Properties

| Name | Summary |
|---|---|
| [[GridItemsLayout.HorizontalItemSpacing\|HorizontalItemSpacing]] |  |
| [[GridItemsLayout.Span\|Span]] |  |
| [[GridItemsLayout.VerticalItemSpacing\|VerticalItemSpacing]] |  |

## Fields

| Name | Summary |
|---|---|
| [[GridItemsLayout.HorizontalItemSpacingProperty\|HorizontalItemSpacingProperty]] | Gets or sets the vertical spacing between items in the grid. |
| [[GridItemsLayout.SpanProperty\|SpanProperty]] | Bindable property for `Span`. |
| [[GridItemsLayout.VerticalItemSpacingProperty\|VerticalItemSpacingProperty]] | Bindable property for `VerticalItemSpacing`. |

## Remarks

`GridItemsLayout` displays items in a grid format, with the number of columns (for vertical scrolling) or rows (for horizontal scrolling) determined by the `Span` property. Use `HorizontalItemSpacing` and `VerticalItemSpacing` to control the spacing between items. This layout is commonly used in `CollectionView` for displaying items in a multi-column or multi-row format.

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.griditemslayout)
