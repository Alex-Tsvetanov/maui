---
title: "IItemsLayout"
tags:
  - api
  - kind/interface
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.IItemsLayout"
namespace: "Microsoft.Maui.Controls"
kind: interface
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

# IItemsLayout

> [!abstract] Interface in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.IItemsLayout`

Defines the contract for an items layout that arranges items in collection and carousel views.

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


## Remarks

This interface serves as a marker for classes that define how items are laid out in views like `CollectionView` and `CarouselView`. Implementations include `LinearItemsLayout` for single-row/column layouts and `GridItemsLayout` for grid-based layouts. The type converter attribute enables XAML parsing of layout specifications.

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.iitemslayout)
