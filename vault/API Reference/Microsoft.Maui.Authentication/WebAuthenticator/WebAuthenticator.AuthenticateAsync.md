---
title: "WebAuthenticator.AuthenticateAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Authentication
aliases:
  - "Microsoft.Maui.Authentication.WebAuthenticator.AuthenticateAsync"
declaring_type: "WebAuthenticator"
member_kind: method
---

# WebAuthenticator.AuthenticateAsync

> [!abstract] Method of [[WebAuthenticator|WebAuthenticator]]
> Namespace: `Microsoft.Maui.Authentication`

Asynchronously begins a web authentication flow using the specified options and returns the authentication result.

## Signatures

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.Authentication.WebAuthenticatorResult!>! static AuthenticateAsync(Microsoft.Maui.Authentication.WebAuthenticatorOptions! webAuthenticatorOptions, System.Threading.CancellationToken cancellationToken)
System.Threading.Tasks.Task<Microsoft.Maui.Authentication.WebAuthenticatorResult!>! static AuthenticateAsync(Microsoft.Maui.Authentication.WebAuthenticatorOptions! webAuthenticatorOptions)
System.Threading.Tasks.Task<Microsoft.Maui.Authentication.WebAuthenticatorResult!>! static AuthenticateAsync(System.Uri! url, System.Uri! callbackUrl, System.Threading.CancellationToken cancellationToken)
System.Threading.Tasks.Task<Microsoft.Maui.Authentication.WebAuthenticatorResult!>! static AuthenticateAsync(System.Uri! url, System.Uri! callbackUrl)
```

## See also

- Declaring type: [[WebAuthenticator|WebAuthenticator]]
- [[_Microsoft.Maui.Authentication|Microsoft.Maui.Authentication namespace]]
