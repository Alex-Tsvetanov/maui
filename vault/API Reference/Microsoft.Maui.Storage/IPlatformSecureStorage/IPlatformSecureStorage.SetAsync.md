---
title: "IPlatformSecureStorage.SetAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Storage
aliases:
  - "Microsoft.Maui.Storage.IPlatformSecureStorage.SetAsync"
declaring_type: "IPlatformSecureStorage"
member_kind: method
---

# IPlatformSecureStorage.SetAsync

> [!abstract] Method of [[IPlatformSecureStorage|IPlatformSecureStorage]]
> Namespace: `Microsoft.Maui.Storage`

Sets and encrypts a value for a given key.

## Signature

```csharp
System.Threading.Tasks.Task! SetAsync(string! key, string! value, Security.SecAccessible accessible)
```

## Returns

A `Task` object with the current status of the asynchronous operation.

## Parameters

| Parameter | Description |
|---|---|
| `key` | The key to set the value for. |
| `value` | Value to set. |
| `accessible` | The KeyChain accessibility to create the encrypted record with. |

## See also

- Declaring type: [[IPlatformSecureStorage|IPlatformSecureStorage]]
- [[_Microsoft.Maui.Storage|Microsoft.Maui.Storage namespace]]
