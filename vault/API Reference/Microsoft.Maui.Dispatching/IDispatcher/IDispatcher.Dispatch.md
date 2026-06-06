---
title: "IDispatcher.Dispatch"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Dispatching
aliases:
  - "Microsoft.Maui.Dispatching.IDispatcher.Dispatch"
declaring_type: "IDispatcher"
member_kind: method
---

# IDispatcher.Dispatch

> [!abstract] Method of [[IDispatcher|IDispatcher]]
> Namespace: `Microsoft.Maui.Dispatching`

Schedules the provided action on the UI thread from a worker thread.

## Signature

```csharp
bool Dispatch(System.Action! action)
```

## Returns

`true` when the action has been dispatched successfully, otherwise `false`.

## Parameters

| Parameter | Description |
|---|---|
| `action` | The `Action` to be scheduled for processing on the UI thread. |

## See also

- Declaring type: [[IDispatcher|IDispatcher]]
- [[_Microsoft.Maui.Dispatching|Microsoft.Maui.Dispatching namespace]]
