---
title: "PositionChangedEventArgs"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.PositionChangedEventArgs"
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

# PositionChangedEventArgs

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.PositionChangedEventArgs`

Provides data for the PositionChanged event in carousel and collection views.

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


## Properties

| Name | Summary |
|---|---|
| [[PositionChangedEventArgs.CurrentPosition\|CurrentPosition]] | Gets the position index that is now current after the change. |
| [[PositionChangedEventArgs.PreviousPosition\|PreviousPosition]] | Gets the position index that was current before the change. |

## Remarks

This event args class is used when the current position changes in a `CarouselView` or `IndicatorView`. It provides both the previous and current positions as zero-based indices.

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.positionchangedeventargs)
