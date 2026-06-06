---
title: "IDispatcher.DispatchDelayed"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Dispatching
aliases:
  - "Microsoft.Maui.Dispatching.IDispatcher.DispatchDelayed"
declaring_type: "IDispatcher"
member_kind: method
---

# IDispatcher.DispatchDelayed

> [!abstract] Method of [[IDispatcher|IDispatcher]]
> Namespace: `Microsoft.Maui.Dispatching`

Schedules the provided action on the UI thread from a worker thread, taking into account the provided delay.

## Signature

```csharp
bool DispatchDelayed(System.TimeSpan delay, System.Action! action)
```

## Returns

`true` when the action has been dispatched successfully, otherwise `false`.

## Parameters

| Parameter | Description |
|---|---|
| `delay` | The delay taken into account before `action` is dispatched. |
| `action` | The `Action` to be scheduled for processing on the UI thread. |

## See also

- Declaring type: [[IDispatcher|IDispatcher]]
- [[_Microsoft.Maui.Dispatching|Microsoft.Maui.Dispatching namespace]]
