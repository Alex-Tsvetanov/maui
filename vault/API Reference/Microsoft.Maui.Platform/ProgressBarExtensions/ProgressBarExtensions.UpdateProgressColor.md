---
title: "ProgressBarExtensions.UpdateProgressColor"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.ProgressBarExtensions.UpdateProgressColor"
declaring_type: "ProgressBarExtensions"
member_kind: method
---

# ProgressBarExtensions.UpdateProgressColor

> [!abstract] Method of [[ProgressBarExtensions|ProgressBarExtensions]]
> Namespace: `Microsoft.Maui.Platform`

Updates the native progress bar's color to match the cross-platform progress.

## Signatures

```csharp
void static UpdateProgressColor(this Android.Widget.ProgressBar! platformProgressBar, Microsoft.Maui.IProgress! progress)
void static UpdateProgressColor(this UIKit.UIProgressView! platformProgressBar, Microsoft.Maui.IProgress! progress)
void static UpdateProgressColor(this Tizen.UIExtensions.NUI.GraphicsView.ProgressBar! platformProgressBar, Microsoft.Maui.IProgress! progress)
void static UpdateProgressColor(this Microsoft.UI.Xaml.Controls.ProgressBar! platformProgressBar, Microsoft.Maui.IProgress! progress)
```

## See also

- Declaring type: [[ProgressBarExtensions|ProgressBarExtensions]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
