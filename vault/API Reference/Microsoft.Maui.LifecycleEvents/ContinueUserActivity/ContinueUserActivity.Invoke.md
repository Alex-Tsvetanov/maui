---
title: "ContinueUserActivity.Invoke"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-LifecycleEvents
aliases:
  - "Microsoft.Maui.LifecycleEvents.iOSLifecycle.ContinueUserActivity.Invoke"
declaring_type: "ContinueUserActivity"
member_kind: method
---

# ContinueUserActivity.Invoke

> [!abstract] Method of [[ContinueUserActivity|ContinueUserActivity]]
> Namespace: `Microsoft.Maui.LifecycleEvents`

Invokes the delegate when iOS asks the application to continue an activity, returning whether the activity was handled.

## Signature

```csharp
bool virtual Invoke(UIKit.UIApplication! application, Foundation.NSUserActivity! userActivity, UIKit.UIApplicationRestorationHandler! completionHandler)
```

## See also

- Declaring type: [[ContinueUserActivity|ContinueUserActivity]]
- [[_Microsoft.Maui.LifecycleEvents|Microsoft.Maui.LifecycleEvents namespace]]
