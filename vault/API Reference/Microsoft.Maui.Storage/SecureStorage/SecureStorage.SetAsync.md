---
title: "SecureStorage.SetAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Storage
aliases:
  - "Microsoft.Maui.Storage.SecureStorage.SetAsync"
declaring_type: "SecureStorage"
member_kind: method
---

# SecureStorage.SetAsync

> [!abstract] Method of [[SecureStorage|SecureStorage]]
> Namespace: `Microsoft.Maui.Storage`

Sets and encrypts a value for a given key.

## Signatures

```csharp
System.Threading.Tasks.Task! static SetAsync(string! key, string! value)
System.Threading.Tasks.Task! static SetAsync(string! key, string! value, Security.SecAccessible accessible)
```

## Parameters

| Parameter | Description |
|---|---|
| `key` | The key to set the value for. |
| `value` | Value to set. |

## Returns

A `Task` object with the current status of the asynchronous operation.

## See also

- Declaring type: [[SecureStorage|SecureStorage]]
- [[_Microsoft.Maui.Storage|Microsoft.Maui.Storage namespace]]
