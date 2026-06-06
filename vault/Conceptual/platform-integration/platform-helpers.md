---
title: "Platform helpers"
description: "The Platform class, in the Microsoft.Maui.ApplicationModel, contains helpers for each platform."
tags:
  - conceptual
  - area/platform-integration
ms_date: "03/27/2023"
source: "https://learn.microsoft.com/dotnet/maui/platform-integration/platform-helpers?view=net-maui-10.0"
---

# Platform helpers

The static `Platform` class, in the `ApplicationModel` namespace, contains helpers for each platform.

<!-- markdownlint-disable MD025 -->
# [Android](#tab/android)

The `Platform` class contains the following helpers on Android:

| Member | Purpose |
| ------ | ------- |
| `ActivityStateChanged` | An event that's raised when any Activity's state changes. |
| `AppContext` | A property that gets the `Context` object that represents the current app context. |
| `CurrentActivity` | A property that gets the current `Activity` object that represents the current activity. |
| `Intent` | A static class that contains the `ActionAppAction` string, which is the identifier for the `Intent` used by app actions. |
| `OnNewIntent` | Pass an `Intent` from an activity's overridden method, when invoking an app action. |
| `OnResume` | Pass an `Activity` from an activity's overridden method, when an `Activity` is resumed as part of invoking an app action. |
| `OnRequestPermissionsResult` | Pass permission request results from an activity's overridden method, for handling internal permission requests. |
| `WaitForActivityAsync` | Wait for an `Activity` to be created or become active. |

The following example shows how to access the current `Context` or `Activity` for the running app:

```csharp
var context = Platform.AppContext;

// Current Activity or null if not initialized or not started.
var activity = Platform.CurrentActivity;
```

If there's a situation where the `Activity` is needed, but the app hasn't fully started, call the `WaitForActivityAsync` method:

```csharp
var activity = await Platform.WaitForActivityAsync();
```

# [iOS/Mac Catalyst](#tab/macios)

The `Platform` class contains the following helper methods on iOS and Mac Catalyst:

| Method | Purpose |
| ------ | ------- |
| `ContinueUserActivity` | Informs the app that there's data associated with continuing a task specified as a `NSUserActivity` object, and then returns whether the app continued the activity. |
| `GetCurrentUIViewController` | Gets the current view controller. This method will return `null` if unable to detect a `UIViewController`. |
| `OpenUrl` | Opens the specified URI to start an authentication flow. |
| `PerformActionForShortcutItem` | Invokes the action that corresponds to the chosen `AppAction` by the user. |

The following example shows how to retrieve the currently visible `UIViewController`:

```csharp
var viewController = Platform.GetCurrentUIViewController();
```

# [Windows](#tab/windows)

The `Platform` class contains the following helpers on Windows:

| Method | Purpose |
| ------ | ------- |
| `MapServiceToken` | A property that gets or sets the map service API key. |
| `OnLaunched` | The lifecycle method that's called when the app is launched. |
| `OnActivated` | Sets the app's new active window. |

---
<!-- markdownlint-enable MD025 -->
