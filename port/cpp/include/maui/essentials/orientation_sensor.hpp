#pragma once
// maui::devices::sensors::orientation_sensor    <=  Microsoft.Maui.Devices.Sensors.OrientationSensor (static facade)
// maui::devices::sensors::i_orientation_sensor  <=  Microsoft.Maui.Devices.Sensors.IOrientationSensor
//
// Monitors the device orientation as a quaternion in MAUI's earth frame (Y north, Z vertical).
// The lifecycle contract (shared partial, ported into the shared detail::basic_sensor base):
// start() throws feature_not_supported when unsupported and std::logic_error when already
// monitoring; stop() throws feature_not_supported when unsupported and is a no-op when not
// monitoring. ReadingChanged is event<orientation_sensor_data> (the C# args class collapses to its payload).
//
// Backends (suffix oracle): ios REAL (OrientationSensor.ios.watchos.cs - CMDeviceMotion; the simulator reports the
// sensor unavailable and delivers no data, so on-simulator tests cover the lifecycle only).
// apple/macOS NOT SUPPORTED (OrientationSensor.netstandard.tvos.macos.cs). Headless mirrors netstandard
// until faked - the fake is the readings test path.

#include <memory>

#include "maui/core/event.hpp"
#include "maui/essentials/sensor_types.hpp"

namespace maui::devices::sensors
{
    class i_orientation_sensor
    {
    public:
        virtual ~i_orientation_sensor() = default;

        // ReadingChanged (args collapse to the orientation_sensor_data payload).
        virtual maui::core::event<orientation_sensor_data>& reading_changed() = 0;

        [[nodiscard]] virtual bool is_supported() const = 0;
        [[nodiscard]] virtual bool is_monitoring() const = 0;

        virtual void start(sensor_speed speed) = 0;
        virtual void stop() = 0;

    protected:
        i_orientation_sensor() = default;
        i_orientation_sensor(const i_orientation_sensor&) = default;
        i_orientation_sensor(i_orientation_sensor&&) = default;
        i_orientation_sensor& operator=(const i_orientation_sensor&) = default;
        i_orientation_sensor& operator=(i_orientation_sensor&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (OrientationSensorImplementation), one per backend under
        // src/platform/<backend>/essentials_orientation_sensor.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_orientation_sensor> make_orientation_sensor();
    } // namespace detail

    // The static facade over orientation_sensor::default_() (C# OrientationSensor.Default).
    class orientation_sensor final
    {
    public:
        orientation_sensor() = delete;

        static maui::core::event<orientation_sensor_data>& reading_changed()
        {
            return default_().reading_changed();
        }
        [[nodiscard]] static bool is_supported()
        {
            return default_().is_supported();
        }
        [[nodiscard]] static bool is_monitoring()
        {
            return default_().is_monitoring();
        }
        static void start(sensor_speed speed)
        {
            default_().start(speed);
        }
        static void stop()
        {
            default_().stop();
        }

        // OrientationSensor.Default (lazy platform default) + SetDefault (the test seam; nullptr resets).
        [[nodiscard]] static i_orientation_sensor& default_();
        static void set_default(std::shared_ptr<i_orientation_sensor> implementation);
    };
} // namespace maui::devices::sensors
