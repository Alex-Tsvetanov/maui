---
title: "CurrentItemChangedEventArgs"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.CurrentItemChangedEventArgs"
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

# CurrentItemChangedEventArgs

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.CurrentItemChangedEventArgs`

Provides data for the CurrentItemChanged event in carousel and collection views.

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
| [[CurrentItemChangedEventArgs.CurrentItem\|CurrentItem]] | Gets the item that is now current after the change. |
| [[CurrentItemChangedEventArgs.PreviousItem\|PreviousItem]] | Gets the item that was previously current before the change. |

## Remarks

This event args class is used when the current item changes in a `CarouselView` or similar control. It provides both the previous and current items to allow tracking of item changes.

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.currentitemchangedeventargs)
