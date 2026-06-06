---
title: "IPlatformWebAuthenticatorCallback.OpenUrlCallback"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Authentication
aliases:
  - "Microsoft.Maui.Authentication.IPlatformWebAuthenticatorCallback.OpenUrlCallback"
declaring_type: "IPlatformWebAuthenticatorCallback"
member_kind: method
---

# IPlatformWebAuthenticatorCallback.OpenUrlCallback

> [!abstract] Method of [[IPlatformWebAuthenticatorCallback|IPlatformWebAuthenticatorCallback]]
> Namespace: `Microsoft.Maui.Authentication`

Opens the specified URI to start the authentication flow.

## Signature

```csharp
bool OpenUrlCallback(System.Uri! uri)
```

## Returns

`true` when the URI has been opened, otherwise `false`.

## Parameters

| Parameter | Description |
|---|---|
| `uri` | The URI to open that will start the authentication flow. |

## See also

- Declaring type: [[IPlatformWebAuthenticatorCallback|IPlatformWebAuthenticatorCallback]]
- [[_Microsoft.Maui.Authentication|Microsoft.Maui.Authentication namespace]]
