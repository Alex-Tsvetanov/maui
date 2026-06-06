---
title: "ISecureStorage.GetAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Storage
aliases:
  - "Microsoft.Maui.Storage.ISecureStorage.GetAsync"
declaring_type: "ISecureStorage"
member_kind: method
---

# ISecureStorage.GetAsync

> [!abstract] Method of [[ISecureStorage|ISecureStorage]]
> Namespace: `Microsoft.Maui.Storage`

Gets and decrypts the value for a given key.

## Signature

```csharp
System.Threading.Tasks.Task<string?>! GetAsync(string! key)
```

## Returns

The decrypted string value or `null` if a value was not found.

## Parameters

| Parameter | Description |
|---|---|
| `key` | The key to retrieve the value for. |

## See also

- Declaring type: [[ISecureStorage|ISecureStorage]]
- [[_Microsoft.Maui.Storage|Microsoft.Maui.Storage namespace]]
