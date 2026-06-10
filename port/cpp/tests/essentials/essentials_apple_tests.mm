// The essentials suite against the REAL macOS partials (the apple backend's lazy defaults): true
// NSProcessInfo/IOKit/SCDynamicStore device_info reads, NSScreen display metrics + the IOPM
// keep-screen-on assertion, IOPSCopyPowerSourcesInfo battery reads + the run-loop-source listener
// lifecycle, the NSHapticFeedbackManager haptics, and the suffix-oracle support matrix (flashlight
// / vibration / all six sensors are NOT supported on macOS and must throw; geolocation/geocoding
// exist but their queries need authorization/network, so only the passive surface is asserted -
// the headless fakes cover the behavioral contract).

#import <AppKit/AppKit.h>

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

    TEST(essentials_apple, device_info_reads_real_values)
    {
        EXPECT_EQ(device_info::manufacturer(), "Apple");
        EXPECT_TRUE(device_info::platform() == device_platform::mac_os());
        EXPECT_TRUE(device_info::idiom() == device_idiom::desktop());
        EXPECT_EQ(device_info::device_type(), device_type::physical);

        EXPECT_FALSE(device_info::version_string().empty());
        EXPECT_GT(device_info::version().major, 0); // a real macOS major version

        // Model comes from the IOPlatformExpertDevice registry ("MacXX,Y" on hardware); the
        // computer name may be empty in sandboxed environments - both must simply not throw.
        EXPECT_NO_THROW((void)device_info::model());
        EXPECT_NO_THROW((void)device_info::name());
    }

    TEST(essentials_apple, device_display_reads_real_metrics)
    {
        const display_info info = device_display::main_display_info();
        EXPECT_GT(info.width, 0);
        EXPECT_GT(info.height, 0);
        EXPECT_GT(info.density, 0);
        EXPECT_NE(info.orientation, display_orientation::unknown);
        EXPECT_EQ(info.rotation, display_rotation::rotation_0); // fixed on macOS
    }

    TEST(essentials_apple, device_display_keep_screen_on_round_trips)
    {
        EXPECT_FALSE(device_display::keep_screen_on());
        device_display::set_keep_screen_on(true); // IOPM "PreventUserIdleDisplaySleep" assertion
        EXPECT_TRUE(device_display::keep_screen_on());
        device_display::set_keep_screen_on(false);
        EXPECT_FALSE(device_display::keep_screen_on());
    }

    TEST(essentials_apple, device_display_listener_lifecycle)
    {
        const auto token = device_display::add_main_display_info_changed([](const display_info&) {});
        EXPECT_TRUE(device_display::remove_main_display_info_changed(token));
        EXPECT_FALSE(device_display::remove_main_display_info_changed(token));
    }

    TEST(essentials_apple, battery_reads_real_values)
    {
        // No battery aggregates to 1.0 (the C# IOKit helper's fallback); with one, a 0..1 ratio.
        const double level = battery::charge_level();
        EXPECT_GT(level, 0.0);
        EXPECT_LE(level, 1.0);

        const battery_state state = battery::state();
        EXPECT_TRUE(state == battery_state::charging || state == battery_state::discharging ||
                    state == battery_state::full || state == battery_state::not_charging ||
                    state == battery_state::not_present || state == battery_state::unknown);

        const battery_power_source source = battery::power_source();
        EXPECT_TRUE(source == battery_power_source::battery || source == battery_power_source::ac ||
                    source == battery_power_source::unknown);

        EXPECT_EQ(battery::energy_saver_status(), energy_saver_status::off); // always Off on macOS
    }

    TEST(essentials_apple, battery_listener_lifecycle)
    {
        const auto battery_token = battery::add_battery_info_changed([](const battery_info_changed_event_args&) {});
        EXPECT_TRUE(battery::remove_battery_info_changed(battery_token));

        // Energy-saver listeners are no-ops on macOS but the add/remove pair must round-trip.
        const auto saver_token = battery::add_energy_saver_status_changed([](const enum energy_saver_status&) {});
        EXPECT_TRUE(battery::remove_energy_saver_status_changed(saver_token));
    }

    TEST(essentials_apple, haptic_feedback_performs)
    {
        EXPECT_TRUE(haptic_feedback::is_supported());
        EXPECT_NO_THROW(haptic_feedback::perform());                                 // Click: no-op on macOS
        EXPECT_NO_THROW(haptic_feedback::perform(haptic_feedback_type::long_press)); // Generic pattern
    }

    TEST(essentials_apple, flashlight_is_not_supported)
    {
        EXPECT_FALSE(flashlight::is_supported());
        EXPECT_THROW(flashlight::turn_on(), feature_not_supported);
        EXPECT_THROW(flashlight::turn_off(), feature_not_supported);
    }

    TEST(essentials_apple, vibration_is_not_supported)
    {
        EXPECT_THROW((void)vibration::is_supported(), feature_not_supported);
        EXPECT_THROW(vibration::vibrate(), feature_not_supported);
        EXPECT_THROW(vibration::cancel(), feature_not_supported);
    }

    // The suffix oracle: every sensor's macOS coverage is the netstandard partial -> throws.
    template <class Facade> void expect_sensor_not_supported()
    {
        EXPECT_FALSE(Facade::is_monitoring());
        EXPECT_THROW((void)Facade::is_supported(), feature_not_supported);
        EXPECT_THROW(Facade::start(sensor_speed::default_), feature_not_supported);
        EXPECT_THROW(Facade::stop(), feature_not_supported);
    }

    TEST(essentials_apple, sensors_are_not_supported)
    {
        expect_sensor_not_supported<accelerometer>();
        expect_sensor_not_supported<gyroscope>();
        expect_sensor_not_supported<magnetometer>();
        expect_sensor_not_supported<compass>();
        expect_sensor_not_supported<barometer>();
        expect_sensor_not_supported<orientation_sensor>();
    }

    TEST(essentials_apple, geolocation_passive_surface)
    {
        EXPECT_FALSE(geolocation::is_listening_foreground());
        EXPECT_NO_THROW(geolocation::stop_listening_foreground()); // no-op while not listening
        EXPECT_NO_THROW((void)geolocation::is_enabled());          // a real CLLocationManager query
    }

    TEST(essentials_apple, geocoding_default_exists)
    {
        // The real CLGeocoder queries need network + authorization; the headless fake is the
        // behavioral test path. Here: the platform implementation must simply materialize.
        EXPECT_NO_THROW((void)geocoding::default_());
    }
} // namespace
