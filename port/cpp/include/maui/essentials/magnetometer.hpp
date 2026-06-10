#pragma once
// maui::devices::sensors::magnetometer    <=  Microsoft.Maui.Devices.Sensors.Magnetometer (static facade)
// maui::devices::sensors::i_magnetometer  <=  Microsoft.Maui.Devices.Sensors.IMagnetometer
//
// Monitors the device's surrounding magnetic field (readings in microteslas).
// The lifecycle contract (shared partial, ported into the shared detail::basic_sensor base):
// start() throws feature_not_supported when unsupported and std::logic_error when already
// monitoring; stop() throws feature_not_supported when unsupported and is a no-op when not
// monitoring. ReadingChanged is event<magnetometer_data> (the C# args class collapses to its payload).
//
// Backends (suffix oracle): ios REAL (Magnetometer.ios.watchos.cs; the simulator reports the
// sensor unavailable and delivers no data, so on-simulator tests cover the lifecycle only).
// apple/macOS NOT SUPPORTED (Magnetometer.netstandard.tvos.macos.cs). Headless mirrors netstandard
// until faked - the fake is the readings test path.

#include <memory>

#include "maui/core/event.hpp"
#include "maui/essentials/sensor_types.hpp"

namespace maui::devices::sensors
{
    class i_magnetometer
    {
    public:
        virtual ~i_magnetometer() = default;

        // ReadingChanged (args collapse to the magnetometer_data payload).
        virtual maui::core::event<magnetometer_data>& reading_changed() = 0;

        [[nodiscard]] virtual bool is_supported() const = 0;
        [[nodiscard]] virtual bool is_monitoring() const = 0;

        virtual void start(sensor_speed speed) = 0;
        virtual void stop() = 0;

    protected:
        i_magnetometer() = default;
        i_magnetometer(const i_magnetometer&) = default;
        i_magnetometer(i_magnetometer&&) = default;
        i_magnetometer& operator=(const i_magnetometer&) = default;
        i_magnetometer& operator=(i_magnetometer&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (MagnetometerImplementation), one per backend under
        // src/platform/<backend>/essentials_magnetometer.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_magnetometer> make_magnetometer();
    } // namespace detail

    // The static facade over magnetometer::default_() (C# Magnetometer.Default).
    class magnetometer final
    {
    public:
        magnetometer() = delete;

        static maui::core::event<magnetometer_data>& reading_changed()
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

        // Magnetometer.Default (lazy platform default) + SetDefault (the test seam; nullptr resets).
        [[nodiscard]] static i_magnetometer& default_();
        static void set_default(std::shared_ptr<i_magnetometer> implementation);
    };
} // namespace maui::devices::sensors
