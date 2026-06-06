---
title: "Haptic Feedback"
description: "Learn how to use the .NET MAUI IHapticFeedback class in the Microsoft.Maui.Devices namespace. This interface lets you control haptic feedback on a device."
tags:
  - conceptual
  - area/platform-integration
ms_date: "01/07/2025"
source: "https://learn.microsoft.com/dotnet/maui/platform-integration/device/haptic-feedback?view=net-maui-10.0"
---

# Haptic feedback

[![Browse sample.](~/media/code-sample.png) Browse the sample](/samples/dotnet/maui-samples/platformintegration-essentials)

This article describes how you can use the .NET Multi-platform App UI (.NET MAUI) [[IHapticFeedback|IHapticFeedback]] interface to control haptic feedback on a device. Haptic feedback is generally manifested by a gentle vibration sensation provided by the device to give a response to the user. Some examples of haptic feedback are when a user types on a virtual keyboard or when they play a game where the player's character has an encounter with an enemy character.

The default implementation of the `IHapticFeedback` interface is available through the [[HapticFeedback.Default|HapticFeedback.Default]] property. Both the `IHapticFeedback` interface and `HapticFeedback` class are contained in the `Microsoft.Maui.Devices` namespace.

## Get started

To access the haptic feedback functionality, the following platform-specific setup is required.

<!-- markdownlint-disable MD025 -->
# [Android](#tab/android)

The `Vibrate` permission is required and must be configured in the Android project. This can be added in the following ways:

- Add the assembly-based permission:

  Open the _Platforms/Android/MainApplication.cs_ file and add the following assembly attributes after `using` directives:

  ```csharp
  [assembly: UsesPermission(Android.Manifest.Permission.Vibrate)]
  ```

  \- or -

- Update the Android Manifest:

  Open the _Platforms/Android/AndroidManifest.xml_ file and add the following in the `manifest` node:

  ```xml
  <uses-permission android:name="android.permission.VIBRATE" />
  ```

  \- or -

- Update the Android Manifest in the manifest editor:

  In Visual Studio double-click on the *Platforms/Android/AndroidManifest.xml* file to open the Android manifest editor. Then, under **Required permissions** check the **VIBRATE** permission. This will automatically update the *AndroidManifest.xml* file.

# [iOS/Mac Catalyst](#tab/macios)

No setup is required.

# [Windows](#tab/windows)

No setup is required.

-----
<!-- markdownlint-enable MD025 -->

## Use haptic feedback

The haptic feedback functionality is performed in two modes: a short [[HapticFeedbackType.Click|Click]] or a [[HapticFeedbackType.LongPress|LongPress]]. The following code example initiates a `Click` or `LongPress` haptic feedback response to the user based on which [[Button (Controls)|Button]] they click:

:::code language="csharp" source="../snippets/shared_1/DeviceDetailsPage.xaml.cs" id="hapticfeedback":::

> [!IMPORTANT]
> On Apple platforms, haptic feedback functionality must be executed on the UI thread.
