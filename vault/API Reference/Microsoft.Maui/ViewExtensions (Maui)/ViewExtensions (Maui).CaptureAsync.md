---
title: "ViewExtensions (Maui).CaptureAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.ViewExtensions.CaptureAsync"
declaring_type: "ViewExtensions (Maui)"
member_kind: method
---

# ViewExtensions (Maui).CaptureAsync

> [!abstract] Method of [[ViewExtensions (Maui)|ViewExtensions (Maui)]]
> Namespace: `Microsoft.Maui`

Captures a screenshot of the specified `view`.

## Signature

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.Media.IScreenshotResult?>! static CaptureAsync(this Microsoft.Maui.IView! view)
```

## Remarks

On non-built-in platform TFMs (e.g. net10.0-macos AppKit backends, net10.0 Linux/GTK backends) where MAUI does not ship a screenshot implementation, capture is routed through a keyed DI hook. Third-party platform backends can opt in by registering a `Func{T, TResult}` of `object` to Task<IScreenshotResult?> under the service key "Microsoft.Maui.ViewCapture" : builder.Services.AddKeyedSingleton<Func<object, Task<IScreenshotResult?>>>( "Microsoft.Maui.ViewCapture", (_, _) => platformView => ((AppKit.NSView)platformView).CaptureAsync()); If no hook is registered (or the `PlatformView` is `null`), the returned task resolves to `null`.

## See also

- Declaring type: [[ViewExtensions (Maui)|ViewExtensions (Maui)]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
