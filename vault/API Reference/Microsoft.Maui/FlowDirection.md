---
title: "FlowDirection"
tags:
  - api
  - kind/enum
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.FlowDirection"
namespace: "Microsoft.Maui"
kind: enum
platforms:
  - All platforms (.NET)
  - Android
  - iOS
  - Mac Catalyst
  - Windows
  - Tizen
  - .NET Standard
  - .NET Standard 2.0
assemblies:
  - src
---

# FlowDirection

> [!abstract] Enum in `Microsoft.Maui`
> Full name: `Microsoft.Maui.FlowDirection`

Enumerates values that control the layout direction for views.

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
| .NET Standard 2.0 | ✅ |


## Fields

| Name | Summary |
|---|---|
| [[FlowDirection.LeftToRight\|LeftToRight]] |  |
| [[FlowDirection.MatchParent\|MatchParent]] |  |
| [[FlowDirection.RightToLeft\|RightToLeft]] |  |

## Remarks

The default value for an element without a parent is `LeftToRight`, even on platforms with right-to-left device defaults. To opt in to right-to-left layout, set the root element's FlowDirection property to `RightToLeft`, or use `MatchParent` to follow the device. All elements with a parent default to `MatchParent`.

## See also

- [[_Microsoft.Maui|Microsoft.Maui namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.flowdirection)
