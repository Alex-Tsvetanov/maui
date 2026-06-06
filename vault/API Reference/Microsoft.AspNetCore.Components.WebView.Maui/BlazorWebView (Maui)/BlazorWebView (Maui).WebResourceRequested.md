---
title: "BlazorWebView (Maui).WebResourceRequested"
tags:
  - api
  - member/event
  - ns/Microsoft-AspNetCore-Components-WebView-Maui
aliases:
  - "Microsoft.AspNetCore.Components.WebView.Maui.BlazorWebView.WebResourceRequested"
declaring_type: "BlazorWebView (Maui)"
member_kind: event
---

# BlazorWebView (Maui).WebResourceRequested

> [!abstract] Event of [[BlazorWebView (Maui)|BlazorWebView (Maui)]]
> Namespace: `Microsoft.AspNetCore.Components.WebView.Maui`

Raised when a web resource is requested. This event allows the application to intercept the request and provide a custom response. The event handler can set the `Handled` property to true to indicate that the request has been handled and no further processing is needed. If the event handler does set this property to true, it must also call the `SetResponse` or `SetResponse` method to provide a response to the request.

## Signature

```csharp
System.EventHandler<Microsoft.Maui.Controls.WebViewWebResourceRequestedEventArgs!>? WebResourceRequested
```

## See also

- Declaring type: [[BlazorWebView (Maui)|BlazorWebView (Maui)]]
- [[_Microsoft.AspNetCore.Components.WebView.Maui|Microsoft.AspNetCore.Components.WebView.Maui namespace]]
