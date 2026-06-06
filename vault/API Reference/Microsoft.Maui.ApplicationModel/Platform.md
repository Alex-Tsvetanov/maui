---
title: "Platform"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Platform"
namespace: "Microsoft.Maui.ApplicationModel"
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

# Platform

> [!abstract] Class in `Microsoft.Maui.ApplicationModel`
> Full name: `Microsoft.Maui.ApplicationModel.Platform`

A static class that contains platform-specific helper methods.

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
| [[Platform.AppContext\|AppContext]] | Gets the `Context` object that represents the current application context. |
| [[Platform.CurrentActivity\|CurrentActivity]] |  |
| [[Platform.CurrentPackage\|CurrentPackage]] |  |
| [[Platform.MapServiceToken\|MapServiceToken]] |  |

## Methods

| Name | Summary |
|---|---|
| [[Platform.ContinueUserActivity\|ContinueUserActivity]] | Informs the app that there is data associated with continuing a task specified as a `NSUserActivity` object, and then returns whether the app continued the a… |
| [[Platform.GetCurrentUIViewController\|GetCurrentUIViewController]] | Gets the current view controller through the `WindowStateManager`. |
| [[Platform.Init\|Init]] | Initializes the `WindowStateManager` for the given `UIViewController`. |
| [[Platform.OnActivated\|OnActivated]] |  |
| [[Platform.OnLaunched\|OnLaunched]] | Gets or sets the map service API key for this platform. |
| [[Platform.OnNewIntent\|OnNewIntent]] | Called when a new `Intent` was created as part of invoking an app action. |
| [[Platform.OnPlatformWindowInitialized\|OnPlatformWindowInitialized]] |  |
| [[Platform.OnRequestPermissionsResult\|OnRequestPermissionsResult]] | Pass permission request results from an activity's overridden method to the library for handling internal permission requests. |
| [[Platform.OnResume\|OnResume]] | Called when a `Activity` is resumed as part of invoking an app action. |
| [[Platform.OpenUrl\|OpenUrl]] | Opens the specified URI to start a authentication flow. |
| [[Platform.PerformActionForShortcutItem\|PerformActionForShortcutItem]] | Invokes the action that corresponds to the chosen `AppAction` by the user. |
| [[Platform.WaitForActivityAsync\|WaitForActivityAsync]] |  |

## Events

| Name | Summary |
|---|---|
| [[Platform.ActivityStateChanged\|ActivityStateChanged]] |  |

## See also

- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.applicationmodel.platform)
