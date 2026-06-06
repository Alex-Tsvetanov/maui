---
title: "WebViewExtensions (Platform).Eval"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.WebViewExtensions.Eval"
declaring_type: "WebViewExtensions (Platform)"
member_kind: method
---

# WebViewExtensions (Platform).Eval

> [!abstract] Method of [[WebViewExtensions (Platform)|WebViewExtensions (Platform)]]
> Namespace: `Microsoft.Maui.Platform`

Evaluates the specified script in the native web view associated with the cross-platform web view.

## Signatures

```csharp
void static Eval(this Android.Webkit.WebView! platformWebView, Microsoft.Maui.IWebView! webView, string! script)
void static Eval(this WebKit.WKWebView! platformWebView, Microsoft.Maui.IWebView! webView, string! script)
void static Eval(this Microsoft.UI.Xaml.Controls.WebView2! platformWebView, Microsoft.Maui.IWebView! webView, string! script)
```

## See also

- Declaring type: [[WebViewExtensions (Platform)|WebViewExtensions (Platform)]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
