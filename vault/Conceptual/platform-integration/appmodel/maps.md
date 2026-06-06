---
title: "Open the map app"
description: "Learn how to use the .NET MAUI IMap interface in the Microsoft.Maui.ApplicationModel namespace. This interface enables an application to open the installed map application to a specific location or place mark."
tags:
  - conceptual
  - area/platform-integration
ms_date: "02/02/2023"
source: "https://learn.microsoft.com/dotnet/maui/platform-integration/appmodel/maps?view=net-maui-10.0"
---

# Open the map app

[![Browse sample.](~/media/code-sample.png) Browse the sample](/samples/dotnet/maui-samples/platformintegration-essentials)

This article describes how you can use the .NET Multi-platform App UI (.NET MAUI) [[IMap (ApplicationModel)|IMap]] interface. This interface enables an application to open the installed map application to a specific location or place mark.

The default implementation of the `IMap` interface is available through the [[Map (ApplicationModel).Default|Map.Default]] property. Both the `IMap` interface and `Map` class are contained in the `Microsoft.Maui.ApplicationModel` namespace.

## Get started

To access the browser functionality, the following platform-specific setup is required.

<!-- markdownlint-disable MD025 -->
# [Android](#tab/android)

Android uses the `geo:` URI scheme to launch the maps application on the device. This may prompt the user to select from an existing app that supports this URI scheme. Google Maps supports this scheme.

In the _Platforms/Android/AndroidManifest.xml_ file, add the following `queries/intent` nodes to the `manifest` node:

```xml
<queries>
  <intent>
    <action android:name="android.intent.action.VIEW" />
    <data android:scheme="geo"/>
  </intent>
</queries>
```

# [iOS/Mac Catalyst](#tab/macios)

No setup is required.

# [Windows](#tab/windows)

No setup is required.

-----
<!-- markdownlint-enable MD025 -->

## Using the map

The map functionality works by calling the `IMap.OpenAsync%2A` method, and passing either an instance of the [[Location|Location]] or [[Placemark|Placemark]] type. The following example opens the installed map app at a specific GPS location:

:::code language="csharp" source="../snippets/shared_1/AppModelPage.xaml.cs" id="navigate_building":::

> [!TIP]
> The `Location` and `Placemark` types are in the `Microsoft.Maui.Devices.Sensors` namespace.

When you use a `Placemark` to open the map, more information is required. The information helps the map app search for the place you're looking for. The following information is required:

- [[Placemark.CountryName|CountryName]]
- [[Placemark.AdminArea|AdminArea]]
- [[Placemark.Thoroughfare|Thoroughfare]]
- [[Placemark.Locality|Locality]]

:::code language="csharp" source="../snippets/shared_1/AppModelPage.xaml.cs" id="navigate_building_placemark":::

### Testing if the map opened

There's always the possibility that opening the map app failed, such as when there isn't a map app or your app doesn't have the correct permissions. For each `IMap.OpenAsync%2A` method overload, there's a corresponding `IMap.TryOpenAsync%2A` method, which returns a Boolean value indicating that the map app was successfully opened. The following code example uses the `TryOpenAsync` method to open the map:

:::code language="csharp" source="../snippets/shared_1/AppModelPage.xaml.cs" id="navigate_tryopen":::

## Extension methods

As long as the `Microsoft.Maui.Devices.Sensors` namespace is imported, which a new .NET MAUI project automatically does, you can use the built-in extension method `OpenMapsAsync` to open the map:

:::code language="csharp" source="../snippets/shared_1/AppModelPage.xaml.cs" id="navigate_building_placemark_extension":::

## Add navigation

When you open the map, you can calculate a route from the device's current location to the specified location. Pass the [[MapLaunchOptions|MapLaunchOptions]] type to the `Map.OpenAsync` method, specifying the navigation mode. The following example opens the map app and specifies a driving navigation mode:

:::code language="csharp" source="../snippets/shared_1/AppModelPage.xaml.cs" id="navigate_building_driving":::

## Platform differences

This section describes the platform-specific differences with the maps API.

<!-- markdownlint-disable MD025 -->
# [Android](#tab/android)

`NavigationMode` supports Bicycling, Driving, and Walking.

# [iOS/Mac Catalyst](#tab/macios)

`NavigationMode` supports Driving, Transit, and Walking.

# [Windows](#tab/windows)

`NavigationMode` supports Driving, Transit, and Walking.

-----
<!-- markdownlint-enable MD025 -->
