---
title: "ItemsViewScrolledEventArgs"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ItemsViewScrolledEventArgs"
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

# ItemsViewScrolledEventArgs

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.ItemsViewScrolledEventArgs`

Provides data for the Scrolled event in items views.

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
| [[ItemsViewScrolledEventArgs.ItemsViewScrolledEventArgs\|ItemsViewScrolledEventArgs]] |  |

## Properties

| Name | Summary |
|---|---|
| [[ItemsViewScrolledEventArgs.CenterItemIndex\|CenterItemIndex]] | Gets or sets the index of the item at the center of the view. |
| [[ItemsViewScrolledEventArgs.FirstVisibleItemIndex\|FirstVisibleItemIndex]] | Gets or sets the index of the first visible item in the view. |
| [[ItemsViewScrolledEventArgs.HorizontalDelta\|HorizontalDelta]] | Gets or sets the horizontal distance scrolled since the last scroll event. |
| [[ItemsViewScrolledEventArgs.HorizontalOffset\|HorizontalOffset]] | Gets or sets the current horizontal scroll position. |
| [[ItemsViewScrolledEventArgs.LastVisibleItemIndex\|LastVisibleItemIndex]] | Gets or sets the index of the last visible item in the view. |
| [[ItemsViewScrolledEventArgs.VerticalDelta\|VerticalDelta]] | Gets or sets the vertical distance scrolled since the last scroll event. |
| [[ItemsViewScrolledEventArgs.VerticalOffset\|VerticalOffset]] | Gets or sets the current vertical scroll position. |

## Remarks

This event args class contains information about scroll position changes in `CollectionView` and related controls. It includes both delta (change) values and absolute offset values, as well as information about which items are currently visible.

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.itemsviewscrolledeventargs)
