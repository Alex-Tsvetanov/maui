---
title: "IWebAuthenticator.AuthenticateAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Authentication
aliases:
  - "Microsoft.Maui.Authentication.IWebAuthenticator.AuthenticateAsync"
declaring_type: "IWebAuthenticator"
member_kind: method
---

# IWebAuthenticator.AuthenticateAsync

> [!abstract] Method of [[IWebAuthenticator|IWebAuthenticator]]
> Namespace: `Microsoft.Maui.Authentication`

Begin an authentication flow by navigating to the specified URL and waiting for a callback/redirect to the callback URL scheme.

## Signatures

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.Authentication.WebAuthenticatorResult!>! AuthenticateAsync(Microsoft.Maui.Authentication.WebAuthenticatorOptions! webAuthenticatorOptions, System.Threading.CancellationToken cancellationToken)
System.Threading.Tasks.Task<Microsoft.Maui.Authentication.WebAuthenticatorResult!>! AuthenticateAsync(Microsoft.Maui.Authentication.WebAuthenticatorOptions! webAuthenticatorOptions)
```

## Returns

A `WebAuthenticatorResult` object with the results of this operation.

## Parameters

| Parameter | Description |
|---|---|
| `webAuthenticatorOptions` | A `WebAuthenticatorOptions` instance containing additional configuration for this authentication call. |

## See also

- Declaring type: [[IWebAuthenticator|IWebAuthenticator]]
- [[_Microsoft.Maui.Authentication|Microsoft.Maui.Authentication namespace]]
