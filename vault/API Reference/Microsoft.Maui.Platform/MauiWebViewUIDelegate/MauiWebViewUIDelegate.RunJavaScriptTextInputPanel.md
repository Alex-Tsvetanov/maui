---
title: "MauiWebViewUIDelegate.RunJavaScriptTextInputPanel"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.MauiWebViewUIDelegate.RunJavaScriptTextInputPanel"
declaring_type: "MauiWebViewUIDelegate"
member_kind: method
---

# MauiWebViewUIDelegate.RunJavaScriptTextInputPanel

> [!abstract] Method of [[MauiWebViewUIDelegate|MauiWebViewUIDelegate]]
> Namespace: `Microsoft.Maui.Platform`

Displays a native text input panel in response to a JavaScript prompt request and invokes the completion handler with the entered text.

## Signature

```csharp
void override RunJavaScriptTextInputPanel(WebKit.WKWebView! webView, string! prompt, string? defaultText, WebKit.WKFrameInfo! frame, System.Action<string!>! completionHandler)
```

## See also

- Declaring type: [[MauiWebViewUIDelegate|MauiWebViewUIDelegate]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
