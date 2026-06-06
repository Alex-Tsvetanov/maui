---
title: "ImageHandler.ConnectHandler"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Handlers
aliases:
  - "Microsoft.Maui.Handlers.ImageHandler.ConnectHandler"
declaring_type: "ImageHandler"
member_kind: method
---

# ImageHandler.ConnectHandler

> [!abstract] Method of [[ImageHandler|ImageHandler]]
> Namespace: `Microsoft.Maui.Handlers`

Connects the handler to the Android `AWebView` and registers platform-specific back navigation handling so that the WebView can consume back presses before the page is popped.

## Signatures

```csharp
void override ConnectHandler(Android.Widget.ImageView! platformView)
void override ConnectHandler(Microsoft.UI.Xaml.Controls.Image! platformView)
```

## Remarks

This override calls the base implementation and then registers an `OnBackPressed` lifecycle event handler. The handler checks `CanGoBack` and, when possible, navigates back within the WebView instead of allowing the back press (or predictive back gesture on Android 13+) to propagate and pop the containing page. When multiple BlazorWebView instances exist, the handler includes focus and visibility checks to ensure only the currently visible and focused WebView handles the back navigation, preventing conflicts between instances. Inheritors that override this method should call the base implementation to preserve this back navigation behavior unless they intentionally replace it.

## Parameters

| Parameter | Description |
|---|---|
| `platformView` | The native Android `AWebView` instance associated with this handler. |

## See also

- Declaring type: [[ImageHandler|ImageHandler]]
- [[_Microsoft.Maui.Handlers|Microsoft.Maui.Handlers namespace]]
