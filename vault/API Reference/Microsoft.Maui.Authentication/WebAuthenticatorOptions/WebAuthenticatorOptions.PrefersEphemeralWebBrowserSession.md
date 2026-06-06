---
title: "WebAuthenticatorOptions.PrefersEphemeralWebBrowserSession"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Authentication
aliases:
  - "Microsoft.Maui.Authentication.WebAuthenticatorOptions.PrefersEphemeralWebBrowserSession"
declaring_type: "WebAuthenticatorOptions"
member_kind: property
---

# WebAuthenticatorOptions.PrefersEphemeralWebBrowserSession

> [!abstract] Property of [[WebAuthenticatorOptions|WebAuthenticatorOptions]]
> Namespace: `Microsoft.Maui.Authentication`

Gets or sets whether the browser used for the authentication flow is short-lived. This means it will not share session nor cookies with the regular browser on this device if set the `true`.

## Signature

```csharp
bool PrefersEphemeralWebBrowserSession { get; set; }
```

## Remarks

This setting only has effect on iOS.

## See also

- Declaring type: [[WebAuthenticatorOptions|WebAuthenticatorOptions]]
- [[_Microsoft.Maui.Authentication|Microsoft.Maui.Authentication namespace]]
