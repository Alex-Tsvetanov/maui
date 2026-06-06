---
title: "OpenUrl.Invoke"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-LifecycleEvents
aliases:
  - "Microsoft.Maui.LifecycleEvents.iOSLifecycle.OpenUrl.Invoke"
declaring_type: "OpenUrl"
member_kind: method
---

# OpenUrl.Invoke

> [!abstract] Method of [[OpenUrl|OpenUrl]]
> Namespace: `Microsoft.Maui.LifecycleEvents`

Invokes the delegate when iOS asks the application to open a URL, returning whether the URL was handled.

## Signature

```csharp
bool virtual Invoke(UIKit.UIApplication! app, Foundation.NSUrl! url, Foundation.NSDictionary! options)
```

## See also

- Declaring type: [[OpenUrl|OpenUrl]]
- [[_Microsoft.Maui.LifecycleEvents|Microsoft.Maui.LifecycleEvents namespace]]
