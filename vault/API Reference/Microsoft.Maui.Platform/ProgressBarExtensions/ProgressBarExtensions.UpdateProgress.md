---
title: "ProgressBarExtensions.UpdateProgress"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.ProgressBarExtensions.UpdateProgress"
declaring_type: "ProgressBarExtensions"
member_kind: method
---

# ProgressBarExtensions.UpdateProgress

> [!abstract] Method of [[ProgressBarExtensions|ProgressBarExtensions]]
> Namespace: `Microsoft.Maui.Platform`

Updates the native progress bar's value to match the cross-platform progress.

## Signatures

```csharp
void static UpdateProgress(this Android.Widget.ProgressBar! platformProgressBar, Microsoft.Maui.IProgress! progress)
void static UpdateProgress(this UIKit.UIProgressView! platformProgressBar, Microsoft.Maui.IProgress! progress)
void static UpdateProgress(this Tizen.UIExtensions.NUI.GraphicsView.ProgressBar! platformProgressBar, Microsoft.Maui.IProgress! progress)
void static UpdateProgress(this Microsoft.UI.Xaml.Controls.ProgressBar! platformProgressBar, Microsoft.Maui.IProgress! progress)
```

## See also

- Declaring type: [[ProgressBarExtensions|ProgressBarExtensions]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
