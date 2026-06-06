---
title: "BlazorWebViewHandler.CreatePlatformView"
tags:
  - api
  - member/method
  - ns/Microsoft-AspNetCore-Components-WebView-Maui
aliases:
  - "Microsoft.AspNetCore.Components.WebView.Maui.BlazorWebViewHandler.CreatePlatformView"
declaring_type: "BlazorWebViewHandler"
member_kind: method
---

# BlazorWebViewHandler.CreatePlatformView

> [!abstract] Method of [[BlazorWebViewHandler|BlazorWebViewHandler]]
> Namespace: `Microsoft.AspNetCore.Components.WebView.Maui`

Gets the concrete LifecycleEventService to access internal RemoveEvent method. RemoveEvent is internal because it's not part of the public ILifecycleEventService contract, but is needed for proper cleanup of lifecycle event handlers.

## Signature

```csharp
Android.Webkit.WebView! override CreatePlatformView()
```

## See also

- Declaring type: [[BlazorWebViewHandler|BlazorWebViewHandler]]
- [[_Microsoft.AspNetCore.Components.WebView.Maui|Microsoft.AspNetCore.Components.WebView.Maui namespace]]
