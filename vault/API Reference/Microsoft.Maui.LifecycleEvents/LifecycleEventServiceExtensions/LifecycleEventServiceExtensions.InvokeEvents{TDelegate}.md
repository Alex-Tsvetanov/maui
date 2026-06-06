---
title: "LifecycleEventServiceExtensions.InvokeEvents<TDelegate>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-LifecycleEvents
aliases:
  - "Microsoft.Maui.LifecycleEvents.LifecycleEventServiceExtensions.InvokeEvents<TDelegate>"
declaring_type: "LifecycleEventServiceExtensions"
member_kind: method
---

# LifecycleEventServiceExtensions.InvokeEvents<TDelegate>

> [!abstract] Method of [[LifecycleEventServiceExtensions|LifecycleEventServiceExtensions]]
> Namespace: `Microsoft.Maui.LifecycleEvents`

Invokes all delegates registered for the lifecycle event with the specified name.

## Signature

```csharp
void static InvokeEvents<TDelegate>(this Microsoft.Maui.LifecycleEvents.ILifecycleEventService! lifecycleService, string! eventName, System.Action<TDelegate!>! action)
```

## See also

- Declaring type: [[LifecycleEventServiceExtensions|LifecycleEventServiceExtensions]]
- [[_Microsoft.Maui.LifecycleEvents|Microsoft.Maui.LifecycleEvents namespace]]
