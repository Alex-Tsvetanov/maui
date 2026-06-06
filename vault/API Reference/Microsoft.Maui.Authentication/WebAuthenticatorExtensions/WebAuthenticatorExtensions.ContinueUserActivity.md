---
title: "WebAuthenticatorExtensions.ContinueUserActivity"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Authentication
aliases:
  - "Microsoft.Maui.Authentication.WebAuthenticatorExtensions.ContinueUserActivity"
declaring_type: "WebAuthenticatorExtensions"
member_kind: method
---

# WebAuthenticatorExtensions.ContinueUserActivity

> [!abstract] Method of [[WebAuthenticatorExtensions|WebAuthenticatorExtensions]]
> Namespace: `Microsoft.Maui.Authentication`

Informs the app that there is data associated with continuing a task specified as a `NSUserActivity` object, and then returns whether the app continued the activity.

## Signature

```csharp
bool static ContinueUserActivity(this Microsoft.Maui.Authentication.IWebAuthenticator! webAuthenticator, UIKit.UIApplication! application, Foundation.NSUserActivity! userActivity, UIKit.UIApplicationRestorationHandler! completionHandler)
```

## Returns

`true` if the app handled the user activity, otherwise `false`.

## Parameters

| Parameter | Description |
|---|---|
| `application` | The application this action is invoked on. |
| `userActivity` | The user activity identifier. |
| `completionHandler` | The action that is invoked when the operation is completed. |

## See also

- Declaring type: [[WebAuthenticatorExtensions|WebAuthenticatorExtensions]]
- [[_Microsoft.Maui.Authentication|Microsoft.Maui.Authentication namespace]]
