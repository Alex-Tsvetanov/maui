---
title: "OnRequestPermissionsResult.Invoke"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-LifecycleEvents
aliases:
  - "Microsoft.Maui.LifecycleEvents.AndroidLifecycle.OnRequestPermissionsResult.Invoke"
declaring_type: "OnRequestPermissionsResult"
member_kind: method
---

# OnRequestPermissionsResult.Invoke

> [!abstract] Method of [[OnRequestPermissionsResult|OnRequestPermissionsResult]]
> Namespace: `Microsoft.Maui.LifecycleEvents`

Invokes the delegate when an Android runtime permission request returns its grant results.

## Signature

```csharp
void virtual Invoke(Android.App.Activity! activity, int requestCode, string![]! permissions, Android.Content.PM.Permission[]! grantResults)
```

## See also

- Declaring type: [[OnRequestPermissionsResult|OnRequestPermissionsResult]]
- [[_Microsoft.Maui.LifecycleEvents|Microsoft.Maui.LifecycleEvents namespace]]
