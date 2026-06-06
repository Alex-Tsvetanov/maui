---
title: "IPlatformWebAuthenticatorCallback.OnResumeCallback"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Authentication
aliases:
  - "Microsoft.Maui.Authentication.IPlatformWebAuthenticatorCallback.OnResumeCallback"
declaring_type: "IPlatformWebAuthenticatorCallback"
member_kind: method
---

# IPlatformWebAuthenticatorCallback.OnResumeCallback

> [!abstract] Method of [[IPlatformWebAuthenticatorCallback|IPlatformWebAuthenticatorCallback]]
> Namespace: `Microsoft.Maui.Authentication`

The event that is triggered when an authentication flow calls back into the Android application.

## Signature

```csharp
bool OnResumeCallback(Android.Content.Intent! intent)
```

## Returns

`true` when the callback can be processed, otherwise `false`.

## Parameters

| Parameter | Description |
|---|---|
| `intent` | An `Intent` object containing additional data about this resume operation. |

## See also

- Declaring type: [[IPlatformWebAuthenticatorCallback|IPlatformWebAuthenticatorCallback]]
- [[_Microsoft.Maui.Authentication|Microsoft.Maui.Authentication namespace]]
