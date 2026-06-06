---
title: "SMS"
description: "Learn how to use the .NET MAUI ISms interface in Microsoft.Maui.ApplicationModel.Communication to open the default SMS application. The text message can be preloaded with a message and recipient."
tags:
  - conceptual
  - area/platform-integration
ms_date: "02/02/2023"
source: "https://learn.microsoft.com/dotnet/maui/platform-integration/communication/sms?view=net-maui-10.0"
---

# SMS

[![Browse sample.](~/media/code-sample.png) Browse the sample](/samples/dotnet/maui-samples/platformintegration-essentials)

This article describes how you can use the .NET Multi-platform App UI (.NET MAUI) [[ISms|ISms]] interface to open the default SMS app and preload it with a message and recipient.

The default implementation of the `ISms` interface is available through the [[Sms (Communication).Default|Sms.Default]] property. Both the `ISms` interface and `Sms` class are contained in the `Microsoft.Maui.ApplicationModel.Communication` namespace.

## Get started

To access the SMS functionality, the following platform specific setup is required.

<!-- markdownlint-disable MD025 -->
# [Android](#tab/android)

If your project's Target Android version is set to **Android 11 (R API 30)** or higher, you must update your _Android Manifest_ with queries that use Android's [package visibility requirements](https://developer.android.com/preview/privacy/package-visibility).

In the _Platforms/Android/AndroidManifest.xml_ file, add the following `queries/intent` nodes in the `manifest` node:

```xml
<queries>
  <intent>
    <action android:name="android.intent.action.VIEW" />
    <data android:scheme="smsto"/>
  </intent>
</queries>
```

# [iOS/Mac Catalyst](#tab/macios)

No setup is required.

# [Windows](#tab/windows)

No setup is required.

-----
<!-- markdownlint-enable MD025 -->

## Create a message

The SMS functionality works by creating a new [[SmsMessage|SmsMessage]] object, and calling the `ComposeAsync%2A` method. You can optionally include a message and zero or more recipients.

:::code language="csharp" source="../snippets/shared_1/CommsPage.xaml.cs" id="sms_send":::
