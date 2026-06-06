---
title: "Open the browser"
description: "The IBrowser interface in the Microsoft.Maui.ApplicationModel namespace enables an application to open a web link in the optimized system preferred browser or the external browser."
tags:
  - conceptual
  - area/platform-integration
ms_date: "02/02/2023"
source: "https://learn.microsoft.com/dotnet/maui/platform-integration/appmodel/open-browser?view=net-maui-10.0"
---

# Browser

[![Browse sample.](~/media/code-sample.png) Browse the sample](/samples/dotnet/maui-samples/platformintegration-essentials)

This article describes how you can use the .NET Multi-platform App UI (.NET MAUI) [[IBrowser|IBrowser]] interface. This interface enables an application to open a web link in the system-preferred browser or the external browser.

The default implementation of the `IBrowser` interface is available through the [[Browser.Default|Browser.Default]] property. Both the `IBrowser` interface and `Browser` class are contained in the `Microsoft.Maui.ApplicationModel` namespace.

## Get started

To access the browser functionality, the following platform-specific setup is required.

<!-- markdownlint-disable MD025 -->
# [Android](#tab/android)

If your project's Target Android version is set to **Android 11 (R API 30)** or higher, you must update your _Android Manifest_ with queries that use Android's [package visibility requirements](https://developer.android.com/preview/privacy/package-visibility).

In the _Platforms/Android/AndroidManifest.xml_ file, add the following `queries/intent` nodes in the `manifest` node:

```xml
<queries>
  <intent>
    <action android:name="android.intent.action.VIEW" />
    <data android:scheme="http"/>
  </intent>
  <intent>
    <action android:name="android.intent.action.VIEW" />
    <data android:scheme="https"/>
  </intent>
</queries>
```

# [iOS/Mac Catalyst](#tab/macios)

No setup is required.

# [Windows](#tab/windows)

No setup is required.

-----
<!-- markdownlint-enable MD025 -->

## Open the browser

The browser is opened by calling the `IBrowser.OpenAsync%2A` method with the `Uri` and the type of [[BrowserLaunchMode|BrowserLaunchMode]]. The following code example demonstrates opening the browser:

:::code language="csharp" source="../snippets/shared_1/AppModelPage.xaml.cs" id="browser_open":::

This method returns after the browser is launched, not after it's closed by the user. `Browser.OpenAsync` returns a `bool` value to indicate if the browser was successfully launched.

## Customization

If you're using the system-preferred browser, there are several customization options available for iOS and Android. These options include a `TitleMode` (Android only) and preferred color for the `Toolbar` (iOS and Android) and `Controls` (iOS only) that appear.

Specify these options using [[BrowserLaunchOptions|BrowserLaunchOptions]] when you call `OpenAsync`.

:::code language="csharp" source="../snippets/shared_1/AppModelPage.xaml.cs" id="browser_open_custom":::

## Platform differences

This section describes the platform-specific differences with the browser API.

<!-- markdownlint-disable MD025 -->
<!-- markdownlint-disable MD024 -->
### [Android](#tab/android)

The [[BrowserLaunchOptions.LaunchMode|BrowserLaunchOptions.LaunchMode]] determines how the browser is launched:

- [[BrowserLaunchMode.SystemPreferred|SystemPreferred]]

  [Custom Tabs](https://developer.chrome.com/multidevice/android/customtabs) are used to load the URI and keep navigation awareness.

- [[BrowserLaunchMode.External|External]]

  An `Intent` is used to request the URI be opened through the system's normal browser.

# [iOS/Mac Catalyst](#tab/macios)

The [[BrowserLaunchOptions.LaunchMode|BrowserLaunchOptions.LaunchMode]] determines how the browser is launched:

- [[BrowserLaunchMode.SystemPreferred|SystemPreferred]]

  SFSafariViewController is used to load the URI and keep navigation awareness.

- [[BrowserLaunchMode.External|External]]

  The standard `OpenUrl` on the main application is used to launch the default browser outside of the application.

# [Windows](#tab/windows)

The user's default browser is always launched regardless of the [[BrowserLaunchMode|BrowserLaunchMode]].

-----
<!-- markdownlint-enable MD024 -->
<!-- markdownlint-enable MD025 -->
