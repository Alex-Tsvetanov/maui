#pragma once
// maui::devices::sensors::accelerometer    <=  Microsoft.Maui.Devices.Sensors.Accelerometer (static facade)
// maui::devices::sensors::i_accelerometer  <=  Microsoft.Maui.Devices.Sensors.IAccelerometer
//
// Monitors device acceleration in three-dimensional space (readings in G's). The lifecycle
// contract (shared partial, ported into detail::accelerometer_base): start() throws
// feature_not_supported when unsupported and std::logic_error("Accelerometer has already been
// started.") when already monitoring; stop() throws feature_not_supported when unsupported and is
// a no-op when not monitoring. The reading event is event<accelerometer_data>
// (AccelerometerChangedEventArgs collapses to its payload); ShakeDetected is event<> and fires
// when 3/4 of the readings in the last half second exceed the acceleration threshold (the
// AccelerometerQueue algorithm, ported 1:1 in src/essentials/detail/accelerometer_queue.hpp).
//
// Backends (suffix oracle): ios REAL (Accelerometer.ios.watchos.cs - CMMotionManager; the
// simulator reports it unavailable and delivers no data, so on-simulator tests cover the lifecycle
// only). apple/macOS NOT SUPPORTED (Accelerometer.netstandard.tvos.macos.cs). Headless mirrors
// netstandard until faked - the fake is the readings test path.

#include <memory>

#include "maui/core/event.hpp"
#include "maui/essentials/sensor_types.hpp"

namespace maui::devices::sensors
{
    class i_accelerometer
    {
    public:
        virtual ~i_accelerometer() = default;

        // ReadingChanged (args collapse to the accelerometer_data payload).
        virtual maui::core::event<accelerometer_data>& reading_changed() = 0;
        // ShakeDetected.
        virtual maui::core::event<>& shake_detected() = 0;

        [[nodiscard]] virtual bool is_supported() const = 0;
        [[nodiscard]] virtual bool is_monitoring() const = 0;

        virtual void start(sensor_speed speed) = 0;
        virtual void stop() = 0;

    protected:
        i_accelerometer() = default;
        i_accelerometer(const i_accelerometer&) = default;
        i_accelerometer(i_accelerometer&&) = default;
        i_accelerometer& operator=(const i_accelerometer&) = default;
        i_accelerometer& operator=(i_accelerometer&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (AccelerometerImplementation), one per backend under
        // src/platform/<backend>/essentials_accelerometer.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_accelerometer> make_accelerometer();
    } // namespace detail

    // The static facade over accelerometer::default_() (C# Accelerometer.Default).
    class accelerometer final
    {
    public:
        accelerometer() = delete;

        static maui::core::event<accelerometer_data>& reading_changed()
        {
            return default_().reading_changed();
        }
        static maui::core::event<>& shake_detected()
        {
            return default_().shake_detected();
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

        // Accelerometer.Default (lazy platform default) + SetDefault (the test seam; nullptr resets).
        [[nodiscard]] static i_accelerometer& default_();
        static void set_default(std::shared_ptr<i_accelerometer> implementation);
    };
} // namespace maui::devices::sensors
