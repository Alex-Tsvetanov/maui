---
title: "HybridWebView.WebResourceRequested"
tags:
  - api
  - member/event
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.HybridWebView.WebResourceRequested"
declaring_type: "HybridWebView"
member_kind: event
---

# HybridWebView.WebResourceRequested

> [!abstract] Event of [[HybridWebView|HybridWebView]]
> Namespace: `Microsoft.Maui.Controls`

Raised when a web resource is requested. This event allows the application to intercept the request and provide a custom response. The event handler can set the `Handled` property to true to indicate that the request has been handled and no further processing is needed. If the event handler does set this property to true, it must also call the `SetResponse` or `SetResponse` method to provide a response to the request.

## Signature

```csharp
System.EventHandler<Microsoft.Maui.Controls.WebViewWebResourceRequestedEventArgs!>? WebResourceRequested
```

## See also

- Declaring type: [[HybridWebView|HybridWebView]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
