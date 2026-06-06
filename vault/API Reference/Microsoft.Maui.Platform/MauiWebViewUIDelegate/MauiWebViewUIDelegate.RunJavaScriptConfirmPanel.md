---
title: "MauiWebViewUIDelegate.RunJavaScriptConfirmPanel"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.MauiWebViewUIDelegate.RunJavaScriptConfirmPanel"
declaring_type: "MauiWebViewUIDelegate"
member_kind: method
---

# MauiWebViewUIDelegate.RunJavaScriptConfirmPanel

> [!abstract] Method of [[MauiWebViewUIDelegate|MauiWebViewUIDelegate]]
> Namespace: `Microsoft.Maui.Platform`

Displays a native confirm panel in response to a JavaScript confirm request and invokes the completion handler with the result.

## Signature

```csharp
void override RunJavaScriptConfirmPanel(WebKit.WKWebView! webView, string! message, WebKit.WKFrameInfo! frame, System.Action<bool>! completionHandler)
```

## See also

- Declaring type: [[MauiWebViewUIDelegate|MauiWebViewUIDelegate]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
