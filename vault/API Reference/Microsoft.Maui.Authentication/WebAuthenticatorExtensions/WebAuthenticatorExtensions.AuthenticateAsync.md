---
title: "WebAuthenticatorExtensions.AuthenticateAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Authentication
aliases:
  - "Microsoft.Maui.Authentication.WebAuthenticatorExtensions.AuthenticateAsync"
declaring_type: "WebAuthenticatorExtensions"
member_kind: method
---

# WebAuthenticatorExtensions.AuthenticateAsync

> [!abstract] Method of [[WebAuthenticatorExtensions|WebAuthenticatorExtensions]]
> Namespace: `Microsoft.Maui.Authentication`

Begin an authentication flow by navigating to the specified url and waiting for a callback/redirect to the callbackUrl scheme.

## Signatures

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.Authentication.WebAuthenticatorResult!>! static AuthenticateAsync(this Microsoft.Maui.Authentication.IWebAuthenticator! webAuthenticator, System.Uri! url, System.Uri! callbackUrl, System.Threading.CancellationToken cancellationToken)
System.Threading.Tasks.Task<Microsoft.Maui.Authentication.WebAuthenticatorResult!>! static AuthenticateAsync(this Microsoft.Maui.Authentication.IWebAuthenticator! webAuthenticator, System.Uri! url, System.Uri! callbackUrl)
```

## Returns

Returns a result parsed out from the callback url.

## Parameters

| Parameter | Description |
|---|---|
| `webAuthenticator` | The `IWebAuthenticator` to use for the authentication flow. |
| `url` | Url to navigate to, beginning the authentication flow. |
| `callbackUrl` | Expected callback url that the navigation flow will eventually redirect to. |

## See also

- Declaring type: [[WebAuthenticatorExtensions|WebAuthenticatorExtensions]]
- [[_Microsoft.Maui.Authentication|Microsoft.Maui.Authentication namespace]]
