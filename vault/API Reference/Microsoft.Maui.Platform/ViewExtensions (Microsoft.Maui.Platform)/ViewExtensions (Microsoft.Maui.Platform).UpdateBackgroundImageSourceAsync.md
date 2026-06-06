---
title: "ViewExtensions (Microsoft.Maui.Platform).UpdateBackgroundImageSourceAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.ViewExtensions.UpdateBackgroundImageSourceAsync"
declaring_type: "ViewExtensions (Microsoft.Maui.Platform)"
member_kind: method
---

# ViewExtensions (Microsoft.Maui.Platform).UpdateBackgroundImageSourceAsync

> [!abstract] Method of [[ViewExtensions (Microsoft.Maui.Platform)|ViewExtensions (Microsoft.Maui.Platform)]]
> Namespace: `Microsoft.Maui.Platform`

Updates the native platform view to reflect the cross-platform view's BackgroundImageSourceAsync value.

## Signatures

```csharp
System.Threading.Tasks.Task! static UpdateBackgroundImageSourceAsync(this Android.Views.View! platformView, Microsoft.Maui.IImageSource? imageSource, Microsoft.Maui.IImageSourceServiceProvider? provider)
System.Threading.Tasks.Task! static UpdateBackgroundImageSourceAsync(this UIKit.UIView! platformView, Microsoft.Maui.IImageSource? imageSource, Microsoft.Maui.IImageSourceServiceProvider? provider)
System.Threading.Tasks.Task! static UpdateBackgroundImageSourceAsync(this Tizen.NUI.BaseComponents.View! platformView, Microsoft.Maui.IImageSource? imageSource, Microsoft.Maui.IImageSourceServiceProvider? provider)
System.Threading.Tasks.Task! static UpdateBackgroundImageSourceAsync(this Microsoft.UI.Xaml.FrameworkElement! platformView, Microsoft.Maui.IImageSource? imageSource, Microsoft.Maui.IImageSourceServiceProvider? provider)
System.Threading.Tasks.Task! static UpdateBackgroundImageSourceAsync(this object! platformView, Microsoft.Maui.IImageSource? imageSource, Microsoft.Maui.IImageSourceServiceProvider? provider)
```

## See also

- Declaring type: [[ViewExtensions (Microsoft.Maui.Platform)|ViewExtensions (Microsoft.Maui.Platform)]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
