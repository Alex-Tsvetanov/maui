---
title: "ScreenshotExtensions.CaptureAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Media
aliases:
  - "Microsoft.Maui.Media.ScreenshotExtensions.CaptureAsync"
declaring_type: "ScreenshotExtensions"
member_kind: method
---

# ScreenshotExtensions.CaptureAsync

> [!abstract] Method of [[ScreenshotExtensions|ScreenshotExtensions]]
> Namespace: `Microsoft.Maui.Media`

Captures a screenshot of the specified activity.

## Signatures

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.Media.IScreenshotResult!>! static CaptureAsync(this Microsoft.Maui.Media.IScreenshot! screenshot, Android.App.Activity! activity)
System.Threading.Tasks.Task<Microsoft.Maui.Media.IScreenshotResult?>! static CaptureAsync(this Microsoft.Maui.Media.IScreenshot! screenshot, Android.Views.View! view)
System.Threading.Tasks.Task<Microsoft.Maui.Media.IScreenshotResult?>! static CaptureAsync(this Microsoft.Maui.Media.IScreenshot! screenshot, CoreAnimation.CALayer! layer, bool skipChildren)
System.Threading.Tasks.Task<Microsoft.Maui.Media.IScreenshotResult?>! static CaptureAsync(this Microsoft.Maui.Media.IScreenshot! screenshot, UIKit.UIView! view)
System.Threading.Tasks.Task<Microsoft.Maui.Media.IScreenshotResult!>! static CaptureAsync(this Microsoft.Maui.Media.IScreenshot! screenshot, UIKit.UIWindow! window)
System.Threading.Tasks.Task<Microsoft.Maui.Media.IScreenshotResult?>! static CaptureAsync(this Microsoft.Maui.Media.IScreenshot! screenshot, Tizen.NUI.BaseComponents.View! view)
System.Threading.Tasks.Task<Microsoft.Maui.Media.IScreenshotResult!>! static CaptureAsync(this Microsoft.Maui.Media.IScreenshot! screenshot, Tizen.NUI.Window! window)
System.Threading.Tasks.Task<Microsoft.Maui.Media.IScreenshotResult?>! static CaptureAsync(this Microsoft.Maui.Media.IScreenshot! screenshot, Microsoft.UI.Xaml.UIElement! element)
System.Threading.Tasks.Task<Microsoft.Maui.Media.IScreenshotResult!>! static CaptureAsync(this Microsoft.Maui.Media.IScreenshot! screenshot, Microsoft.UI.Xaml.Window! window)
```

## Returns

An instance of `IScreenshotResult` with information about the captured screenshot.

## Parameters

| Parameter | Description |
|---|---|
| `screenshot` | The object this method is invoked on. |
| `activity` | The activity to capture. |

## See also

- Declaring type: [[ScreenshotExtensions|ScreenshotExtensions]]
- [[_Microsoft.Maui.Media|Microsoft.Maui.Media namespace]]
