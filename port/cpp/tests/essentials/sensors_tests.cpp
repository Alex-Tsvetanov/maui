// The six sensors on the headless backend. Ports the netstandard halves of
// {Accelerometer,Gyroscope,Magnetometer,Compass,Barometer,OrientationSensor}_Tests.cs (Start/Stop/
// IsSupported throw, IsMonitoring false) and exercises the shared lifecycle + readings through the
// fakes: start/stop bookkeeping, the per-sensor double-start std::logic_error message, reading
// events, the compass low-pass-filter default (Start(speed) => true), and the accelerometer's
// shake detection end-to-end with a deterministic clock.

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include "maui/essentials/accelerometer.hpp"
#include "maui/essentials/barometer.hpp"
#include "maui/essentials/compass.hpp"
#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/gyroscope.hpp"
#include "maui/essentials/magnetometer.hpp"
#include "maui/essentials/orientation_sensor.hpp"
#include "maui/essentials/sensor_types.hpp"

#include "src/platform/headless/essentials_fakes.hpp"

namespace
{
    using namespace maui::devices::sensors;
    using maui::application_model::feature_not_supported;

    class sensors_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            reset();
        }
        void TearDown() override
        {
            reset();
        }
        static void reset()
        {
            accelerometer::set_default(nullptr);
            gyroscope::set_default(nullptr);
            magnetometer::set_default(nullptr);
            compass::set_default(nullptr);
            barometer::set_default(nullptr);
            orientation_sensor::set_default(nullptr);
        }
    };

    // The netstandard mirror, identical for every sensor facade (the C# *_Tests netstandard half).
    template <class Facade> void expect_netstandard_mirror()
    {
        EXPECT_FALSE(Facade::is_monitoring());
        EXPECT_THROW((void)Facade::is_supported(), feature_not_supported);
        EXPECT_THROW(Facade::start(sensor_speed::default_), feature_not_supported);
        EXPECT_THROW(Facade::stop(), feature_not_supported);
    }

    TEST_F(sensors_test, netstandard_mirror_for_every_sensor)
    {
        expect_netstandard_mirror<accelerometer>();
        expect_netstandard_mirror<gyroscope>();
        expect_netstandard_mirror<magnetometer>();
        expect_netstandard_mirror<compass>();
        expect_netstandard_mirror<barometer>();
        expect_netstandard_mirror<orientation_sensor>();
    }

    // The shared lifecycle, exercised once per fake type through its facade.
    template <class Facade, class Fake, class Data>
    void expect_lifecycle(const Data& reading, const std::string& double_start_message)
    {
        auto fake = std::make_shared<Fake>();
        fake->set_is_supported(true);
        Facade::set_default(fake);

        EXPECT_TRUE(Facade::is_supported());
        EXPECT_FALSE(Facade::is_monitoring());

        Facade::start(sensor_speed::game);
        EXPECT_TRUE(Facade::is_monitoring());
        ASSERT_TRUE(fake->started_speed().has_value());
        EXPECT_EQ(fake->started_speed(), std::optional(sensor_speed::game));

        // Double start -> InvalidOperationException("<Sensor> has already been started.").
        try
        {
            Facade::start(sensor_speed::ui);
            FAIL() << "expected std::logic_error";
        }
        catch (const std::logic_error& error)
        {
            EXPECT_EQ(std::string(error.what()), double_start_message);
        }
        EXPECT_TRUE(Facade::is_monitoring()); // the failed re-start does not stop monitoring

        // A reading flows through the shared raise path.
        int raises = 0;
        Data last{};
        const auto token = Facade::reading_changed().connect([&](const Data& data) {
            ++raises;
            last = data;
        });
        fake->simulate_reading(reading);
        EXPECT_EQ(raises, 1);
        EXPECT_TRUE(last == reading);
        Facade::reading_changed().disconnect(token);

        Facade::stop();
        EXPECT_FALSE(Facade::is_monitoring());
        EXPECT_FALSE(fake->started_speed().has_value());
        Facade::stop(); // stopping while stopped is a no-op
        EXPECT_FALSE(Facade::is_monitoring());

        // Unsupported again -> the facade gates throw once more.
        fake->set_is_supported(false);
        EXPECT_THROW(Facade::start(sensor_speed::default_), feature_not_supported);
    }

    TEST_F(sensors_test, accelerometer_lifecycle)
    {
        expect_lifecycle<accelerometer, headless_accelerometer>(accelerometer_data(0.1F, 0.2F, 0.3F),
                                                                "Accelerometer has already been started.");
    }

    TEST_F(sensors_test, gyroscope_lifecycle)
    {
        expect_lifecycle<gyroscope, headless_gyroscope>(gyroscope_data(1.0F, 2.0F, 3.0F),
                                                        "Gyroscope has already been started.");
    }

    TEST_F(sensors_test, magnetometer_lifecycle)
    {
        expect_lifecycle<magnetometer, headless_magnetometer>(magnetometer_data(4.0F, 5.0F, 6.0F),
                                                              "Magnetometer has already been started.");
    }

    TEST_F(sensors_test, barometer_lifecycle)
    {
        expect_lifecycle<barometer, headless_barometer>(barometer_data(1013.25), "Barometer has already been started.");
    }

    TEST_F(sensors_test, orientation_sensor_lifecycle)
    {
        expect_lifecycle<orientation_sensor, headless_orientation_sensor>(
            orientation_sensor_data(0.0F, 0.0F, 0.7F, 0.7F), "Orientation sensor has already been started.");
    }

    TEST_F(sensors_test, compass_lifecycle_and_filter_default)
    {
        auto fake = std::make_shared<headless_compass>();
        fake->set_is_supported(true);
        compass::set_default(fake);

        // Start(speed) forwards applyLowPassFilter = true (the C# overload).
        compass::start(sensor_speed::ui);
        EXPECT_TRUE(compass::is_monitoring());
        EXPECT_TRUE(fake->started_with_low_pass_filter());
        EXPECT_THROW(compass::start(sensor_speed::ui), std::logic_error);
        compass::stop();

        compass::start(sensor_speed::fastest, false);
        EXPECT_FALSE(fake->started_with_low_pass_filter());
        ASSERT_TRUE(fake->started_speed().has_value());
        EXPECT_EQ(fake->started_speed(), std::optional(sensor_speed::fastest));

        int raises = 0;
        compass_data last{};
        compass::reading_changed().connect([&](const compass_data& data) {
            ++raises;
            last = data;
        });
        fake->simulate_reading(compass_data(123.5));
        EXPECT_EQ(raises, 1);
        EXPECT_TRUE(last == compass_data(123.5));
        compass::stop();
        EXPECT_FALSE(compass::is_monitoring());
    }

    // Compass.ShouldDisplayHeadingCalibration (Compass.shared.cs / Compass.ios.cs): defaults to
    // false and round-trips get/set. The headless fake overrides the iOS-only IPlatformCompass
    // member so the value can be steered for test control even though the OS overlay is iOS-only.
    TEST_F(sensors_test, compass_should_display_heading_calibration)
    {
        auto fake = std::make_shared<headless_compass>();
        compass::set_default(fake);

        // Default is false (the C# auto-property's initializer).
        EXPECT_FALSE(compass::should_display_heading_calibration());
        EXPECT_FALSE(fake->should_display_heading_calibration());

        compass::set_should_display_heading_calibration(true);
        EXPECT_TRUE(compass::should_display_heading_calibration());
        EXPECT_TRUE(fake->should_display_heading_calibration());

        compass::set_should_display_heading_calibration(false);
        EXPECT_FALSE(compass::should_display_heading_calibration());
        EXPECT_FALSE(fake->should_display_heading_calibration());
    }

    TEST_F(sensors_test, accelerometer_shake_detected_end_to_end)
    {
        auto fake = std::make_shared<headless_accelerometer>();
        fake->set_is_supported(true);
        accelerometer::set_default(fake);
        accelerometer::start(sensor_speed::fastest);

        int shakes = 0;
        accelerometer::shake_detected().connect([&] { ++shakes; });

        // Readings are in G's: |a| = 2 G -> g = (2 * 9.81)^2 ~ 385, above the 169 threshold. Two
        // accelerating samples 0.3 s apart span the >= 250 ms window with 2/2 >= 3/4 accelerating
        // -> shake fires (and the queue clears), exactly per the AccelerometerQueue algorithm.
        constexpr std::int64_t step_ns = 300'000'000;
        std::int64_t now = 1'000'000'000'000;
        for (int i = 0; i < 2; ++i)
        {
            fake->set_now_nanoseconds(now);
            fake->simulate_reading(accelerometer_data(2.0F, 0.0F, 0.0F));
            now += step_ns;
        }
        EXPECT_EQ(shakes, 1); // detected once, then the queue clears

        // Calm readings on the cleared queue (1 G of gravity -> g ~ 96 < 169) never re-trigger.
        for (int i = 0; i < 4; ++i)
        {
            fake->set_now_nanoseconds(now);
            fake->simulate_reading(accelerometer_data(0.0F, 0.0F, 1.0F));
            now += step_ns;
        }
        EXPECT_EQ(shakes, 1);
        accelerometer::stop();
    }

    TEST_F(sensors_test, sensor_interval_mirrors_to_platform)
    {
        using std::chrono::milliseconds;
        EXPECT_EQ(sensor_interval(sensor_speed::default_), milliseconds{200});
        EXPECT_EQ(sensor_interval(sensor_speed::ui), milliseconds{60});
        EXPECT_EQ(sensor_interval(sensor_speed::game), milliseconds{20});
        EXPECT_EQ(sensor_interval(sensor_speed::fastest), milliseconds{5});
    }
} // namespace
