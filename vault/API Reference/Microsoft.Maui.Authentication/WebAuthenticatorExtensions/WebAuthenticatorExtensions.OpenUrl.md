---
title: "WebAuthenticatorExtensions.OpenUrl"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Authentication
aliases:
  - "Microsoft.Maui.Authentication.WebAuthenticatorExtensions.OpenUrl"
declaring_type: "WebAuthenticatorExtensions"
member_kind: method
---

# WebAuthenticatorExtensions.OpenUrl

> [!abstract] Method of [[WebAuthenticatorExtensions|WebAuthenticatorExtensions]]
> Namespace: `Microsoft.Maui.Authentication`

Opens the specified URI to start the authentication flow.

## Signatures

```csharp
bool static OpenUrl(this Microsoft.Maui.Authentication.IWebAuthenticator! webAuthenticator, System.Uri! uri)
bool static OpenUrl(this Microsoft.Maui.Authentication.IWebAuthenticator! webAuthenticator, UIKit.UIApplication! app, Foundation.NSUrl! url, Foundation.NSDictionary! options)
```

## Returns

`true` when the URI has been opened, otherwise `false`.

## Parameters

| Parameter | Description |
|---|---|
| `uri` | The URI to open that will start the authentication flow. |

## See also

- Declaring type: [[WebAuthenticatorExtensions|WebAuthenticatorExtensions]]
- [[_Microsoft.Maui.Authentication|Microsoft.Maui.Authentication namespace]]
