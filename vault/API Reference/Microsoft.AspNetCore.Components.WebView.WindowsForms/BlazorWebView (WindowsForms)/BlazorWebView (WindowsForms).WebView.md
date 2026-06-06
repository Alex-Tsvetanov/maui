---
title: "BlazorWebView (WindowsForms).WebView"
tags:
  - api
  - member/property
  - ns/Microsoft-AspNetCore-Components-WebView-WindowsForms
aliases:
  - "Microsoft.AspNetCore.Components.WebView.WindowsForms.BlazorWebView.WebView"
declaring_type: "BlazorWebView (WindowsForms)"
member_kind: property
---

# BlazorWebView (WindowsForms).WebView

> [!abstract] Property of [[BlazorWebView (WindowsForms)|BlazorWebView (WindowsForms)]]
> Namespace: `Microsoft.AspNetCore.Components.WebView.WindowsForms`

Returns the inner `WebView2Control` used by this control.

## Signature

```csharp
Microsoft.Web.WebView2.WinForms.WebView2! WebView { get; }
```

## Remarks

Directly using some functionality of the inner web view can cause unexpected results because its behavior is controlled by the `BlazorWebView` that is hosting it.

## See also

- Declaring type: [[BlazorWebView (WindowsForms)|BlazorWebView (WindowsForms)]]
- [[_Microsoft.AspNetCore.Components.WebView.WindowsForms|Microsoft.AspNetCore.Components.WebView.WindowsForms namespace]]
