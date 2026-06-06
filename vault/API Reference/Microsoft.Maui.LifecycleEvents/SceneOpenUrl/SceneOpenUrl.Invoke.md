---
title: "SceneOpenUrl.Invoke"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-LifecycleEvents
aliases:
  - "Microsoft.Maui.LifecycleEvents.iOSLifecycle.SceneOpenUrl.Invoke"
declaring_type: "SceneOpenUrl"
member_kind: method
---

# SceneOpenUrl.Invoke

> [!abstract] Method of [[SceneOpenUrl|SceneOpenUrl]]
> Namespace: `Microsoft.Maui.LifecycleEvents`

Invokes the delegate when an iOS scene is asked to open one or more URL contexts, returning whether they were handled.

## Signature

```csharp
bool virtual Invoke(UIKit.UIScene! scene, Foundation.NSSet<UIKit.UIOpenUrlContext!>! urlContexts)
```

## See also

- Declaring type: [[SceneOpenUrl|SceneOpenUrl]]
- [[_Microsoft.Maui.LifecycleEvents|Microsoft.Maui.LifecycleEvents namespace]]
