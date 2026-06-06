---
title: "SecureStorageExtensions.SetAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Storage
aliases:
  - "Microsoft.Maui.Storage.SecureStorageExtensions.SetAsync"
declaring_type: "SecureStorageExtensions"
member_kind: method
---

# SecureStorageExtensions.SetAsync

> [!abstract] Method of [[SecureStorageExtensions|SecureStorageExtensions]]
> Namespace: `Microsoft.Maui.Storage`

Sets and encrypts a value for a given key.

## Signature

```csharp
System.Threading.Tasks.Task! static SetAsync(this Microsoft.Maui.Storage.ISecureStorage! secureStorage, string! key, string! value, Security.SecAccessible accessible)
```

## Parameters

| Parameter | Description |
|---|---|
| `secureStorage` | The object this method is invoked on. |
| `key` | The key to set the value for. |
| `value` | Value to set. |
| `accessible` | The KeyChain accessibility to create the encrypted record with. |

## Returns

A `Task` object with the current status of the asynchronous operation.

## See also

- Declaring type: [[SecureStorageExtensions|SecureStorageExtensions]]
- [[_Microsoft.Maui.Storage|Microsoft.Maui.Storage namespace]]
