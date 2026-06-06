---
title: "IPreferences.Get<T>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Storage
aliases:
  - "Microsoft.Maui.Storage.IPreferences.Get<T>"
declaring_type: "IPreferences"
member_kind: method
---

# IPreferences.Get<T>

> [!abstract] Method of [[IPreferences|IPreferences]]
> Namespace: `Microsoft.Maui.Storage`

Gets the value for a given key, or the default specified if the key does not exist.

## Signature

```csharp
T Get<T>(string! key, T defaultValue, string? sharedName = null)
```

## Returns

Value for the given key, or the value in `defaultValue` if it does not exist.

## Parameters

| Parameter | Description |
|---|---|
| `key` | The key to retrieve the value for. |
| `defaultValue` | The default value to return when no existing value for `key` exists. |
| `sharedName` | Shared container name. |

## See also

- Declaring type: [[IPreferences|IPreferences]]
- [[_Microsoft.Maui.Storage|Microsoft.Maui.Storage namespace]]
