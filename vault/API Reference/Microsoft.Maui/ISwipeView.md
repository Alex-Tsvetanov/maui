---
title: "ISwipeView"
tags:
  - api
  - kind/interface
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.ISwipeView"
namespace: "Microsoft.Maui"
kind: interface
platforms:
  - All platforms (.NET)
  - Android
  - iOS
  - Mac Catalyst
  - Windows
  - Tizen
  - .NET Standard
  - .NET Standard 2.0
assemblies:
  - src
---

# ISwipeView

> [!abstract] Interface in `Microsoft.Maui`
> Full name: `Microsoft.Maui.ISwipeView`

Represents a container that provides access to contextual commands through touch interactions.

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
| .NET Standard 2.0 | ✅ |


## Properties

| Name | Summary |
|---|---|
| [[ISwipeView.BottomItems\|BottomItems]] | Gets the items that can be invoked when the control is swiped from the bottom up. |
| [[ISwipeView.IsOpen\|IsOpen]] | Gets a value indicating whether the swipe view is open or not. |
| [[ISwipeView.LeftItems\|LeftItems]] | Gets the items that can be invoked when the control is swiped from the left side. |
| [[ISwipeView.RightItems\|RightItems]] | Gets the items that can be invoked when the control is swiped from the right side. |
| [[ISwipeView.SwipeTransitionMode\|SwipeTransitionMode]] | Define the swipe transition in the control. |
| [[ISwipeView.Threshold\|Threshold]] | Gets a value that represents the minimum swipe distance that must be achieved for a swipe to be recognized. |
| [[ISwipeView.TopItems\|TopItems]] | Gets the items that can be invoked when the control is swiped from the top down. |

## Methods

| Name | Summary |
|---|---|
| [[ISwipeView.RequestClose\|RequestClose]] | Closes the swipe view. |
| [[ISwipeView.RequestOpen\|RequestOpen]] | Open the swipe view. |
| [[ISwipeView.SwipeChanging\|SwipeChanging]] | Event that is fired when the swipe starts. |
| [[ISwipeView.SwipeEnded\|SwipeEnded]] | Event that is fired when the swipe is finished. |
| [[ISwipeView.SwipeStarted\|SwipeStarted]] | Event that is fired when the swipe starts. |

## See also

- [[_Microsoft.Maui|Microsoft.Maui namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.iswipeview)
