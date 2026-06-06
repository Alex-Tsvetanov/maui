---
title: "Preferences"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Storage
aliases:
  - "Microsoft.Maui.Storage.Preferences"
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

# Preferences

> [!abstract] Class in `Microsoft.Maui.Storage`
> Full name: `Microsoft.Maui.Storage.Preferences`

Checks for the existence of a given key.

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
| [[Preferences.Default\|Default]] | Provides the default implementation for static usage of this API. |

## Methods

| Name | Summary |
|---|---|
| [[Preferences.Clear\|Clear]] | Clears all keys and values. |
| [[Preferences.ContainsKey\|ContainsKey]] | Checks the existence of a given key. |
| [[Preferences.Get\|Get]] | Gets the value for a given key, or the default specified if the key does not exist. |
| [[Preferences.Remove\|Remove]] | Removes a key and its associated value if it exists. |
| [[Preferences.Set\|Set]] | Sets a value for a given key. |

## Remarks

Each platform uses the platform provided native APIs for storing application/user preferences: iOS: NSUserDefaults Android: SharedPreferences Windows: ApplicationDataContainer

## Guide

- 📖 Conceptual: [[preferences]]

## See also

- [[_Microsoft.Maui.Storage|Microsoft.Maui.Storage namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.storage.preferences)
