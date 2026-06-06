---
title: "ViewExtensions (Microsoft.Maui.Platform).InvalidateMeasure"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.ViewExtensions.InvalidateMeasure"
declaring_type: "ViewExtensions (Microsoft.Maui.Platform)"
member_kind: method
---

# ViewExtensions (Microsoft.Maui.Platform).InvalidateMeasure

> [!abstract] Method of [[ViewExtensions (Microsoft.Maui.Platform)|ViewExtensions (Microsoft.Maui.Platform)]]
> Namespace: `Microsoft.Maui.Platform`

Invalidates the measure of the view and all its ancestors through `SetNeedsLayout` propagation.

## Signatures

```csharp
void static InvalidateMeasure(this Android.Views.View! platformView, Microsoft.Maui.IView! view)
void static InvalidateMeasure(this UIKit.UIView! platformView, Microsoft.Maui.IView! view)
void static InvalidateMeasure(this Tizen.NUI.BaseComponents.View! platformView, Microsoft.Maui.IView! view)
void static InvalidateMeasure(this Microsoft.UI.Xaml.FrameworkElement! platformView, Microsoft.Maui.IView! view)
void static InvalidateMeasure(this object! platformView, Microsoft.Maui.IView! view)
```

## Remarks

Stops when it reaches the page view or a scrollable area, including `UICollectionView`.

## See also

- Declaring type: [[ViewExtensions (Microsoft.Maui.Platform)|ViewExtensions (Microsoft.Maui.Platform)]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
