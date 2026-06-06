---
title: "BlazorWebView (Wpf).TryDispatchAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-AspNetCore-Components-WebView-Wpf
aliases:
  - "Microsoft.AspNetCore.Components.WebView.Wpf.BlazorWebView.TryDispatchAsync"
declaring_type: "BlazorWebView (Wpf)"
member_kind: method
---

# BlazorWebView (Wpf).TryDispatchAsync

> [!abstract] Method of [[BlazorWebView (Wpf)|BlazorWebView (Wpf)]]
> Namespace: `Microsoft.AspNetCore.Components.WebView.Wpf`

Calls the specified `workItem` asynchronously and passes in the scoped services available to Razor components.

## Signature

```csharp
System.Threading.Tasks.Task<bool>! virtual TryDispatchAsync(System.Action<System.IServiceProvider!>! workItem)
```

## Returns

Returns a `Task` representing true if the `workItem` was called, or false if it was not called because Blazor is not currently running.

## Parameters

| Parameter | Description |
|---|---|
| `workItem` | The action to call. |

## See also

- Declaring type: [[BlazorWebView (Wpf)|BlazorWebView (Wpf)]]
- [[_Microsoft.AspNetCore.Components.WebView.Wpf|Microsoft.AspNetCore.Components.WebView.Wpf namespace]]
