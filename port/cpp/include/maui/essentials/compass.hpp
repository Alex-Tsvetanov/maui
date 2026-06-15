#pragma once
// maui::devices::sensors::compass    <=  Microsoft.Maui.Devices.Sensors.Compass (static facade)
// maui::devices::sensors::i_compass  <=  Microsoft.Maui.Devices.Sensors.ICompass
//
// Monitors the device's magnetic-north heading (degrees). The lifecycle contract (shared partial,
// ported into detail::compass_base): start() throws feature_not_supported when unsupported and
// std::logic_error("Compass has already been started.") when already monitoring; stop() throws
// feature_not_supported when unsupported and is a no-op when not monitoring. ReadingChanged is
// event<compass_data>. start(speed) forwards to start(speed, /*apply_low_pass_filter*/ true),
// exactly like the C# overload; the filter flag is only consumed by the Android partial.
//
// IPlatformCompass.ShouldDisplayHeadingCalibration (Compass.shared.cs / Compass.ios.cs, an iOS-only
// platform extra) is ported as should_display_heading_calibration() get/set on i_compass: a bool
// that defaults to false and, on iOS, feeds CLLocationManager's
// -locationManagerShouldDisplayHeadingCalibration: delegate (whether the OS shows the calibration
// overlay). The base returns false / ignores the setter (matching the C# facade's "false unless
// Current is IPlatformCompass"); only the iOS partial overrides. macOS/headless keep the default.
//
// Backends (suffix oracle): ios REAL (Compass.ios.cs - CLLocationManager heading updates; the
// simulator reports heading unavailable and delivers no data, so on-simulator tests cover the
// lifecycle only). apple/macOS NOT SUPPORTED (Compass.netstandard.tvos.watchos.macos.cs).
// Headless mirrors netstandard until faked - the fake is the readings test path.

#include <memory>

#include "maui/core/event.hpp"
#include "maui/essentials/sensor_types.hpp"

namespace maui::devices::sensors
{
    class i_compass
    {
    public:
        virtual ~i_compass() = default;

        // ReadingChanged (args collapse to the compass_data payload).
        virtual maui::core::event<compass_data>& reading_changed() = 0;

        [[nodiscard]] virtual bool is_supported() const = 0;
        [[nodiscard]] virtual bool is_monitoring() const = 0;

        // ICompass.Start(sensorSpeed) - applies the low-pass filter (the C# overload's behavior).
        virtual void start(sensor_speed speed) = 0;
        // ICompass.Start(sensorSpeed, applyLowPassFilter) - the filter only matters on Android.
        virtual void start(sensor_speed speed, bool apply_low_pass_filter) = 0;
        virtual void stop() = 0;

        // IPlatformCompass.ShouldDisplayHeadingCalibration (iOS-only). The base mirrors the C#
        // facade for a non-IPlatformCompass Current: the getter returns false and the setter is a
        // no-op. Only the iOS partial overrides to track the value and feed CLLocationManager's
        // calibration-overlay delegate.
        [[nodiscard]] virtual bool should_display_heading_calibration() const
        {
            return false;
        }
        virtual void set_should_display_heading_calibration(bool /*value*/)
        {
        }

    protected:
        i_compass() = default;
        i_compass(const i_compass&) = default;
        i_compass(i_compass&&) = default;
        i_compass& operator=(const i_compass&) = default;
        i_compass& operator=(i_compass&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (CompassImplementation), one per backend under
        // src/platform/<backend>/essentials_compass.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_compass> make_compass();
    } // namespace detail

    // The static facade over compass::default_() (C# Compass.Default).
    class compass final
    {
    public:
        compass() = delete;

        static maui::core::event<compass_data>& reading_changed()
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
        static void start(sensor_speed speed, bool apply_low_pass_filter)
        {
            default_().start(speed, apply_low_pass_filter);
        }
        static void stop()
        {
            default_().stop();
        }

        // Compass.ShouldDisplayHeadingCalibration (Compass.shared.cs, iOS/MacCatalyst-gated). The
        // C# getter returns false unless Current is IPlatformCompass; here the i_compass base
        // default already returns false for the non-iOS impls, so the facade simply delegates.
        [[nodiscard]] static bool should_display_heading_calibration()
        {
            return default_().should_display_heading_calibration();
        }
        static void set_should_display_heading_calibration(bool value)
        {
            default_().set_should_display_heading_calibration(value);
        }

        // Compass.Default (lazy platform default) + SetDefault (the test seam; nullptr resets).
        [[nodiscard]] static i_compass& default_();
        static void set_default(std::shared_ptr<i_compass> implementation);
    };
} // namespace maui::devices::sensors
