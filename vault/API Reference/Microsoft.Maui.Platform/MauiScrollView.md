---
title: "MauiScrollView"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.MauiScrollView"
namespace: "Microsoft.Maui.Platform"
kind: class
platforms:
  - Android
  - iOS
  - Mac Catalyst
  - Tizen
assemblies:
  - src
---

# MauiScrollView

> [!abstract] Class in `Microsoft.Maui.Platform`
> Full name: `Microsoft.Maui.Platform.MauiScrollView`

A custom UIScrollView implementation that provides cross-platform layout support and safe area management for .NET MAUI applications on iOS. This class handles the bridge between MAUI's cross-platform layout system and iOS's native UIScrollView behavior.

## Platforms

| Platform | Available |
|---|---|
| Android | ✅ |
| iOS | ✅ |
| Mac Catalyst | ✅ |
| Tizen | ✅ |


## Constructors

| Name | Summary |
|---|---|
| [[MauiScrollView.MauiScrollView\|MauiScrollView]] | Flag indicating whether this scroll view should apply safe area adjustments to its content. Only true when not nested in another scroll view, no parent MauiV… |

## Properties

| Name | Summary |
|---|---|
| [[MauiScrollView.CrossPlatformLayout\|CrossPlatformLayout]] |  |

## Methods

| Name | Summary |
|---|---|
| [[MauiScrollView.AdjustedContentInsetDidChange\|AdjustedContentInsetDidChange]] | Determines whether this scroll view should respond to safe area changes. Returns false if this scroll view is nested within another scroll view, as nested sc… |
| [[MauiScrollView.LayoutSubviews\|LayoutSubviews]] |  |
| [[MauiScrollView.Measure\|Measure]] |  |
| [[MauiScrollView.MovedToWindow\|MovedToWindow]] | Event handler for the MovedToWindow event. This is used to support the IUIViewLifeCycleEvents interface and allows subscribers to be notified when the view h… |
| [[MauiScrollView.OnAttachedToWindow\|OnAttachedToWindow]] |  |
| [[MauiScrollView.OnConfigurationChanged\|OnConfigurationChanged]] |  |
| [[MauiScrollView.OnDetachedFromWindow\|OnDetachedFromWindow]] |  |
| [[MauiScrollView.OnInterceptTouchEvent\|OnInterceptTouchEvent]] |  |
| [[MauiScrollView.OnLayout\|OnLayout]] |  |
| [[MauiScrollView.OnMeasure\|OnMeasure]] |  |
| [[MauiScrollView.OnTouchEvent\|OnTouchEvent]] |  |
| [[MauiScrollView.SafeAreaInsetsDidChange\|SafeAreaInsetsDidChange]] | Called by iOS when the safe area insets change (e.g., device rotation, notch visibility). This method marks the safe area as invalidated. Note that UIKit aut… |
| [[MauiScrollView.ScrollRectToVisible\|ScrollRectToVisible]] | Invalidates the cached constraint values, forcing a re-measurement and re-arrangement on the next layout pass. |
| [[MauiScrollView.ScrollTo\|ScrollTo]] | Marks that the SafeAreaEdges configuration changed so we re-request window insets next layout. |
| [[MauiScrollView.SetContent\|SetContent]] |  |
| [[MauiScrollView.SetHorizontalScrollBarVisibility\|SetHorizontalScrollBarVisibility]] |  |
| [[MauiScrollView.SetOrientation\|SetOrientation]] |  |
| [[MauiScrollView.SetVerticalScrollBarVisibility\|SetVerticalScrollBarVisibility]] |  |
| [[MauiScrollView.SizeThatFits\|SizeThatFits]] | Calculates the size that fits within the given constraints. This method is called by iOS when the system needs to determine the natural size of the scroll view. |

## See also

- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.platform.mauiscrollview)
