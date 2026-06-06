---
title: "DispatcherExtensions.StartTimer"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Dispatching
aliases:
  - "Microsoft.Maui.Dispatching.DispatcherExtensions.StartTimer"
declaring_type: "DispatcherExtensions"
member_kind: method
---

# DispatcherExtensions.StartTimer

> [!abstract] Method of [[DispatcherExtensions|DispatcherExtensions]]
> Namespace: `Microsoft.Maui.Dispatching`

Starts a timer on the specified `IDispatcher` context.

## Signature

```csharp
void static StartTimer(this Microsoft.Maui.Dispatching.IDispatcher! dispatcher, System.TimeSpan interval, System.Func<bool>! callback)
```

## Parameters

| Parameter | Description |
|---|---|
| `dispatcher` | The `IDispatcher` instance this method is called on. |
| `interval` | Sets the amount of time between timer ticks. |
| `callback` | The callback on which the dispatcher returns when the event is dispatched. If the result of the callback is `true`, the timer will repeat, otherwise the timer stops. |

## See also

- Declaring type: [[DispatcherExtensions|DispatcherExtensions]]
- [[_Microsoft.Maui.Dispatching|Microsoft.Maui.Dispatching namespace]]
