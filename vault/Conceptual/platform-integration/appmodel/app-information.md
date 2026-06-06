---
title: "App Information"
description: "Describes the IAppInfo interface in the Microsoft.Maui.ApplicationModel namespace, which provides information about your application. For example, it exposes the app name and version."
tags:
  - conceptual
  - area/platform-integration
ms_date: "08/07/2023"
source: "https://learn.microsoft.com/dotnet/maui/platform-integration/appmodel/app-information?view=net-maui-10.0"
---

# App information

[![Browse sample.](~/media/code-sample.png) Browse the sample](/samples/dotnet/maui-samples/platformintegration-essentials)

This article describes how you can use the .NET Multi-platform App UI (.NET MAUI) [[IAppInfo|IAppInfo]] interface, which provides information about your application.

The default implementation of the `IAppInfo` interface is available through the [[AppInfo.Current|AppInfo.Current]] property. Both the `IAppInfo` interface and `AppInfo` class are contained in the `Microsoft.Maui.ApplicationModel` namespace.

## Read the app information

The `IAppInfo` interface exposes the following properties:

- [[IAppInfo.Name|Name]] &mdash; The application name.
- [[IAppInfo.PackageName|PackageName]] &mdash; The package name or application identifier, such as `com.microsoft.myapp`.
- [[IAppInfo.VersionString|VersionString]] &mdash; The application version, such as `1.0.0`.
- [[IAppInfo.Version|Version]] &mdash; The application version, as a `Version` object.
- [[IAppInfo.BuildString|BuildString]] &mdash; The build number of the version, such as `1000`.
- [[IAppInfo.RequestedTheme|RequestedTheme]] &mdash; The detected theme of the system or application.
- [[IAppInfo.PackagingModel|PackagingModel]] &mdash; The packaging model of the application.
- [[IAppInfo.RequestedLayoutDirection|RequestedLayoutDirection]] &mdash; The requested layout direction of the system or application.

The following code example demonstrates accessing some of these properties:

:::code language="csharp" source="../snippets/shared_1/AppModelPage.xaml.cs" id="read_info":::

## Get the current theme

The [[IAppInfo.RequestedTheme|RequestedTheme]] property provides the current requested theme by the system for your application. One of the following values is returned:

- [[AppTheme.Unspecified|Unspecified]]
- [[AppTheme.Light|Light]]
- [[AppTheme.Dark|Dark]]

`Unspecified` is returned when the operating system doesn't have a specific user interface style. An example of this is on devices running versions of iOS older than 13.0.

The following code example demonstrates getting the theme:

:::code language="csharp" source="../snippets/shared_1/AppModelPage.xaml.cs" id="read_theme":::

## Get the layout direction

The [[IAppInfo.RequestedLayoutDirection|RequestedLayoutDirection]] property provides the current layout direction used by the system for your application. One of the following values is returned:

- [[LayoutDirection.Unknown|Unknown]]
- [[LayoutDirection.LeftToRight|LeftToRight]]
- [[LayoutDirection.RightToLeft|RightToLeft]]

`Unknown` is returned when the layout direction is unknown.

The following code example demonstrates getting the layout direction:

```csharp
LayoutDirection layoutDirection = AppInfo.Current.RequestedLayoutDirection;
```

## Display app settings

The [[IAppInfo|IAppInfo]] class can also display a page of settings maintained by the operating system for the application:

:::code language="csharp" source="../snippets/shared_1/AppModelPage.xaml.cs" id="show_settings":::

This settings page allows the user to change application permissions and perform other platform-specific tasks.

## Platform implementation specifics

This section describes platform-specific implementation details related to the [[IAppInfo|IAppInfo]] interface.

<!-- markdownlint-disable MD025 -->

# [Android](#tab/android)

App information is taken from the _AndroidManifest.xml_ for the following fields:

- [[IAppInfo.BuildString|BuildString]] &mdash; `android:versionCode` in `manifest` node
- [[IAppInfo.Name|Name]] &mdash; `android:label` in the `application` node
- [[IAppInfo.PackageName|PackageName]] &mdash; `package` in the `manifest` node
- [[IAppInfo.VersionString|VersionString]] &mdash; `android:versionName` in the `manifest` node

### Requested theme

Android uses configuration modes to specify the type of theme to request from the user. Based on the version of Android, it can be changed by the user or may be changed when battery saver mode is enabled.

You can read more on the official [Android documentation for Dark Theme](https://developer.android.com/guide/topics/ui/look-and-feel/darktheme).

# [iOS/Mac Catalyst](#tab/macios)

App information is taken from the _Info.plist_ for the following fields:

- [[IAppInfo.BuildString|BuildString]] &mdash; `CFBundleVersion`
- [[IAppInfo.Name|Name]] &mdash; `CFBundleDisplayName` if set, else `CFBundleName`
- [[IAppInfo.PackageName|PackageName]] &mdash; `CFBundleIdentifier`
- [[IAppInfo.VersionString|VersionString]] &mdash; `CFBundleShortVersionString`

### Requested theme

_Unspecified_ is always returned on versions of iOS older than 13.0

# [Windows](#tab/windows)

App information is taken from the _Package.appxmanifest_ for the following fields:

- [[IAppInfo.BuildString|BuildString]] &mdash; Uses the `Build` from the `Version` on the `Identity` node
- [[IAppInfo.Name|Name]] &mdash; `DisplayName` on the `Properties` node
- [[IAppInfo.PackageName|PackageName]] &mdash; `Name` on the `Identity` node
- [[IAppInfo.VersionString|VersionString]] &mdash; `Version` on the `Identity` node

### Requested theme

Code that accesses the `IAppInfo.RequestedTheme` property must be called on the UI thread or an exception will be thrown.

Windows applications respect the `RequestedTheme` property setting in the Windows _App.xaml_. If it's set to a specific theme, this API always returns this setting. To use the dynamic theme of the OS, remove this property from your application. When your app is run, it returns the theme set by the user in Windows settings: **Settings** > **Personalization** > **Colors** > **Choose your default app mode**.

<!-- TODO: You can read more on the [Windows Requested Theme Documentation](/uwp/api/windows.ui.xaml.application.requestedtheme). -->

--------------

<!-- markdownlint-enable MD025 -->
