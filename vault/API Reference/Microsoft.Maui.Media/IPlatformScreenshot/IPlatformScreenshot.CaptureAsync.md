---
title: "IPlatformScreenshot.CaptureAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Media
aliases:
  - "Microsoft.Maui.Media.IPlatformScreenshot.CaptureAsync"
declaring_type: "IPlatformScreenshot"
member_kind: method
---

# IPlatformScreenshot.CaptureAsync

> [!abstract] Method of [[IPlatformScreenshot|IPlatformScreenshot]]
> Namespace: `Microsoft.Maui.Media`

Captures a screenshot of the specified activity.

## Signatures

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.Media.IScreenshotResult!>! CaptureAsync(Android.App.Activity! activity)
System.Threading.Tasks.Task<Microsoft.Maui.Media.IScreenshotResult?>! CaptureAsync(Android.Views.View! view)
System.Threading.Tasks.Task<Microsoft.Maui.Media.IScreenshotResult?>! CaptureAsync(CoreAnimation.CALayer! layer, bool skipChildren)
System.Threading.Tasks.Task<Microsoft.Maui.Media.IScreenshotResult?>! CaptureAsync(UIKit.UIView! view)
System.Threading.Tasks.Task<Microsoft.Maui.Media.IScreenshotResult!>! CaptureAsync(UIKit.UIWindow! window)
System.Threading.Tasks.Task<Microsoft.Maui.Media.IScreenshotResult?>! CaptureAsync(Tizen.NUI.BaseComponents.View! view)
System.Threading.Tasks.Task<Microsoft.Maui.Media.IScreenshotResult!>! CaptureAsync(Tizen.NUI.Window! window)
System.Threading.Tasks.Task<Microsoft.Maui.Media.IScreenshotResult?>! CaptureAsync(Microsoft.UI.Xaml.UIElement! element)
System.Threading.Tasks.Task<Microsoft.Maui.Media.IScreenshotResult!>! CaptureAsync(Microsoft.UI.Xaml.Window! window)
```

## Returns

An instance of `IScreenshotResult` with information about the captured screenshot.

## Parameters

| Parameter | Description |
|---|---|
| `activity` | The activity to capture. |

## See also

- Declaring type: [[IPlatformScreenshot|IPlatformScreenshot]]
- [[_Microsoft.Maui.Media|Microsoft.Maui.Media namespace]]
