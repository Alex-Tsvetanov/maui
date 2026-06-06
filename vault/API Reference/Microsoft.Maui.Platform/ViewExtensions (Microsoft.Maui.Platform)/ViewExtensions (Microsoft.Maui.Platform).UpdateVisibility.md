---
title: "ViewExtensions (Microsoft.Maui.Platform).UpdateVisibility"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.ViewExtensions.UpdateVisibility"
declaring_type: "ViewExtensions (Microsoft.Maui.Platform)"
member_kind: method
---

# ViewExtensions (Microsoft.Maui.Platform).UpdateVisibility

> [!abstract] Method of [[ViewExtensions (Microsoft.Maui.Platform)|ViewExtensions (Microsoft.Maui.Platform)]]
> Namespace: `Microsoft.Maui.Platform`

Updates the native platform view to reflect the cross-platform view's Visibility value.

## Signatures

```csharp
void static UpdateVisibility(this Android.Views.View! platformView, Microsoft.Maui.IView! view)
void static UpdateVisibility(this UIKit.UIView! platformView, Microsoft.Maui.IView! view)
void static UpdateVisibility(this UIKit.UIView! platformView, Microsoft.Maui.Visibility visibility)
void static UpdateVisibility(this Tizen.NUI.BaseComponents.View! platformView, Microsoft.Maui.IView! view)
void static UpdateVisibility(this Microsoft.UI.Xaml.FrameworkElement! platformView, Microsoft.Maui.IView! view)
void static UpdateVisibility(this object! platformView, Microsoft.Maui.IView! view)
```

## See also

- Declaring type: [[ViewExtensions (Microsoft.Maui.Platform)|ViewExtensions (Microsoft.Maui.Platform)]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
