---
title: "WebAuthenticatorResult.IdToken"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Authentication
aliases:
  - "Microsoft.Maui.Authentication.WebAuthenticatorResult.IdToken"
declaring_type: "WebAuthenticatorResult"
member_kind: property
---

# WebAuthenticatorResult.IdToken

> [!abstract] Property of [[WebAuthenticatorResult|WebAuthenticatorResult]]
> Namespace: `Microsoft.Maui.Authentication`

The value for the `id_token` key.

## Signature

```csharp
string IdToken { get; }
```

## Remarks

Apple doesn't return an access token on iOS native sign in, but it does return id_token as a JWT.

## See also

- Declaring type: [[WebAuthenticatorResult|WebAuthenticatorResult]]
- [[_Microsoft.Maui.Authentication|Microsoft.Maui.Authentication namespace]]
