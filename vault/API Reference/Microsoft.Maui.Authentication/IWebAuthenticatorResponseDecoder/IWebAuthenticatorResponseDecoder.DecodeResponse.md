---
title: "IWebAuthenticatorResponseDecoder.DecodeResponse"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Authentication
aliases:
  - "Microsoft.Maui.Authentication.IWebAuthenticatorResponseDecoder.DecodeResponse"
declaring_type: "IWebAuthenticatorResponseDecoder"
member_kind: method
---

# IWebAuthenticatorResponseDecoder.DecodeResponse

> [!abstract] Method of [[IWebAuthenticatorResponseDecoder|IWebAuthenticatorResponseDecoder]]
> Namespace: `Microsoft.Maui.Authentication`

Decodes the given URIs query string into a dictionary.

## Signature

```csharp
System.Collections.Generic.IDictionary<string!, string!>? DecodeResponse(System.Uri! uri)
```

## Returns

A `IDictionary{TKey, TValue}` object where each of the query parameters values of `uri` are accessible through their respective keys.

## Parameters

| Parameter | Description |
|---|---|
| `uri` | The `Uri` object to decode the query parameters from. |

## See also

- Declaring type: [[IWebAuthenticatorResponseDecoder|IWebAuthenticatorResponseDecoder]]
- [[_Microsoft.Maui.Authentication|Microsoft.Maui.Authentication namespace]]
