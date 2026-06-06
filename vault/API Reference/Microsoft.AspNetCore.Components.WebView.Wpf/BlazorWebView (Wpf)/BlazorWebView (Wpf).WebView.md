---
title: "BlazorWebView (Wpf).WebView"
tags:
  - api
  - member/property
  - ns/Microsoft-AspNetCore-Components-WebView-Wpf
aliases:
  - "Microsoft.AspNetCore.Components.WebView.Wpf.BlazorWebView.WebView"
declaring_type: "BlazorWebView (Wpf)"
member_kind: property
---

# BlazorWebView (Wpf).WebView

> [!abstract] Property of [[BlazorWebView (Wpf)|BlazorWebView (Wpf)]]
> Namespace: `Microsoft.AspNetCore.Components.WebView.Wpf`

Returns the inner `WebView2Control` used by this control.

## Signature

```csharp
Microsoft.Web.WebView2.Wpf.WebView2CompositionControl! WebView { get; }
```

## Remarks

Directly using some functionality of the inner web view can cause unexpected results because its behavior is controlled by the `BlazorWebView` that is hosting it.

## See also

- Declaring type: [[BlazorWebView (Wpf)|BlazorWebView (Wpf)]]
- [[_Microsoft.AspNetCore.Components.WebView.Wpf|Microsoft.AspNetCore.Components.WebView.Wpf namespace]]
