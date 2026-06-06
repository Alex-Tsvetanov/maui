---
title: "WeakEventManager.HandleEvent"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.WeakEventManager.HandleEvent"
declaring_type: "WeakEventManager"
member_kind: method
---

# WeakEventManager.HandleEvent

> [!abstract] Method of [[WeakEventManager|WeakEventManager]]
> Namespace: `Microsoft.Maui`

Invokes the handlers registered for the specified event. Removes handlers whose targets have been garbage collected.

## Signature

```csharp
void HandleEvent(object? sender, object? args, string! eventName)
```

## Parameters

| Parameter | Description |
|---|---|
| `sender` | The source of the event. |
| `args` | The event arguments. |
| `eventName` | The name of the event to raise. |

## See also

- Declaring type: [[WeakEventManager|WeakEventManager]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
