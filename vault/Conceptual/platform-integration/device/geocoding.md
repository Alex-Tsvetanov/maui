---
title: "Geocoding"
description: "Learn how to use the .NET MAUI IGeocoding interface in the Microsoft.Maui.Devices.Sensors namespace. This interface provides APIs to both geocode a placemark to a positional coordinate, and reverse geocode coordinates to a placemark."
tags:
  - conceptual
  - area/platform-integration
ms_date: "02/02/2023"
source: "https://learn.microsoft.com/dotnet/maui/platform-integration/device/geocoding?view=net-maui-10.0"
---

# Geocoding

[![Browse sample.](~/media/code-sample.png) Browse the sample](/samples/dotnet/maui-samples/platformintegration-essentials)

This article describes how you can use the .NET Multi-platform App UI (.NET MAUI) [[IGeocoding|IGeocoding]] interface. This interfaces provides APIs to geocode a placemark to a positional coordinates and reverse geocode coordinates to a placemark.

The default implementation of the `IGeocoding` interface is available through the [[Geocoding.Default|Geocoding.Default]] property. Both the `IGeocoding` interface and `Geocoding` class are contained in the `Microsoft.Maui.Devices.Sensors` namespace.

## Get started

To access the **Geocoding** functionality the following platform-specific setup is required.

<!-- markdownlint-disable MD025 -->
# [Android](#tab/android)

No setup is required.

# [iOS/Mac Catalyst](#tab/macios)

No setup is required.

# [Windows](#tab/windows)

A Bing Maps API key is required to use geocoding functionality. Sign up for a free [Bing Maps](https://www.bingmapsportal.com/) account. Under **My account** > **My keys**, create a new key and fill out information based on your application type, which should be **Windows Application**.

To enable geocoding functionality in your app, invoke the `ConfigureEssentials%2A` method on the [[MauiAppBuilder|MauiAppBuilder]] object in the _MauiProgram.cs_ file. Then, on the [[IEssentialsBuilder|IEssentialsBuilder]] object, call the `UseMapServiceToken%2A` method and pass your Bing Maps API key as the argument:

:::code language="csharp" source="../snippets/shared_1/MauiProgram.cs" id="bootstrap_maptoken" highlight="12-15":::

-----
<!-- markdownlint-enable MD025 -->

## Use geocoding

The following example demonstrates how to get the location coordinates for an address:

:::code language="csharp" source="../snippets/shared_1/SensorsPage.xaml.cs" id="geocoding_location":::

The altitude isn't always available. If it isn't available, the [[Location.Altitude|Altitude]] property might be `null`, or the value might be `0`. If the altitude is available, the value is in meters above sea level.

## Reverse geocoding

Reverse geocoding is the process of getting placemarks for an existing set of coordinates. The following example demonstrates getting placemarks:

:::code language="csharp" source="../snippets/shared_1/SensorsPage.xaml.cs" id="geocoding_reverse":::

## Get the distance between two locations

The [[Location|Location]] and [[LocationExtensions|LocationExtensions]] classes define methods to calculate the distance between two locations. For an example of getting the distance between two locations, see [[geolocation#distance-between-two-locations|Distance between two locations]].
