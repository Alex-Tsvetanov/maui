---
title: "BlazorWebViewHandler.TryDispatchAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-AspNetCore-Components-WebView-Maui
aliases:
  - "Microsoft.AspNetCore.Components.WebView.Maui.BlazorWebViewHandler.TryDispatchAsync"
declaring_type: "BlazorWebViewHandler"
member_kind: method
---

# BlazorWebViewHandler.TryDispatchAsync

> [!abstract] Method of [[BlazorWebViewHandler|BlazorWebViewHandler]]
> Namespace: `Microsoft.AspNetCore.Components.WebView.Maui`

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

- Declaring type: [[BlazorWebViewHandler|BlazorWebViewHandler]]
- [[_Microsoft.AspNetCore.Components.WebView.Maui|Microsoft.AspNetCore.Components.WebView.Maui namespace]]
