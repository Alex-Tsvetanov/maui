---
title: "BlazorWebViewHandler.ConnectHandler"
tags:
  - api
  - member/method
  - ns/Microsoft-AspNetCore-Components-WebView-Maui
aliases:
  - "Microsoft.AspNetCore.Components.WebView.Maui.BlazorWebViewHandler.ConnectHandler"
declaring_type: "BlazorWebViewHandler"
member_kind: method
---

# BlazorWebViewHandler.ConnectHandler

> [!abstract] Method of [[BlazorWebViewHandler|BlazorWebViewHandler]]
> Namespace: `Microsoft.AspNetCore.Components.WebView.Maui`

Connects the handler to the Android `AWebView` and registers platform-specific back navigation handling so that the WebView can consume back presses before the page is popped.

## Signature

```csharp
void override ConnectHandler(Tizen.NUI.BaseComponents.WebView! platformView)
```

## Parameters

| Parameter | Description |
|---|---|
| `platformView` | The native Android `AWebView` instance associated with this handler. |

## Remarks

This override calls the base implementation and then registers an `OnBackPressed` lifecycle event handler. The handler checks `CanGoBack` and, when possible, navigates back within the WebView instead of allowing the back press (or predictive back gesture on Android 13+) to propagate and pop the containing page. When multiple BlazorWebView instances exist, the handler includes focus and visibility checks to ensure only the currently visible and focused WebView handles the back navigation, preventing conflicts between instances. Inheritors that override this method should call the base implementation to preserve this back navigation behavior unless they intentionally replace it.

## See also

- Declaring type: [[BlazorWebViewHandler|BlazorWebViewHandler]]
- [[_Microsoft.AspNetCore.Components.WebView.Maui|Microsoft.AspNetCore.Components.WebView.Maui namespace]]
