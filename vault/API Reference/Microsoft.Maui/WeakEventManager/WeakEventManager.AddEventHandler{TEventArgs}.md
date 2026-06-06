---
title: "WeakEventManager.AddEventHandler<TEventArgs>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.WeakEventManager.AddEventHandler<TEventArgs>"
declaring_type: "WeakEventManager"
member_kind: method
---

# WeakEventManager.AddEventHandler<TEventArgs>

> [!abstract] Method of [[WeakEventManager|WeakEventManager]]
> Namespace: `Microsoft.Maui`

Adds an event handler for the specified event, storing a weak reference to the handler's target.

## Signature

```csharp
void AddEventHandler<TEventArgs>(System.EventHandler<TEventArgs!>! handler, string! eventName = "")
```

## Parameters

| Parameter | Description |
|---|---|
| `handler` | The event handler to add. |
| `eventName` | The name of the event. |

## See also

- Declaring type: [[WeakEventManager|WeakEventManager]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
