---
title: "IWebRequestInterceptingWebView.WebResourceRequested"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.IWebRequestInterceptingWebView.WebResourceRequested"
declaring_type: "IWebRequestInterceptingWebView"
member_kind: method
---

# IWebRequestInterceptingWebView.WebResourceRequested

> [!abstract] Method of [[IWebRequestInterceptingWebView|IWebRequestInterceptingWebView]]
> Namespace: `Microsoft.Maui`

Invoked when a web resource is requested. This event can be used to intercept requests and provide custom responses.

## Signature

```csharp
bool WebResourceRequested(Microsoft.Maui.WebResourceRequestedEventArgs! args)
```

## Returns

true if the request was handled; otherwise, false .

## Parameters

| Parameter | Description |
|---|---|
| `args` | The event arguments containing the request details. |

## See also

- Declaring type: [[IWebRequestInterceptingWebView|IWebRequestInterceptingWebView]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
