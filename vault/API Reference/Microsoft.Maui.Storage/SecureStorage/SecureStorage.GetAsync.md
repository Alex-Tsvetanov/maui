---
title: "SecureStorage.GetAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Storage
aliases:
  - "Microsoft.Maui.Storage.SecureStorage.GetAsync"
declaring_type: "SecureStorage"
member_kind: method
---

# SecureStorage.GetAsync

> [!abstract] Method of [[SecureStorage|SecureStorage]]
> Namespace: `Microsoft.Maui.Storage`

Gets and decrypts the value for a given key.

## Signature

```csharp
System.Threading.Tasks.Task<string?>! static GetAsync(string! key)
```

## Returns

The decrypted string value or `null` if a value was not found.

## Parameters

| Parameter | Description |
|---|---|
| `key` | The key to retrieve the value for. |

## See also

- Declaring type: [[SecureStorage|SecureStorage]]
- [[_Microsoft.Maui.Storage|Microsoft.Maui.Storage namespace]]
