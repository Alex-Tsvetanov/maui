---
title: "IPreferences"
tags:
  - api
  - kind/interface
  - ns/Microsoft-Maui-Storage
aliases:
  - "Microsoft.Maui.Storage.IPreferences"
namespace: "Microsoft.Maui.Storage"
kind: interface
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

# IPreferences

> [!abstract] Interface in `Microsoft.Maui.Storage`
> Full name: `Microsoft.Maui.Storage.IPreferences`

The Preferences API helps to store application preferences in a key/value store.

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


## Methods

| Name | Summary |
|---|---|
| [[IPreferences.Clear\|Clear]] |  |
| [[IPreferences.ContainsKey\|ContainsKey]] |  |
| [[IPreferences.Get{T}\|Get<T>]] |  |
| [[IPreferences.Remove\|Remove]] |  |
| [[IPreferences.Set{T}\|Set<T>]] |  |

## Remarks

Each platform uses the platform-provided APIs for storing application/user preferences: iOS: NSUserDefaults Android: SharedPreferences Windows: ApplicationDataContainer

## See also

- [[_Microsoft.Maui.Storage|Microsoft.Maui.Storage namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.storage.ipreferences)
