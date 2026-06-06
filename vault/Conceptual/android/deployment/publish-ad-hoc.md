---
title: "Publish a .NET MAUI Android app for ad-hoc distribution"
description: "Learn how to publish a .NET MAUI Android app for ad-hoc distribution."
tags:
  - conceptual
  - area/android
ms_date: "09/30/2024"
source: "https://learn.microsoft.com/dotnet/maui/android/deployment/publish-ad-hoc?view=net-maui-10.0"
---

# Publish an Android app for ad-hoc distribution

> [!div class="op_single_selector"]
>
> - [[publish-google-play|Publish for Google Play distribution]]
> - [[publish-cli|Publish using the command line]]

When distributing Android apps outside Google Play, and other marketplaces, *ad-hoc* distribution enables you to make the app available for download on a website or server. Android requires that apps created for ad-hoc distribution use the Android Package (APK) format.

To distribute a .NET Multi-platform App UI (.NET MAUI) Android app, you'll need to sign it with a key from your keystore. Keystores are binary files that serve as repositories of certificates and private keys.

The process for publishing a .NET MAUI Android app for ad-hoc distribution is as follows:

1. Ensure your app uses the correct package format. For more information, see [Ensure correct package format](#ensure-correct-package-format).
1. Build and sign your app in Visual Studio. For more information, see [Distribute your app through Visual Studio](#distribute-your-app-through-visual-studio).

## Ensure correct package format

By default, the package format for .NET MAUI Android release builds is AAB. To publish a .NET MAUI Android app for ad-hoc distribution requires that you first change the package format to APK:

1. In **Solution Explorer** right-click on your .NET MAUI app project and select **Properties**. Then, navigate to the **Android > Options** tab and ensure that the value of the **Release** field is set to **apk**:

    ![](media/publish/vs/ad-hoc-change-package-format.png)

## Distribute your app through Visual Studio

![[publish-vs]]

![[publish-ad-hoc]]

The app can then be distributed to Android devices through a website or server. When users browse to a download link from their Android device, the file is downloaded. Android will automatically start installing it on the device, provided that the user has configured their settings to allow the installation of apps from unknown sources. For more information about opting into allowing apps from unknown sources, see [User opt-in for unknown apps and sources](https://developer.android.com/studio/publish#publishing-unknown) on developer.android.com.
