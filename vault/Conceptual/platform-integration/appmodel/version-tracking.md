---
title: "Version tracking"
description: "Learn how to use the .NET MAUI IVersionTracking interface, which lets you check the applications version and build numbers along with seeing additional information."
tags:
  - conceptual
  - area/platform-integration
ms_date: "02/02/2023"
source: "https://learn.microsoft.com/dotnet/maui/platform-integration/appmodel/version-tracking?view=net-maui-10.0"
---

# Version tracking

[![Browse sample.](~/media/code-sample.png) Browse the sample](/samples/dotnet/maui-samples/platformintegration-essentials)

This article describes how you can use the .NET Multi-platform App UI (.NET MAUI) [[IVersionTracking|IVersionTracking]] interface. This interface lets you check the applications version and build numbers along with seeing additional information such as if it's the first time the application launched.

The default implementation of the `IVersionTracking` interface is available through the [[VersionTracking.Default|VersionTracking.Default]] property. Both the `IVersionTracking` interface and `VersionTracking` class are contained in the `Microsoft.Maui.ApplicationModel` namespace.

## Get started

To enable version tracking in your app, invoke the `ConfigureEssentials%2A` method on the [[MauiAppBuilder|MauiAppBuilder]] object in the _MauiProgram.cs_ file. Then, on the [[IEssentialsBuilder|IEssentialsBuilder]] object, call the [[IEssentialsBuilder.UseVersionTracking|UseVersionTracking]] method:

:::code language="csharp" source="../snippets/shared_1/MauiProgram.cs" id="bootstrap_versiontracking" highlight="12-15":::

## Check the version

The [[IVersionTracking|IVersionTracking]] interface provides many properties that describe the current version of the app and how it relates to the previous version. The following example writes the tracking information to labels on the page:

:::code language="csharp" source="../snippets/shared_1/AppModelPage.xaml.cs" id="version_read":::

The first time the app is run after version tracking is enabled, the [[IVersionTracking.IsFirstLaunchEver|IsFirstLaunchEver]] property will return `true`. If you add version tracking in a newer version of an already released app, `IsFirstLaunchEver` may incorrectly report `true`. This property always returns `true` the first time version tracking is enabled and the user runs the app. You can't fully rely on this property if users have upgraded from older versions that weren't tracking the version.

## Platform differences

All version information is stored using the [[preferences|Preferences]] API, and is stored with a filename of _[YOUR-APP-PACKAGE-ID].microsoft.maui.essentials.versiontracking_ and follows the same data persistence outlined in the [[preferences#persistence|Preferences]] documentation.
