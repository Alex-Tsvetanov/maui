---
title: "MauiView"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.MauiView"
namespace: "Microsoft.Maui.Platform"
kind: class
platforms:
  - iOS
  - Mac Catalyst
assemblies:
  - src
---

# MauiView

> [!abstract] Class in `Microsoft.Maui.Platform`
> Full name: `Microsoft.Maui.Platform.MauiView`

Base class for MAUI views on iOS that provides cross-platform layout capabilities and safe area handling. This view bridges the gap between iOS native UIView and MAUI's cross-platform layout system.

## Platforms

| Platform | Available |
|---|---|
| iOS | ✅ |
| Mac Catalyst | ✅ |


## Constructors

| Name | Summary |
|---|---|
| [[MauiView.MauiView\|MauiView]] |  |

## Properties

| Name | Summary |
|---|---|
| [[MauiView.CrossPlatformLayout\|CrossPlatformLayout]] |  |
| [[MauiView.View\|View]] |  |

## Methods

| Name | Summary |
|---|---|
| [[MauiView.AdjustForSafeArea\|AdjustForSafeArea]] | Adjusts the given bounds rectangle to account for safe area insets. This method subtracts the safe area padding from the bounds to ensure content doesn't ove… |
| [[MauiView.CacheMeasureConstraints\|CacheMeasureConstraints]] | Caches the measure constraints and the resulting measured size from the last measure operation. |
| [[MauiView.InvalidateConstraintsCache\|InvalidateConstraintsCache]] | Determines if this view has been measured at least once. Used to decide whether a layout pass needs to perform measurement. |
| [[MauiView.IsMeasureValid\|IsMeasureValid]] | Checks if the current measure information is still valid for the given constraints. This optimization avoids redundant measure operations when constraints ha… |
| [[MauiView.LayoutSubviews\|LayoutSubviews]] |  |
| [[MauiView.MovedToWindow\|MovedToWindow]] | Directly invalidates this view's safe area, forcing re-evaluation on next layout pass. |
| [[MauiView.SafeAreaInsetsDidChange\|SafeAreaInsetsDidChange]] | Event handler for the MovedToWindow event. This field is used to store subscriptions to the IUIViewLifeCycleEvents.MovedToWindow event. |
| [[MauiView.SizeThatFits\|SizeThatFits]] | Performs cross-platform arrange operation, optionally adjusting for safe area. This method positions and sizes child elements within the available bounds. |

## See also

- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.platform.mauiview)
