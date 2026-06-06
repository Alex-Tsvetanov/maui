---
title: "Sms (Communication)"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-ApplicationModel-Communication
aliases:
  - "Microsoft.Maui.ApplicationModel.Communication.Sms"
namespace: "Microsoft.Maui.ApplicationModel.Communication"
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

# Sms (Communication)

> [!abstract] Class in `Microsoft.Maui.ApplicationModel.Communication`
> Full name: `Microsoft.Maui.ApplicationModel.Communication.Sms`

Gets a value indicating whether composing of SMS messages is supported on this device.

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
| [[Sms (Communication).Default\|Default]] | Provides the default implementation for static usage of this API. |

## Methods

| Name | Summary |
|---|---|
| [[Sms (Communication).ComposeAsync\|ComposeAsync]] | Opens the default SMS client to allow the user to send the message. |

## Remarks

When using this on Android targeting Android 11 (R API 30) you must update your Android Manifest with queries that are used with the new package visibility requirements. See the conceptual docs for more information.

## See also

- [[_Microsoft.Maui.ApplicationModel.Communication|Microsoft.Maui.ApplicationModel.Communication namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.applicationmodel.communication.sms)
