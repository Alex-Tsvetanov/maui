---
title: "PhoneDialer"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-ApplicationModel-Communication
aliases:
  - "Microsoft.Maui.ApplicationModel.Communication.PhoneDialer"
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

# PhoneDialer

> [!abstract] Class in `Microsoft.Maui.ApplicationModel.Communication`
> Full name: `Microsoft.Maui.ApplicationModel.Communication.PhoneDialer`

Gets a value indicating whether using the phone dialer is supported on this device.

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
| [[PhoneDialer.Default\|Default]] | Provides the default implementation for static usage of this API. |
| [[PhoneDialer.IsSupported\|IsSupported]] | Gets a value indicating whether using the phone dialer is supported on this device. |

## Methods

| Name | Summary |
|---|---|
| [[PhoneDialer.Open\|Open]] | Open the phone dialer to a specific phone number. |

## Remarks

Will throw `ArgumentNullException` if `number` is not valid. Will throw `FeatureNotSupportedException` if making phone calls is not supported on the device.

## Guide

- 📖 Conceptual: [[phone-dialer]]

## See also

- [[_Microsoft.Maui.ApplicationModel.Communication|Microsoft.Maui.ApplicationModel.Communication namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.applicationmodel.communication.phonedialer)
