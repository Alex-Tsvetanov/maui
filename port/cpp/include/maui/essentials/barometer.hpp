#pragma once
// maui::devices::sensors::barometer    <=  Microsoft.Maui.Devices.Sensors.Barometer (static facade)
// maui::devices::sensors::i_barometer  <=  Microsoft.Maui.Devices.Sensors.IBarometer
//
// Monitors atmospheric pressure (readings in hectopascals).
// The lifecycle contract (shared partial, ported into the shared detail::basic_sensor base):
// start() throws feature_not_supported when unsupported and std::logic_error when already
// monitoring; stop() throws feature_not_supported when unsupported and is a no-op when not
// monitoring. ReadingChanged is event<barometer_data> (the C# args class collapses to its payload).
//
// Backends (suffix oracle): ios REAL (Barometer.ios.watchos.cs - CMAltimeter; the simulator reports the
// sensor unavailable and delivers no data, so on-simulator tests cover the lifecycle only).
// apple/macOS NOT SUPPORTED (Barometer.netstandard.tvos.macos.cs). Headless mirrors netstandard
// until faked - the fake is the readings test path.

#include <memory>

#include "maui/core/event.hpp"
#include "maui/essentials/sensor_types.hpp"

namespace maui::devices::sensors
{
    class i_barometer
    {
    public:
        virtual ~i_barometer() = default;

        // ReadingChanged (args collapse to the barometer_data payload).
        virtual maui::core::event<barometer_data>& reading_changed() = 0;

        [[nodiscard]] virtual bool is_supported() const = 0;
        [[nodiscard]] virtual bool is_monitoring() const = 0;

        virtual void start(sensor_speed speed) = 0;
        virtual void stop() = 0;

    protected:
        i_barometer() = default;
        i_barometer(const i_barometer&) = default;
        i_barometer(i_barometer&&) = default;
        i_barometer& operator=(const i_barometer&) = default;
        i_barometer& operator=(i_barometer&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (BarometerImplementation), one per backend under
        // src/platform/<backend>/essentials_barometer.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_barometer> make_barometer();
    } // namespace detail

    // The static facade over barometer::default_() (C# Barometer.Default).
    class barometer final
    {
    public:
        barometer() = delete;

        static maui::core::event<barometer_data>& reading_changed()
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

        // Barometer.Default (lazy platform default) + SetDefault (the test seam; nullptr resets).
        [[nodiscard]] static i_barometer& default_();
        static void set_default(std::shared_ptr<i_barometer> implementation);
    };
} // namespace maui::devices::sensors
