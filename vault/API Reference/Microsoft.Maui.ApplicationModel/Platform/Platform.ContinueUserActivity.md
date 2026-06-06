---
title: "Platform.ContinueUserActivity"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Platform.ContinueUserActivity"
declaring_type: "Platform"
member_kind: method
---

# Platform.ContinueUserActivity

> [!abstract] Method of [[Platform|Platform]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Informs the app that there is data associated with continuing a task specified as a `NSUserActivity` object, and then returns whether the app continued the activity.

## Signature

```csharp
bool static ContinueUserActivity(UIKit.UIApplication! application, Foundation.NSUserActivity! userActivity, UIKit.UIApplicationRestorationHandler! completionHandler)
```

## Parameters

| Parameter | Description |
|---|---|
| `application` | The application this action is invoked on. |
| `userActivity` | The user activity identifier. |
| `completionHandler` | The action that is invoked when the operation is completed. |

## Returns

`true` if the app handled the user activity, otherwise `false`.

## See also

- Declaring type: [[Platform|Platform]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
