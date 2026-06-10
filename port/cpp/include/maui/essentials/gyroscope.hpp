#pragma once
// maui::devices::sensors::gyroscope    <=  Microsoft.Maui.Devices.Sensors.Gyroscope (static facade)
// maui::devices::sensors::i_gyroscope  <=  Microsoft.Maui.Devices.Sensors.IGyroscope
//
// Monitors rotation around the device's three primary axes (readings in rad/s).
// The lifecycle contract (shared partial, ported into the shared detail::basic_sensor base):
// start() throws feature_not_supported when unsupported and std::logic_error when already
// monitoring; stop() throws feature_not_supported when unsupported and is a no-op when not
// monitoring. ReadingChanged is event<gyroscope_data> (the C# args class collapses to its payload).
//
// Backends (suffix oracle): ios REAL (Gyroscope.ios.watchos.cs; the simulator reports the
// sensor unavailable and delivers no data, so on-simulator tests cover the lifecycle only).
// apple/macOS NOT SUPPORTED (Gyroscope.netstandard.tvos.macos.cs). Headless mirrors netstandard
// until faked - the fake is the readings test path.

#include <memory>

#include "maui/core/event.hpp"
#include "maui/essentials/sensor_types.hpp"

namespace maui::devices::sensors
{
    class i_gyroscope
    {
    public:
        virtual ~i_gyroscope() = default;

        // ReadingChanged (args collapse to the gyroscope_data payload).
        virtual maui::core::event<gyroscope_data>& reading_changed() = 0;

        [[nodiscard]] virtual bool is_supported() const = 0;
        [[nodiscard]] virtual bool is_monitoring() const = 0;

        virtual void start(sensor_speed speed) = 0;
        virtual void stop() = 0;

    protected:
        i_gyroscope() = default;
        i_gyroscope(const i_gyroscope&) = default;
        i_gyroscope(i_gyroscope&&) = default;
        i_gyroscope& operator=(const i_gyroscope&) = default;
        i_gyroscope& operator=(i_gyroscope&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (GyroscopeImplementation), one per backend under
        // src/platform/<backend>/essentials_gyroscope.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_gyroscope> make_gyroscope();
    } // namespace detail

    // The static facade over gyroscope::default_() (C# Gyroscope.Default).
    class gyroscope final
    {
    public:
        gyroscope() = delete;

        static maui::core::event<gyroscope_data>& reading_changed()
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

        // Gyroscope.Default (lazy platform default) + SetDefault (the test seam; nullptr resets).
        [[nodiscard]] static i_gyroscope& default_();
        static void set_default(std::shared_ptr<i_gyroscope> implementation);
    };
} // namespace maui::devices::sensors
