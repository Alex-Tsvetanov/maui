---
title: "SecureStorageExtensions.SetDefaultAccessible"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Storage
aliases:
  - "Microsoft.Maui.Storage.SecureStorageExtensions.SetDefaultAccessible"
declaring_type: "SecureStorageExtensions"
member_kind: method
---

# SecureStorageExtensions.SetDefaultAccessible

> [!abstract] Method of [[SecureStorageExtensions|SecureStorageExtensions]]
> Namespace: `Microsoft.Maui.Storage`

Sets the default KeyChain accessibility used to encrypt data.

## Signature

```csharp
void static SetDefaultAccessible(this Microsoft.Maui.Storage.ISecureStorage! secureStorage, Security.SecAccessible accessible)
```

## Parameters

| Parameter | Description |
|---|---|
| `secureStorage` | The object this method is invoked on. |
| `accessible` | The default KeyChain accessibility to set. |

## Returns

The current default `SecAccessible` value.

## See also

- Declaring type: [[SecureStorageExtensions|SecureStorageExtensions]]
- [[_Microsoft.Maui.Storage|Microsoft.Maui.Storage namespace]]
