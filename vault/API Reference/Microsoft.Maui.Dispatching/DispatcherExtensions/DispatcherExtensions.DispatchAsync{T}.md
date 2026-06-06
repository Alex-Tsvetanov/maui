---
title: "DispatcherExtensions.DispatchAsync<T>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Dispatching
aliases:
  - "Microsoft.Maui.Dispatching.DispatcherExtensions.DispatchAsync<T>"
declaring_type: "DispatcherExtensions"
member_kind: method
---

# DispatcherExtensions.DispatchAsync<T>

> [!abstract] Method of [[DispatcherExtensions|DispatcherExtensions]]
> Namespace: `Microsoft.Maui.Dispatching`

Schedules the provided callback on the UI thread from a worker thread, and returns the results asynchronously.

## Signatures

```csharp
System.Threading.Tasks.Task<T>! static DispatchAsync<T>(this Microsoft.Maui.Dispatching.IDispatcher! dispatcher, System.Func<System.Threading.Tasks.Task<T>!>! funcTask)
System.Threading.Tasks.Task<T>! static DispatchAsync<T>(this Microsoft.Maui.Dispatching.IDispatcher! dispatcher, System.Func<T>! func)
```

## Returns

A `Task{TResult}` object containing information about the state of the dispatcher operation.

## Parameters

| Parameter | Description |
|---|---|
| `dispatcher` | The `IDispatcher` instance this method is called on. |
| `func` | The method to be executed by the dispatcher. |

## See also

- Declaring type: [[DispatcherExtensions|DispatcherExtensions]]
- [[_Microsoft.Maui.Dispatching|Microsoft.Maui.Dispatching namespace]]
