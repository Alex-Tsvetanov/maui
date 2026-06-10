// device_display on the headless backend: the netstandard mirror returns defaults (no throw), the
// add/remove accessors drive DeviceDisplayImplementationBase's listener lifecycle (first add
// primes the cache + starts the platform listeners, last remove stops them), and
// on_main_display_info_changed dedupes by DisplayInfo equality (refresh-rate changes alone do NOT
// raise - C# equality excludes it).

#include <memory>

#include <gtest/gtest.h>

#include "maui/essentials/device_display.hpp"

#include "src/platform/headless/essentials_fakes.hpp"

namespace
{
    using namespace maui::devices;

    class device_display_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            device_display::set_current(nullptr);
        }
        void TearDown() override
        {
            device_display::set_current(nullptr);
        }
    };

    TEST_F(device_display_test, netstandard_mirror_returns_defaults)
    {
        EXPECT_FALSE(device_display::keep_screen_on());
        EXPECT_TRUE(device_display::main_display_info() == display_info{});
    }

    TEST_F(device_display_test, keep_screen_on_round_trips_on_the_fake)
    {
        device_display::set_keep_screen_on(true);
        EXPECT_TRUE(device_display::keep_screen_on());
        device_display::set_keep_screen_on(false);
        EXPECT_FALSE(device_display::keep_screen_on());
    }

    TEST_F(device_display_test, listener_lifecycle_and_dedupe)
    {
        auto fake = std::make_shared<headless_device_display>();
        device_display::set_current(fake);

        int raises = 0;
        display_info last{};
        EXPECT_FALSE(fake->is_screen_metrics_listening());
        const auto token = device_display::add_main_display_info_changed([&](const display_info& info) {
            ++raises;
            last = info;
        });
        EXPECT_TRUE(fake->is_screen_metrics_listening()); // first add starts the platform listeners

        const display_info metrics{.width = 1920,
                                   .height = 1080,
                                   .density = 2.0,
                                   .orientation = display_orientation::landscape,
                                   .rotation = display_rotation::rotation_0,
                                   .refresh_rate = 60};
        fake->set_main_display_info(metrics);
        EXPECT_EQ(raises, 1);
        EXPECT_TRUE(last == metrics);

        // Same metrics again -> deduped (no raise).
        fake->set_main_display_info(metrics);
        EXPECT_EQ(raises, 1);

        // A refresh-rate-only change is invisible to DisplayInfo equality -> still no raise.
        display_info faster = metrics;
        faster.refresh_rate = 120;
        fake->set_main_display_info(faster);
        EXPECT_EQ(raises, 1);

        display_info rotated = metrics;
        rotated.orientation = display_orientation::portrait;
        rotated.rotation = display_rotation::rotation_90;
        fake->set_main_display_info(rotated);
        EXPECT_EQ(raises, 2);

        EXPECT_TRUE(device_display::remove_main_display_info_changed(token));
        EXPECT_FALSE(fake->is_screen_metrics_listening()); // last remove stops the listeners

        fake->set_main_display_info(metrics);
        EXPECT_EQ(raises, 2); // nothing raised after teardown
    }

    TEST_F(device_display_test, second_subscriber_does_not_restart_listeners)
    {
        auto fake = std::make_shared<headless_device_display>();
        device_display::set_current(fake);

        const auto first = device_display::add_main_display_info_changed([](const display_info&) {});
        const auto second = device_display::add_main_display_info_changed([](const display_info&) {});
        EXPECT_TRUE(fake->is_screen_metrics_listening());

        EXPECT_TRUE(device_display::remove_main_display_info_changed(first));
        EXPECT_TRUE(fake->is_screen_metrics_listening()); // one subscriber left
        EXPECT_TRUE(device_display::remove_main_display_info_changed(second));
        EXPECT_FALSE(fake->is_screen_metrics_listening());

        EXPECT_FALSE(device_display::remove_main_display_info_changed(second)); // already removed
    }
} // namespace
