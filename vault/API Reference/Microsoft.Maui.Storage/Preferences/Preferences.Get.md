---
title: "Preferences.Get"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Storage
aliases:
  - "Microsoft.Maui.Storage.Preferences.Get"
declaring_type: "Preferences"
member_kind: method
---

# Preferences.Get

> [!abstract] Method of [[Preferences|Preferences]]
> Namespace: `Microsoft.Maui.Storage`

Gets the value for a given key, or the default specified if the key does not exist.

## Signatures

```csharp
bool static Get(string! key, bool defaultValue, string? sharedName)
bool static Get(string! key, bool defaultValue)
double static Get(string! key, double defaultValue, string? sharedName)
double static Get(string! key, double defaultValue)
float static Get(string! key, float defaultValue, string? sharedName)
float static Get(string! key, float defaultValue)
int static Get(string! key, int defaultValue, string? sharedName)
int static Get(string! key, int defaultValue)
long static Get(string! key, long defaultValue, string? sharedName)
long static Get(string! key, long defaultValue)
string? static Get(string! key, string? defaultValue, string? sharedName)
string? static Get(string! key, string? defaultValue)
System.DateTime static Get(string! key, System.DateTime defaultValue, string? sharedName)
System.DateTime static Get(string! key, System.DateTime defaultValue)
System.DateTimeOffset static Get(string! key, System.DateTimeOffset defaultValue, string? sharedName)
System.DateTimeOffset static Get(string! key, System.DateTimeOffset defaultValue)
```

## Parameters

| Parameter | Description |
|---|---|
| `key` | The key to retrieve the value for. |
| `defaultValue` | The default value to return when no existing value for `key` exists. |

## Returns

Value for the given key, or the value in `defaultValue` if it does not exist.

## See also

- Declaring type: [[Preferences|Preferences]]
- [[_Microsoft.Maui.Storage|Microsoft.Maui.Storage namespace]]
