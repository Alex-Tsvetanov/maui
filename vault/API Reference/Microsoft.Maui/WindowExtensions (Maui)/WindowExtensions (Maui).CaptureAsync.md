---
title: "WindowExtensions (Maui).CaptureAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.WindowExtensions.CaptureAsync"
declaring_type: "WindowExtensions (Maui)"
member_kind: method
---

# WindowExtensions (Maui).CaptureAsync

> [!abstract] Method of [[WindowExtensions (Maui)|WindowExtensions (Maui)]]
> Namespace: `Microsoft.Maui`

Captures a screenshot of the specified `window`.

## Signature

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.Media.IScreenshotResult?>! static CaptureAsync(this Microsoft.Maui.IWindow! window)
```

## Remarks

On non-built-in platform TFMs (e.g. net10.0-macos AppKit backends, net10.0 Linux/GTK backends) where MAUI does not ship a screenshot implementation, capture is routed through a keyed DI hook. Third-party platform backends can opt in by registering a `Func{T, TResult}` of `object` to Task<IScreenshotResult?> under the service key "Microsoft.Maui.WindowCapture" : builder.Services.AddKeyedSingleton<Func<object, Task<IScreenshotResult?>>>( "Microsoft.Maui.WindowCapture", (_, _) => platformWindow => ((AppKit.NSWindow)platformWindow).CaptureAsync()); If no hook is registered (or the `PlatformView` is `null`), the returned task resolves to `null`.

## See also

- Declaring type: [[WindowExtensions (Maui)|WindowExtensions (Maui)]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
