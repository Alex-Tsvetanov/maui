---
title: "ItemSizingStrategy"
tags:
  - api
  - kind/enum
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ItemSizingStrategy"
namespace: "Microsoft.Maui.Controls"
kind: enum
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

# ItemSizingStrategy

> [!abstract] Enum in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.ItemSizingStrategy`

Specifies the strategy used to measure and size items in an items view.

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


## Fields

| Name | Summary |
|---|---|
| [[ItemSizingStrategy.MeasureAllItems\|MeasureAllItems]] |  |
| [[ItemSizingStrategy.MeasureFirstItem\|MeasureFirstItem]] |  |

## Remarks

The sizing strategy affects both performance and visual accuracy in collection views. For collections with uniform item sizes, use `MeasureFirstItem` for better performance. For collections with varying item sizes, use `MeasureAllItems` for accurate layout.

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.itemsizingstrategy)
