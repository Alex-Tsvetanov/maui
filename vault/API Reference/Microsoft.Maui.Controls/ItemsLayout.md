---
title: "ItemsLayout"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ItemsLayout"
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

# ItemsLayout

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.ItemsLayout`

Base class for layouts that arrange items in collection and carousel views.

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
| [[ItemsLayout.ItemsLayout\|ItemsLayout]] | Initializes a new instance of the `ItemsLayout` class with the specified orientation. |

## Properties

| Name | Summary |
|---|---|
| [[ItemsLayout.Orientation\|Orientation]] | Gets the orientation of the items layout. |
| [[ItemsLayout.SnapPointsAlignment\|SnapPointsAlignment]] |  |
| [[ItemsLayout.SnapPointsType\|SnapPointsType]] |  |

## Fields

| Name | Summary |
|---|---|
| [[ItemsLayout.SnapPointsAlignmentProperty\|SnapPointsAlignmentProperty]] | Bindable property for `SnapPointsAlignment`. |
| [[ItemsLayout.SnapPointsTypeProperty\|SnapPointsTypeProperty]] | Gets or sets how items align to snap points when scrolling stops. |

## Remarks

`ItemsLayout` is the abstract base class for item arrangement strategies used in `CollectionView` and `CarouselView`. It defines common properties like `Orientation`, `SnapPointsType`, and `SnapPointsAlignment` that control how items are laid out and how scrolling behaves. Concrete implementations include `LinearItemsLayout` and `GridItemsLayout`.

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.itemslayout)
