---
title: "BlazorWebView (WindowsForms).TryDispatchAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-AspNetCore-Components-WebView-WindowsForms
aliases:
  - "Microsoft.AspNetCore.Components.WebView.WindowsForms.BlazorWebView.TryDispatchAsync"
declaring_type: "BlazorWebView (WindowsForms)"
member_kind: method
---

# BlazorWebView (WindowsForms).TryDispatchAsync

> [!abstract] Method of [[BlazorWebView (WindowsForms)|BlazorWebView (WindowsForms)]]
> Namespace: `Microsoft.AspNetCore.Components.WebView.WindowsForms`

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

- Declaring type: [[BlazorWebView (WindowsForms)|BlazorWebView (WindowsForms)]]
- [[_Microsoft.AspNetCore.Components.WebView.WindowsForms|Microsoft.AspNetCore.Components.WebView.WindowsForms namespace]]
