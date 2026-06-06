---
title: "ViewExtensions (Microsoft.Maui.Platform).UpdateBackground"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.ViewExtensions.UpdateBackground"
declaring_type: "ViewExtensions (Microsoft.Maui.Platform)"
member_kind: method
---

# ViewExtensions (Microsoft.Maui.Platform).UpdateBackground

> [!abstract] Method of [[ViewExtensions (Microsoft.Maui.Platform)|ViewExtensions (Microsoft.Maui.Platform)]]
> Namespace: `Microsoft.Maui.Platform`

Updates the native platform view to reflect the cross-platform view's Background value.

## Signatures

```csharp
void static UpdateBackground(this Android.Views.View! platformView, Microsoft.Maui.Graphics.Paint? background)
void static UpdateBackground(this Android.Views.View! platformView, Microsoft.Maui.IView! view)
void static UpdateBackground(this Microsoft.Maui.Platform.ContentViewGroup! platformView, Microsoft.Maui.IBorderStroke! border)
void static UpdateBackground(this Microsoft.Maui.Platform.ContentView! platformView, Microsoft.Maui.IBorderStroke! border)
void static UpdateBackground(this UIKit.UIView! platformView, Microsoft.Maui.Graphics.Paint? paint, Microsoft.Maui.IButtonStroke? stroke = null)
void static UpdateBackground(this UIKit.UIView! platformView, Microsoft.Maui.IView! view)
void static UpdateBackground(this Microsoft.Maui.Platform.ContentViewGroup! platformView, Microsoft.Maui.IBorderView! border)
void static UpdateBackground(this Tizen.NUI.BaseComponents.View! platformView, Microsoft.Maui.Graphics.Paint? paint)
void static UpdateBackground(this Tizen.NUI.BaseComponents.View! platformView, Microsoft.Maui.IView! view)
void static UpdateBackground(this Microsoft.Maui.Platform.ContentPanel! platformView, Microsoft.Maui.IBorderStroke! border)
void static UpdateBackground(this Microsoft.UI.Xaml.FrameworkElement! platformView, Microsoft.Maui.IView! view)
void static UpdateBackground(this object! platformView, Microsoft.Maui.IView! view)
```

## See also

- Declaring type: [[ViewExtensions (Microsoft.Maui.Platform)|ViewExtensions (Microsoft.Maui.Platform)]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
