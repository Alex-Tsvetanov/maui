---
title: "ActivityIndicatorExtensions.UpdateColor"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.ActivityIndicatorExtensions.UpdateColor"
declaring_type: "ActivityIndicatorExtensions"
member_kind: method
---

# ActivityIndicatorExtensions.UpdateColor

> [!abstract] Method of [[ActivityIndicatorExtensions|ActivityIndicatorExtensions]]
> Namespace: `Microsoft.Maui.Platform`

Updates the native activity indicator's color from the specified cross-platform activity indicator.

## Signatures

```csharp
void static UpdateColor(this Android.Widget.ProgressBar! progressBar, Microsoft.Maui.IActivityIndicator! activityIndicator)
void static UpdateColor(this UIKit.UIActivityIndicatorView! activityIndicatorView, Microsoft.Maui.IActivityIndicator! activityIndicator)
void static UpdateColor(this Tizen.UIExtensions.NUI.GraphicsView.ActivityIndicator! platformView, Microsoft.Maui.IActivityIndicator! activityIndicator)
void static UpdateColor(this Microsoft.UI.Xaml.Controls.ProgressRing! platformActivityIndicator, Microsoft.Maui.IActivityIndicator! activityIndicator, object? foregroundDefault)
void static UpdateColor(this Microsoft.UI.Xaml.Controls.ProgressRing! platformActivityIndicator, Microsoft.Maui.IActivityIndicator! activityIndicator)
```

## See also

- Declaring type: [[ActivityIndicatorExtensions|ActivityIndicatorExtensions]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
