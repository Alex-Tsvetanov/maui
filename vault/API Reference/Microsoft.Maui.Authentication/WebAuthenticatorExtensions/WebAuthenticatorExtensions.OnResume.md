---
title: "WebAuthenticatorExtensions.OnResume"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Authentication
aliases:
  - "Microsoft.Maui.Authentication.WebAuthenticatorExtensions.OnResume"
declaring_type: "WebAuthenticatorExtensions"
member_kind: method
---

# WebAuthenticatorExtensions.OnResume

> [!abstract] Method of [[WebAuthenticatorExtensions|WebAuthenticatorExtensions]]
> Namespace: `Microsoft.Maui.Authentication`

The event that is triggered when an authentication flow calls back into the Android application.

## Signature

```csharp
bool static OnResume(this Microsoft.Maui.Authentication.IWebAuthenticator! webAuthenticator, Android.Content.Intent! intent)
```

## Returns

`true` when the callback can be processed, otherwise `false`.

## Parameters

| Parameter | Description |
|---|---|
| `intent` | An `Intent` object containing additional data about this resume operation. |

## See also

- Declaring type: [[WebAuthenticatorExtensions|WebAuthenticatorExtensions]]
- [[_Microsoft.Maui.Authentication|Microsoft.Maui.Authentication namespace]]
