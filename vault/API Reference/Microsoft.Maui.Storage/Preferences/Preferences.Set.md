---
title: "Preferences.Set"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Storage
aliases:
  - "Microsoft.Maui.Storage.Preferences.Set"
declaring_type: "Preferences"
member_kind: method
---

# Preferences.Set

> [!abstract] Method of [[Preferences|Preferences]]
> Namespace: `Microsoft.Maui.Storage`

Sets a value for a given key.

## Signatures

```csharp
void static Set(string! key, bool value, string? sharedName)
void static Set(string! key, bool value)
void static Set(string! key, double value, string? sharedName)
void static Set(string! key, double value)
void static Set(string! key, float value, string? sharedName)
void static Set(string! key, float value)
void static Set(string! key, int value, string? sharedName)
void static Set(string! key, int value)
void static Set(string! key, long value, string? sharedName)
void static Set(string! key, long value)
void static Set(string! key, string? value, string? sharedName)
void static Set(string! key, string? value)
void static Set(string! key, System.DateTime value, string? sharedName)
void static Set(string! key, System.DateTime value)
void static Set(string! key, System.DateTimeOffset value, string? sharedName)
void static Set(string! key, System.DateTimeOffset value)
```

## Parameters

| Parameter | Description |
|---|---|
| `key` | The key to set the value for. |
| `value` | Value to set. |

## See also

- Declaring type: [[Preferences|Preferences]]
- [[_Microsoft.Maui.Storage|Microsoft.Maui.Storage namespace]]
