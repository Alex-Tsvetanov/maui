---
title: "WebAuthenticatorResult"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Authentication
aliases:
  - "Microsoft.Maui.Authentication.WebAuthenticatorResult"
namespace: "Microsoft.Maui.Authentication"
kind: class
platforms:
  - All platforms (.NET)
  - Android
  - iOS
  - Mac Catalyst
  - Windows
  - Tizen
  - .NET Standard
assemblies:
  - src
---

# WebAuthenticatorResult

> [!abstract] Class in `Microsoft.Maui.Authentication`
> Full name: `Microsoft.Maui.Authentication.WebAuthenticatorResult`

Represents a Web Authenticator Result object parsed from the callback Url.

## Platforms

| Platform | Available |
|---|---|
| All platforms (.NET) | ✅ |
| Android | ✅ |
| iOS | ✅ |
| Mac Catalyst | ✅ |
| Windows | ✅ |
| Tizen | ✅ |
| .NET Standard | ✅ |


## Constructors

| Name | Summary |
|---|---|
| [[WebAuthenticatorResult.WebAuthenticatorResult\|WebAuthenticatorResult]] | Initializes a new instance of the `WebAuthenticatorResult` class. |

## Properties

| Name | Summary |
|---|---|
| [[WebAuthenticatorResult.AccessToken\|AccessToken]] |  |
| [[WebAuthenticatorResult.CallbackUri\|CallbackUri]] | The uri that was used to call back with the access token. |
| [[WebAuthenticatorResult.ExpiresIn\|ExpiresIn]] |  |
| [[WebAuthenticatorResult.IdToken\|IdToken]] |  |
| [[WebAuthenticatorResult.Properties\|Properties]] |  |
| [[WebAuthenticatorResult.RefreshToken\|RefreshToken]] |  |
| [[WebAuthenticatorResult.RefreshTokenExpiresIn\|RefreshTokenExpiresIn]] |  |
| [[WebAuthenticatorResult.Timestamp\|Timestamp]] | The timestamp when the class was instantiated, which usually corresponds with the parsed result of a request. |

## Methods

| Name | Summary |
|---|---|
| [[WebAuthenticatorResult.Get\|Get]] | Gets a value for a given key from the dictionary. |
| [[WebAuthenticatorResult.Put\|Put]] | Puts a key/value pair into the dictionary. |

## Remarks

All of the query string or url fragment properties are parsed into a dictionary and can be accessed by their key.

## See also

- [[_Microsoft.Maui.Authentication|Microsoft.Maui.Authentication namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.authentication.webauthenticatorresult)
