---
title: "Microsoft.Maui.Devices.Sensors"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Devices-Sensors
---

# Microsoft.Maui.Devices.Sensors

> [!info] Namespace
> `Microsoft.Maui.Devices.Sensors` — 47 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.devices.sensors)

## Overview

`Microsoft.Maui.Devices.Sensors` provides cross-platform access to a device's hardware sensors and location services from a single .NET MAUI codebase. It abstracts the per-platform sensor APIs of Android, iOS, macOS, and Windows behind a consistent set of interfaces and static facades, so an app can read motion, environmental, and positioning data without writing platform-specific code.

Motion and environmental sensing is exposed through paired interface/facade types: [[IAccelerometer|IAccelerometer]] / [[Accelerometer|Accelerometer]] for three-dimensional acceleration, [[IGyroscope|IGyroscope]] / [[Gyroscope|Gyroscope]] for rotation around the device's axes, [[ICompass|ICompass]] / [[Compass|Compass]] for heading, [[IOrientationSensor|IOrientationSensor]] / [[OrientationSensor|OrientationSensor]] for device orientation, [[IBarometer|IBarometer]] / [[Barometer|Barometer]] for atmospheric pressure, and the magnetometer types. Each sensor follows the same pattern: start and stop monitoring, subscribe to a `ReadingChanged` event, and receive a lightweight reading struct such as [[AccelerometerData|AccelerometerData]], [[GyroscopeData|GyroscopeData]], or [[CompassData|CompassData]]. The [[SensorSpeed|SensorSpeed]] enum controls how frequently updates are delivered, trading responsiveness against power consumption.

Location and geography are covered by [[IGeolocation|IGeolocation]] / [[Geolocation|Geolocation]], which return the device's current or last known [[Location|Location]] and support continuous listening, and by [[IGeocoding|IGeocoding]] / [[Geocoding|Geocoding]], which convert between coordinates and a [[Placemark|Placemark]] (forward and reverse geocoding). Request and result options are configured through types like [[GeolocationRequest|GeolocationRequest]] and [[GeolocationAccuracy|GeolocationAccuracy]], while event-argument and extension types round out the asynchronous, event-driven model used across the namespace.

## Key types

- [[IGeolocation|IGeolocation]] — Provides a way to get the current location of the device.
- [[Geolocation|Geolocation]] — Static facade that returns the last known location of the device and supports location listening.
- [[Location|Location]] — Represents a geographic coordinate with altitude and reference-system information.
- [[IGeocoding|IGeocoding]] — Geocodes a placemark to coordinates and reverse-geocodes coordinates to a placemark.
- [[Placemark|Placemark]] — User-friendly description of a geographic coordinate, including name and address details.
- [[IAccelerometer|IAccelerometer]] — Reports the acceleration of the device in three-dimensional space.
- [[IGyroscope|IGyroscope]] — Monitors rotation around the device's three primary axes.
- [[ICompass|ICompass]] — Monitors changes to the heading/orientation of the user's device.
- [[IOrientationSensor|IOrientationSensor]] — Monitors the orientation of a device in three-dimensional space.
- [[IBarometer|IBarometer]] — Monitors changes to the atmospheric pressure.
- [[SensorSpeed|SensorSpeed]] — Controls how frequently sensor changes are monitored and reported.
- [[GeolocationRequest|GeolocationRequest]] — Configures desired accuracy and timeout when requesting a location.


## Classes

| Type | Summary |
|---|---|
| [[Accelerometer\|Accelerometer]] | Occurs when the sensor reading changes. |
| [[AccelerometerChangedEventArgs\|AccelerometerChangedEventArgs]] | Event arguments containing the current reading of `IAccelerometer`. |
| [[Barometer\|Barometer]] | Gets a value indicating whether reading the barometer is supported on this device. |
| [[BarometerChangedEventArgs\|BarometerChangedEventArgs]] | Contains the current pressure information from the `ReadingChanged` event. |
| [[Compass\|Compass]] | Gets or sets if heading calibration should be shown. |
| [[CompassChangedEventArgs\|CompassChangedEventArgs]] | Event arguments when compass reading changes. |
| [[CompassExtensions\|CompassExtensions]] | This class contains static extension methods for use with `ICompass`. |
| [[Geocoding\|Geocoding]] | Gets or sets the map service API key for this platform. |
| [[GeocodingExtensions\|GeocodingExtensions]] | Static class with extension methods for the `IGeocoding` APIs. |
| [[Geolocation\|Geolocation]] | Returns the last known location of the device. |
| [[GeolocationExtensions\|GeolocationExtensions]] | Static class with extension methods for the `IGeolocation` APIs. |
| [[GeolocationListeningFailedEventArgs\|GeolocationListeningFailedEventArgs]] | Event args for the geolocation listening error event. |
| [[GeolocationListeningRequest\|GeolocationListeningRequest]] | Request options for listening to geolocations |
| [[GeolocationLocationChangedEventArgs\|GeolocationLocationChangedEventArgs]] | Event arguments containing the current reading of `LocationChanged`. |
| [[GeolocationRequest\|GeolocationRequest]] | Represents the best accuracy, using the most power to obtain and typically within 10 meters. |
| [[Gyroscope\|Gyroscope]] | Gets a value indicating whether reading the gyroscope is supported on this device. |
| [[GyroscopeChangedEventArgs\|GyroscopeChangedEventArgs]] | Contains the current axis reading information from the `ReadingChanged` event. |
| [[Location\|Location]] | The altitude reference system was not specified. |
| [[LocationExtensions\|LocationExtensions]] | This class contains static extension methods for use with `Location` objects. |
| [[Magnetometer\|Magnetometer]] |  |
| [[MagnetometerChangedEventArgs\|MagnetometerChangedEventArgs]] |  |
| [[MagnetometerData\|MagnetometerData]] |  |
| [[OrientationSensor\|OrientationSensor]] | Gets a value indicating whether reading the orientation sensor is supported on this device. |
| [[OrientationSensorChangedEventArgs\|OrientationSensorChangedEventArgs]] | Contains the current orientation sensor information from the `ReadingChanged` event. |
| [[OrientationSensorImplementation\|OrientationSensorImplementation]] | Concrete implementation of the `IOrientationSensor` APIs. |
| [[Placemark\|Placemark]] | Represents a user-friendly description of a geographic coordinate. This contains information such as the name of the place, its address, and other information. |
| [[PlacemarkExtensions\|PlacemarkExtensions]] | This class contains static extension methods for use with `Placemark` objects. |

## Interfaces

| Type | Summary |
|---|---|
| [[IAccelerometer\|IAccelerometer]] | Accelerometer data of the acceleration of the device in three-dimensional space. |
| [[IBarometer\|IBarometer]] | Monitor changes to the atmospheric pressure. |
| [[ICompass\|ICompass]] | Monitor changes to the orientation of the user's device. |
| [[IGeocoding\|IGeocoding]] | The Geocoding API provides functionality to geocode a placemark to positional coordinates and reverse-geocode coordinates to a placemark. |
| [[IGeolocation\|IGeolocation]] | Provides a way to get the current location of the device. |
| [[IGyroscope\|IGyroscope]] | The Gyroscope API lets you monitor the device's gyroscope sensor which is the rotation around the device's three primary axes. |
| [[IMagnetometer\|IMagnetometer]] |  |
| [[IOrientationSensor\|IOrientationSensor]] | The OrientationSensor API lets you monitor the orientation of a device in three dimensional space. |
| [[IPlatformCompass\|IPlatformCompass]] | Gets a value indicating whether reading the compass is supported on this device. |
| [[IPlatformGeocoding\|IPlatformGeocoding]] | Retrieve potential placemarks for a given location specified by coordinates. |

## Structs

| Type | Summary |
|---|---|
| [[AccelerometerData\|AccelerometerData]] | Data representing the devices' three accelerometers. |
| [[BarometerData\|BarometerData]] | Contains the pressure measured by the user's device barometer. |
| [[CompassData\|CompassData]] | Contains the orientation of the user's device. |
| [[GyroscopeData\|GyroscopeData]] | Contains the axis readings measured by the device's gyroscope. |
| [[OrientationSensorData\|OrientationSensorData]] | Contains the orientation measured by the user's device orientation sensor. |

## Enums

| Type | Summary |
|---|---|
| [[AltitudeReferenceSystem\|AltitudeReferenceSystem]] | Kilometers. |
| [[DistanceUnits\|DistanceUnits]] | Distance unit for use in conversion. |
| [[GeolocationAccuracy\|GeolocationAccuracy]] | Represents levels of accuracy when determining the device location. |
| [[GeolocationError\|GeolocationError]] | Error values for listening for geolocation changes |
| [[SensorSpeed\|SensorSpeed]] | Represents the sensor speed to monitor device sensors for changes. |

## See also

- [[_API Reference]]
