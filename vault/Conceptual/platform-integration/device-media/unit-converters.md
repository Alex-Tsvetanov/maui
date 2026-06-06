---
title: "Unit converters"
description: "Learn how to use the .NET MAUI UnitConverters class, which provides several unit converters to help developers."
tags:
  - conceptual
  - area/platform-integration
ms_date: "02/02/2023"
source: "https://learn.microsoft.com/dotnet/maui/platform-integration/device-media/unit-converters?view=net-maui-10.0"
---

# Unit converters

[![Browse sample.](~/media/code-sample.png) Browse the sample](/samples/dotnet/maui-samples/platformintegration-essentials)

This article describes how you can use the .NET Multi-platform App UI (.NET MAUI) [[UnitConverters|UnitConverters]] class. This class provides several unit converters to help developers convert from one unit of measurement to another.

## Using unit converters

All unit converters are available by using the static `Microsoft.Maui.Media.UnitConverters` class. For example, you can convert Fahrenheit to Celsius with the `FahrenheitToCelsius%2A` method:

```csharp
var celsius = UnitConverters.FahrenheitToCelsius(32.0);
```

Here is a list of available conversions:

- `FahrenheitToCelsius%2A`
- `CelsiusToFahrenheit%2A`
- `CelsiusToKelvin%2A`
- `KelvinToCelsius%2A`
- `MilesToMeters%2A`
- `MilesToKilometers%2A`
- `KilometersToMiles%2A`
- `MetersToInternationalFeet%2A`
- `InternationalFeetToMeters%2A`
- `DegreesToRadians%2A`
- `RadiansToDegrees%2A`
- `DegreesPerSecondToRadiansPerSecond%2A`
- `RadiansPerSecondToDegreesPerSecond%2A`
- `DegreesPerSecondToHertz%2A`
- `RadiansPerSecondToHertz%2A`
- `HertzToDegreesPerSecond%2A`
- `HertzToRadiansPerSecond%2A`
- `KilopascalsToHectopascals%2A`
- `HectopascalsToKilopascals%2A`
- `KilopascalsToPascals%2A`
- `HectopascalsToPascals%2A`
- `AtmospheresToPascals%2A`
- `PascalsToAtmospheres%2A`
- `CoordinatesToMiles%2A`
- `CoordinatesToKilometers%2A`
- `KilogramsToPounds%2A`
- `PoundsToKilograms%2A`
- `StonesToPounds%2A`
- `PoundsToStones%2A`
