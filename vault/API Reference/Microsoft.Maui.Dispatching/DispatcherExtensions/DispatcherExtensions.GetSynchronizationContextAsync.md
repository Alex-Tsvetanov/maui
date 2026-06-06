---
title: "DispatcherExtensions.GetSynchronizationContextAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Dispatching
aliases:
  - "Microsoft.Maui.Dispatching.DispatcherExtensions.GetSynchronizationContextAsync"
declaring_type: "DispatcherExtensions"
member_kind: method
---

# DispatcherExtensions.GetSynchronizationContextAsync

> [!abstract] Method of [[DispatcherExtensions|DispatcherExtensions]]
> Namespace: `Microsoft.Maui.Dispatching`

Gets the synchronization context for the current thread.

## Signature

```csharp
System.Threading.Tasks.Task<System.Threading.SynchronizationContext!>! static GetSynchronizationContextAsync(this Microsoft.Maui.Dispatching.IDispatcher! dispatcher)
```

## Returns

A `SynchronizationContext` object representing the current synchronization context.

## Parameters

| Parameter | Description |
|---|---|
| `dispatcher` | The `IDispatcher` instance this method is called on. |

## See also

- Declaring type: [[DispatcherExtensions|DispatcherExtensions]]
- [[_Microsoft.Maui.Dispatching|Microsoft.Maui.Dispatching namespace]]
