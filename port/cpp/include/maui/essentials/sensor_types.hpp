#pragma once
// maui::devices::sensors sensor-reading value types  <=  Microsoft.Maui.Devices.Sensors.*
//
//   sensor_speed             <=  Microsoft.Maui.Devices.Sensors.SensorSpeed (Types/SensorSpeed.shared.cs)
//   accelerometer_data       <=  Microsoft.Maui.Devices.Sensors.AccelerometerData
//   gyroscope_data           <=  Microsoft.Maui.Devices.Sensors.GyroscopeData
//   magnetometer_data        <=  Microsoft.Maui.Devices.Sensors.MagnetometerData
//   compass_data             <=  Microsoft.Maui.Devices.Sensors.CompassData
//   barometer_data           <=  Microsoft.Maui.Devices.Sensors.BarometerData
//   orientation_sensor_data  <=  Microsoft.Maui.Devices.Sensors.OrientationSensorData
//
// One header for the whole cluster (PROFILE.md §3's tightly-coupled-cluster allowance): these are
// the tiny readonly reading structs every sensor event carries, ported together from the C# shared
// files. Equality semantics mirror C# exactly (each compares only its payload). The C#
// XxxChangedEventArgs wrappers collapse to the payload itself: a reading event in the port is
// maui::core::event<xxx_data> (args are delivered by const ref; the wrapper class added nothing).
// GetHashCode has no C++ consumer in the port and is not mirrored.

#include <chrono>

#include "maui/graphics/quaternion.hpp"
#include "maui/graphics/vector3.hpp"

namespace maui::devices::sensors
{
    // Speed to monitor a device sensor for changes. Members mirror C# (Default is a C++ keyword,
    // hence the trailing underscore - the port-wide convention).
    enum class sensor_speed
    {
        default_ = 0, // C# SensorSpeed.Default ("default" is a C++ keyword)
        ui = 1,
        game = 2,
        fastest = 3,
    };

    // SensorSpeedExtensions (Types/SensorSpeed.shared.cs): the timing intervals, matching the
    // Android sensor speeds, that the platform partials use to configure update rates.
    // ToPlatform (Types/SensorSpeed.ios.tvos.watchos.cs) = interval(speed) in seconds.
    [[nodiscard]] constexpr std::chrono::milliseconds sensor_interval(sensor_speed speed)
    {
        switch (speed)
        {
            case sensor_speed::fastest:
                return std::chrono::milliseconds{5};
            case sensor_speed::game:
                return std::chrono::milliseconds{20};
            case sensor_speed::ui:
                return std::chrono::milliseconds{60};
            case sensor_speed::default_:
                break;
        }
        return std::chrono::milliseconds{200};
    }

    // The acceleration vector in G's. C#'s (double, double, double) ctor narrows to float; callers
    // pass doubles and convert implicitly here.
    struct accelerometer_data
    {
        maui::graphics::vector3 acceleration; // AccelerometerData.Acceleration

        constexpr accelerometer_data() = default;
        constexpr accelerometer_data(float x, float y, float z) : acceleration(x, y, z)
        {
        }
    };
    constexpr bool operator==(const accelerometer_data& a, const accelerometer_data& b)
    {
        return a.acceleration == b.acceleration;
    }

    // The angular velocity vector in radians per second.
    struct gyroscope_data
    {
        maui::graphics::vector3 angular_velocity; // GyroscopeData.AngularVelocity

        constexpr gyroscope_data() = default;
        constexpr gyroscope_data(float x, float y, float z) : angular_velocity(x, y, z)
        {
        }
    };
    constexpr bool operator==(const gyroscope_data& a, const gyroscope_data& b)
    {
        return a.angular_velocity == b.angular_velocity;
    }

    // The magnetic field vector in microteslas.
    struct magnetometer_data
    {
        maui::graphics::vector3 magnetic_field; // MagnetometerData.MagneticField

        constexpr magnetometer_data() = default;
        constexpr magnetometer_data(float x, float y, float z) : magnetic_field(x, y, z)
        {
        }
    };
    constexpr bool operator==(const magnetometer_data& a, const magnetometer_data& b)
    {
        return a.magnetic_field == b.magnetic_field;
    }

    // The heading (degrees relative to magnetic north).
    struct compass_data
    {
        double heading_magnetic_north = 0; // CompassData.HeadingMagneticNorth

        constexpr compass_data() = default;
        constexpr explicit compass_data(double heading) : heading_magnetic_north(heading)
        {
        }
    };
    constexpr bool operator==(const compass_data& a, const compass_data& b)
    {
        return a.heading_magnetic_north == b.heading_magnetic_north;
    }

    // The atmospheric pressure in hectopascals.
    struct barometer_data
    {
        double pressure_in_hectopascals = 0; // BarometerData.PressureInHectopascals

        constexpr barometer_data() = default;
        constexpr explicit barometer_data(double pressure) : pressure_in_hectopascals(pressure)
        {
        }
    };
    constexpr bool operator==(const barometer_data& a, const barometer_data& b)
    {
        return a.pressure_in_hectopascals == b.pressure_in_hectopascals;
    }

    // The device orientation as a quaternion (MAUI's earth frame: Y north, Z vertical).
    struct orientation_sensor_data
    {
        maui::graphics::quaternion orientation; // OrientationSensorData.Orientation

        constexpr orientation_sensor_data() : orientation(0, 0, 0, 0)
        {
            // C#'s default OrientationSensorData is all-zero (default Quaternion struct), not identity.
        }
        constexpr orientation_sensor_data(float x, float y, float z, float w) : orientation(x, y, z, w)
        {
        }
    };
    constexpr bool operator==(const orientation_sensor_data& a, const orientation_sensor_data& b)
    {
        return a.orientation == b.orientation;
    }
} // namespace maui::devices::sensors
