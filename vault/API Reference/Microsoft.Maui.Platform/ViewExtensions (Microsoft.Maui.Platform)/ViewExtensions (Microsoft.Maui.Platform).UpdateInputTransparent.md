---
title: "ViewExtensions (Microsoft.Maui.Platform).UpdateInputTransparent"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.ViewExtensions.UpdateInputTransparent"
declaring_type: "ViewExtensions (Microsoft.Maui.Platform)"
member_kind: method
---

# ViewExtensions (Microsoft.Maui.Platform).UpdateInputTransparent

> [!abstract] Method of [[ViewExtensions (Microsoft.Maui.Platform)|ViewExtensions (Microsoft.Maui.Platform)]]
> Namespace: `Microsoft.Maui.Platform`

Background and InputTransparent for Windows layouts are heavily intertwined, so setting one usually requires setting the other at the same time.

## Signatures

```csharp
void static UpdateInputTransparent(this UIKit.UIView! platformView, bool isReadOnly, bool inputTransparent)
void static UpdateInputTransparent(this UIKit.UIView! platformView, Microsoft.Maui.IViewHandler! handler, Microsoft.Maui.IView! view)
void static UpdateInputTransparent(this Tizen.NUI.BaseComponents.View! platformView, Microsoft.Maui.IViewHandler! handler, Microsoft.Maui.IView! view)
void static UpdateInputTransparent(this Microsoft.Maui.Platform.LayoutPanel! layoutPanel, Microsoft.Maui.ILayoutHandler! handler, Microsoft.Maui.ILayout! layout)
void static UpdateInputTransparent(this Microsoft.UI.Xaml.FrameworkElement! nativeView, Microsoft.Maui.IViewHandler! handler, Microsoft.Maui.IView! view)
void static UpdateInputTransparent(this object! nativeView, Microsoft.Maui.IViewHandler! handler, Microsoft.Maui.IView! view)
```

## See also

- Declaring type: [[ViewExtensions (Microsoft.Maui.Platform)|ViewExtensions (Microsoft.Maui.Platform)]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
