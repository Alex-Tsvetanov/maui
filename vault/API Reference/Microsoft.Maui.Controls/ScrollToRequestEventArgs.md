---
title: "ScrollToRequestEventArgs"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ScrollToRequestEventArgs"
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

# ScrollToRequestEventArgs

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.ScrollToRequestEventArgs`

Provides data for scroll-to-item requests in items views.

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
| [[ScrollToRequestEventArgs.ScrollToRequestEventArgs\|ScrollToRequestEventArgs]] | Initializes a new instance of the `ScrollToRequestEventArgs` class for scrolling by position index. |

## Properties

| Name | Summary |
|---|---|
| [[ScrollToRequestEventArgs.Group\|Group]] | Gets the group object containing the item to scroll to. |
| [[ScrollToRequestEventArgs.GroupIndex\|GroupIndex]] | Gets the zero-based index of the group containing the item to scroll to. |
| [[ScrollToRequestEventArgs.Index\|Index]] | Gets the zero-based index of the item to scroll to. |
| [[ScrollToRequestEventArgs.IsAnimated\|IsAnimated]] | Gets a value indicating whether the scrolling should be animated. |
| [[ScrollToRequestEventArgs.Item\|Item]] | Gets the data item to scroll to. |
| [[ScrollToRequestEventArgs.Mode\|Mode]] | Gets the scroll mode indicating whether to scroll by position or by element reference. |
| [[ScrollToRequestEventArgs.ScrollToPosition\|ScrollToPosition]] | Gets the position where the target item should be positioned within the visible area. |

## Remarks

This event args class contains information needed to scroll to a specific item or position in `CollectionView` and related controls. It supports two modes: scrolling by index position or by item reference, as indicated by the `Mode` property.

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.scrolltorequesteventargs)
