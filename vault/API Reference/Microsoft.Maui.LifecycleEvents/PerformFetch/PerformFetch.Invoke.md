---
title: "PerformFetch.Invoke"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-LifecycleEvents
aliases:
  - "Microsoft.Maui.LifecycleEvents.iOSLifecycle.PerformFetch.Invoke"
declaring_type: "PerformFetch"
member_kind: method
---

# PerformFetch.Invoke

> [!abstract] Method of [[PerformFetch|PerformFetch]]
> Namespace: `Microsoft.Maui.LifecycleEvents`

Invokes the delegate when iOS asks the application to perform a background fetch.

## Signature

```csharp
void virtual Invoke(UIKit.UIApplication! application, System.Action<UIKit.UIBackgroundFetchResult>! completionHandler)
```

## See also

- Declaring type: [[PerformFetch|PerformFetch]]
- [[_Microsoft.Maui.LifecycleEvents|Microsoft.Maui.LifecycleEvents namespace]]
