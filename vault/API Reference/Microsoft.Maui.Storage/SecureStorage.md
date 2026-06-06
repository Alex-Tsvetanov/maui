---
title: "SecureStorage"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Storage
aliases:
  - "Microsoft.Maui.Storage.SecureStorage"
namespace: "Microsoft.Maui.Storage"
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

# SecureStorage

> [!abstract] Class in `Microsoft.Maui.Storage`
> Full name: `Microsoft.Maui.Storage.SecureStorage`

Default `SecAccessible` to use for all Get/Set calls to KeyChain. Default value is `AfterFirstUnlock`.

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


## Properties

| Name | Summary |
|---|---|
| [[SecureStorage.Default\|Default]] | Provides the default implementation for static usage of this API. |
| [[SecureStorage.DefaultAccessible\|DefaultAccessible]] |  |

## Methods

| Name | Summary |
|---|---|
| [[SecureStorage.GetAsync\|GetAsync]] |  |
| [[SecureStorage.Remove\|Remove]] | Removes a key and its associated value if it exists. |
| [[SecureStorage.RemoveAll\|RemoveAll]] | Removes all of the stored encrypted key/value pairs. |
| [[SecureStorage.SetAsync\|SetAsync]] | Sets and encrypts a value for a given key. |

## Remarks

Each platform uses the platform provided APIs for storing data securely: iOS Data is stored in KeyChain. Additional information on SecAccessible at: `SecAccessible`. Android Encryption keys are stored in KeyStore and encrypted data is stored in a named shared preference container (PackageId.microsoft.maui.essentials.preferences). Windows Data is encrypted with DataProtectionProvider and stored in a named ApplicationDataContainer (with a container name of ApplicationId.microsoft.maui.essentials.preferences). NOTE: On Android devices running below API 23 (6.0 Marshmallow) there is no AES available in KeyStore. As a best practice this API will generate an RSA/ECB/PKCS7Padding key pair stored in KeyStore (the only type supported in KeyStore by these lower API levels), which is used to wrap an AES key generated at runtime. This wrapped key is stored in Preferences.

## Guide

- 📖 Conceptual: [[secure-storage]]

## See also

- [[_Microsoft.Maui.Storage|Microsoft.Maui.Storage namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.storage.securestorage)
