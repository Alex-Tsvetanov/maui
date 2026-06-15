// The essentials suite against the REAL iOS partials, run ON the simulator (via
// tools/ios-sim-run.sh): true UIDevice/sysctl device_info reads, UIScreen display metrics, the
// UIDevice battery surface (the SIMULATOR reports level -1 / state unknown), the actuator support
// matrix (no AVCaptureDevice on the simulator -> flashlight unsupported; vibration + haptics are
// supported no-ops), and the sensor LIFECYCLE contract - the simulator reports the CoreMotion /
// heading / altimeter sensors unavailable and delivers NO data, so the tests branch: supported ->
// full start/double-start/stop lifecycle, unsupported -> the feature_not_supported gates. Readings
// are covered by the headless fakes.

#import <UIKit/UIKit.h>

#include <chrono>
#include <stdexcept>

#include <gtest/gtest.h>

#include "maui/essentials/accelerometer.hpp"
#include "maui/essentials/barometer.hpp"
#include "maui/essentials/battery.hpp"
#include "maui/essentials/compass.hpp"
#include "maui/essentials/device_display.hpp"
#include "maui/essentials/device_info.hpp"
#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/flashlight.hpp"
#include "maui/essentials/geocoding.hpp"
#include "maui/essentials/geolocation.hpp"
#include "maui/essentials/gyroscope.hpp"
#include "maui/essentials/haptic_feedback.hpp"
#include "maui/essentials/magnetometer.hpp"
#include "maui/essentials/orientation_sensor.hpp"
#include "maui/essentials/sensor_types.hpp"
#include "maui/essentials/vibration.hpp"

namespace
{
    using namespace maui::devices;
    using namespace maui::devices::sensors;
    using maui::application_model::feature_not_supported;

    TEST(essentials_ios, device_info_reads_real_values)
    {
        EXPECT_EQ(device_info::manufacturer(), "Apple");
        EXPECT_TRUE(device_info::platform() == device_platform::ios());
        EXPECT_TRUE(device_info::idiom() == device_idiom::phone() || device_info::idiom() == device_idiom::tablet());
        EXPECT_EQ(device_info::device_type(), device_type::virtual_); // always virtual on the simulator

        EXPECT_FALSE(device_info::model().empty());          // "arm64" on the simulator (hw.machine)
        EXPECT_FALSE(device_info::name().empty());           // the simulator device's name
        EXPECT_FALSE(device_info::version_string().empty()); // UIDevice.systemVersion
        EXPECT_GT(device_info::version().major, 0);
    }

    TEST(essentials_ios, device_display_reads_real_metrics)
    {
        const display_info info = device_display::main_display_info();
        EXPECT_GT(info.width, 0);
        EXPECT_GT(info.height, 0);
        EXPECT_GT(info.density, 0);
        EXPECT_GE(info.refresh_rate, 0); // maximumFramesPerSecond
        EXPECT_NE(info.orientation, display_orientation::unknown);
    }

    TEST(essentials_ios, device_display_keep_screen_on_is_callable)
    {
        // The spawned test process has no UIApplication instance; the setter must no-op and the
        // getter report false (with an app it round-trips idleTimerDisabled).
        EXPECT_NO_THROW(device_display::set_keep_screen_on(true));
        if ([UIApplication sharedApplication] != nil)
        {
            EXPECT_TRUE(device_display::keep_screen_on());
        }
        EXPECT_NO_THROW(device_display::set_keep_screen_on(false));
        EXPECT_FALSE(device_display::keep_screen_on());
    }

    TEST(essentials_ios, device_display_listener_lifecycle)
    {
        const auto token = device_display::add_main_display_info_changed([](const display_info&) {});
        EXPECT_TRUE(device_display::remove_main_display_info_changed(token));
        EXPECT_FALSE(device_display::remove_main_display_info_changed(token));
    }

    TEST(essentials_ios, battery_reads_real_values)
    {
        // The simulator reports batteryLevel -1 (unknown); hardware reports 0..1.
        const double level = battery::charge_level();
        EXPECT_TRUE(level == -1.0 || (level >= 0.0 && level <= 1.0));

        EXPECT_NO_THROW((void)battery::state());
        EXPECT_NO_THROW((void)battery::power_source());

        const enum energy_saver_status saver = battery::energy_saver_status();
        EXPECT_TRUE(saver == energy_saver_status::on || saver == energy_saver_status::off);
    }

    TEST(essentials_ios, battery_listener_lifecycle)
    {
        const auto battery_token = battery::add_battery_info_changed([](const battery_info_changed_event_args&) {});
        EXPECT_TRUE(battery::remove_battery_info_changed(battery_token));

        const auto saver_token = battery::add_energy_saver_status_changed([](const enum energy_saver_status&) {});
        EXPECT_TRUE(battery::remove_energy_saver_status_changed(saver_token));
    }

    TEST(essentials_ios, flashlight_matches_capture_device_support)
    {
        // The simulator has no video capture device -> unsupported; on hardware the torch exists.
        if (flashlight::is_supported())
        {
            EXPECT_NO_THROW(flashlight::turn_on());
            EXPECT_NO_THROW(flashlight::turn_off());
        }
        else
        {
            EXPECT_THROW(flashlight::turn_on(), feature_not_supported);
            EXPECT_THROW(flashlight::turn_off(), feature_not_supported);
        }
    }

    TEST(essentials_ios, vibration_is_supported)
    {
        EXPECT_TRUE(vibration::is_supported());
        EXPECT_NO_THROW(vibration::vibrate()); // the system vibrate sound is a no-op on the simulator
        EXPECT_NO_THROW(vibration::vibrate(std::chrono::milliseconds{9000})); // clamped to 5 s upstream
        EXPECT_NO_THROW(vibration::cancel());
    }

    TEST(essentials_ios, haptic_feedback_performs)
    {
        EXPECT_TRUE(haptic_feedback::is_supported());
        EXPECT_NO_THROW(haptic_feedback::perform());
        EXPECT_NO_THROW(haptic_feedback::perform(haptic_feedback_type::long_press));
    }

    // The sensor lifecycle contract: is_supported() never throws on iOS (real partials); when the
    // device exposes the sensor the full start/double-start/stop cycle must hold, and when it does
    // not (every simulator) the feature_not_supported gates must fire.
    template <class Facade> void expect_sensor_lifecycle()
    {
        EXPECT_FALSE(Facade::is_monitoring());
        bool supported = false;
        EXPECT_NO_THROW(supported = Facade::is_supported());
        if (supported)
        {
            Facade::start(sensor_speed::ui);
            EXPECT_TRUE(Facade::is_monitoring());
            EXPECT_THROW(Facade::start(sensor_speed::ui), std::logic_error); // double start
            Facade::stop();
            EXPECT_FALSE(Facade::is_monitoring());
            EXPECT_NO_THROW(Facade::stop()); // stop while stopped is a no-op
        }
        else
        {
            EXPECT_THROW(Facade::start(sensor_speed::ui), feature_not_supported);
            EXPECT_THROW(Facade::stop(), feature_not_supported);
            EXPECT_FALSE(Facade::is_monitoring());
        }
    }

    TEST(essentials_ios, accelerometer_lifecycle)
    {
        expect_sensor_lifecycle<accelerometer>();
    }
    TEST(essentials_ios, gyroscope_lifecycle)
    {
        expect_sensor_lifecycle<gyroscope>();
    }
    TEST(essentials_ios, magnetometer_lifecycle)
    {
        expect_sensor_lifecycle<magnetometer>();
    }
    TEST(essentials_ios, compass_lifecycle)
    {
        expect_sensor_lifecycle<compass>();
    }
    // IPlatformCompass.ShouldDisplayHeadingCalibration (Compass.ios.cs): the real iOS partial tracks
    // the bool independent of monitoring (heading is unavailable on the simulator, so this is a
    // property-only check). Defaults to false and round-trips get/set on the facade.
    TEST(essentials_ios, compass_should_display_heading_calibration_property)
    {
        const bool original = compass::should_display_heading_calibration();
        EXPECT_FALSE(original);

        compass::set_should_display_heading_calibration(true);
        EXPECT_TRUE(compass::should_display_heading_calibration());

        compass::set_should_display_heading_calibration(false);
        EXPECT_FALSE(compass::should_display_heading_calibration());

        compass::set_should_display_heading_calibration(original); // restore the shared default
    }
    TEST(essentials_ios, barometer_lifecycle)
    {
        expect_sensor_lifecycle<barometer>();
    }
    TEST(essentials_ios, orientation_sensor_lifecycle)
    {
        expect_sensor_lifecycle<orientation_sensor>();
    }

    TEST(essentials_ios, geolocation_passive_surface)
    {
        EXPECT_FALSE(geolocation::is_listening_foreground());
        EXPECT_NO_THROW(geolocation::stop_listening_foreground()); // no-op while not listening
        EXPECT_NO_THROW((void)geolocation::is_enabled());          // a real CLLocationManager query
    }

    TEST(essentials_ios, geocoding_default_exists)
    {
        // CLGeocoder queries need network + authorization; the headless fake is the behavioral
        // test path. Here: the platform implementation must simply materialize.
        EXPECT_NO_THROW((void)geocoding::default_());
    }
} // namespace
