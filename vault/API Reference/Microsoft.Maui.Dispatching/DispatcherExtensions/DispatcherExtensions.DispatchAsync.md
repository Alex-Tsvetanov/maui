---
title: "DispatcherExtensions.DispatchAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Dispatching
aliases:
  - "Microsoft.Maui.Dispatching.DispatcherExtensions.DispatchAsync"
declaring_type: "DispatcherExtensions"
member_kind: method
---

# DispatcherExtensions.DispatchAsync

> [!abstract] Method of [[DispatcherExtensions|DispatcherExtensions]]
> Namespace: `Microsoft.Maui.Dispatching`

Schedules the provided action on the UI thread from a worker thread.

## Signatures

```csharp
System.Threading.Tasks.Task! static DispatchAsync(this Microsoft.Maui.Dispatching.IDispatcher! dispatcher, System.Action! action)
System.Threading.Tasks.Task! static DispatchAsync(this Microsoft.Maui.Dispatching.IDispatcher! dispatcher, System.Func<System.Threading.Tasks.Task!>! funcTask)
```

## Parameters

| Parameter | Description |
|---|---|
| `dispatcher` | The `IDispatcher` instance this method is called on. |
| `action` | The method to be executed by the dispatcher. |

## Returns

`Task`.

## See also

- Declaring type: [[DispatcherExtensions|DispatcherExtensions]]
- [[_Microsoft.Maui.Dispatching|Microsoft.Maui.Dispatching namespace]]
