---
title: "ActivityIndicatorExtensions.UpdateIsRunning"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.ActivityIndicatorExtensions.UpdateIsRunning"
declaring_type: "ActivityIndicatorExtensions"
member_kind: method
---

# ActivityIndicatorExtensions.UpdateIsRunning

> [!abstract] Method of [[ActivityIndicatorExtensions|ActivityIndicatorExtensions]]
> Namespace: `Microsoft.Maui.Platform`

Updates the native activity indicator's running state from the specified cross-platform activity indicator.

## Signatures

```csharp
void static UpdateIsRunning(this Android.Widget.ProgressBar! progressBar, Microsoft.Maui.IActivityIndicator! activityIndicator)
void static UpdateIsRunning(this UIKit.UIActivityIndicatorView! activityIndicatorView, Microsoft.Maui.IActivityIndicator! activityIndicator)
void static UpdateIsRunning(this Tizen.UIExtensions.NUI.GraphicsView.ActivityIndicator! platformView, Microsoft.Maui.IActivityIndicator! activityIndicator)
void static UpdateIsRunning(this Microsoft.UI.Xaml.Controls.ProgressRing! platformActivityIndicator, Microsoft.Maui.IActivityIndicator! virtualView)
```

## See also

- Declaring type: [[ActivityIndicatorExtensions|ActivityIndicatorExtensions]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
