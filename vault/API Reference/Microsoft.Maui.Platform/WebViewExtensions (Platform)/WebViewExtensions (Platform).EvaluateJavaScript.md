---
title: "WebViewExtensions (Platform).EvaluateJavaScript"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.WebViewExtensions.EvaluateJavaScript"
declaring_type: "WebViewExtensions (Platform)"
member_kind: method
---

# WebViewExtensions (Platform).EvaluateJavaScript

> [!abstract] Method of [[WebViewExtensions (Platform)|WebViewExtensions (Platform)]]
> Namespace: `Microsoft.Maui.Platform`

Evaluates JavaScript in the native web view and returns its result via the specified request.

## Signatures

```csharp
void static EvaluateJavaScript(this Android.Webkit.WebView! webView, Microsoft.Maui.EvaluateJavaScriptAsyncRequest! request)
void static EvaluateJavaScript(this WebKit.WKWebView! webView, Microsoft.Maui.EvaluateJavaScriptAsyncRequest! request)
void static EvaluateJavaScript(this Microsoft.Maui.Platform.MauiWebView! platformWebView, Microsoft.Maui.EvaluateJavaScriptAsyncRequest! request)
void static EvaluateJavaScript(this Microsoft.UI.Xaml.Controls.WebView2! webView, Microsoft.Maui.EvaluateJavaScriptAsyncRequest! request)
```

## See also

- Declaring type: [[WebViewExtensions (Platform)|WebViewExtensions (Platform)]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
