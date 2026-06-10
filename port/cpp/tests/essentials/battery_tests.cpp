// battery on the headless backend. Ports Battery_Tests.cs (the netstandard throws - including the
// listener start hidden behind the event add) and exercises the shared BatteryImplementation
// logic via the fake: the listener lifecycle behind add/remove and OnBatteryInfoChanged's
// change-dedupe cache (raise only when level/state/source actually changed); energy-saver raises
// are never deduped.

#include <memory>

#include <gtest/gtest.h>

#include "maui/essentials/battery.hpp"
#include "maui/essentials/feature_not_supported.hpp"

#include "src/platform/headless/essentials_fakes.hpp"

namespace
{
    using namespace maui::devices;
    using maui::application_model::feature_not_supported;

    class battery_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            battery::set_default(nullptr);
        }
        void TearDown() override
        {
            battery::set_default(nullptr);
        }
    };

    // Battery_Tests: Charge_Level/State/Power_Source/Changed_Event on netstandard all throw.
    TEST_F(battery_test, netstandard_mirror_throws_until_configured)
    {
        EXPECT_THROW((void)battery::charge_level(), feature_not_supported);
        EXPECT_THROW((void)battery::state(), feature_not_supported);
        EXPECT_THROW((void)battery::power_source(), feature_not_supported);
        EXPECT_THROW((void)battery::energy_saver_status(), feature_not_supported);
        EXPECT_THROW((void)battery::add_battery_info_changed([](const battery_info_changed_event_args&) {}),
                     feature_not_supported);
        EXPECT_THROW((void)battery::add_energy_saver_status_changed([](const enum energy_saver_status&) {}),
                     feature_not_supported);
    }

    TEST_F(battery_test, configured_fake_reads_through_facade)
    {
        auto fake = std::make_shared<headless_battery>();
        fake->set_charge_level(0.87);
        fake->set_state(battery_state::charging);
        fake->set_power_source(battery_power_source::ac);
        fake->set_energy_saver_status(energy_saver_status::off);
        battery::set_default(fake);

        EXPECT_DOUBLE_EQ(battery::charge_level(), 0.87);
        EXPECT_EQ(battery::state(), battery_state::charging);
        EXPECT_EQ(battery::power_source(), battery_power_source::ac);
        EXPECT_EQ(battery::energy_saver_status(), energy_saver_status::off);
    }

    TEST_F(battery_test, battery_info_changed_lifecycle_and_dedupe)
    {
        auto fake = std::make_shared<headless_battery>();
        fake->set_charge_level(0.5);
        fake->set_state(battery_state::discharging);
        fake->set_power_source(battery_power_source::battery);
        battery::set_default(fake);

        int raises = 0;
        battery_info_changed_event_args last{};
        EXPECT_FALSE(fake->is_battery_listening());
        const auto token = battery::add_battery_info_changed([&](const battery_info_changed_event_args& args) {
            ++raises;
            last = args;
        });
        EXPECT_TRUE(fake->is_battery_listening());

        // First change notification: values differ from the (zero) cache -> raise.
        fake->simulate_battery_info_changed();
        EXPECT_EQ(raises, 1);
        EXPECT_DOUBLE_EQ(last.charge_level, 0.5);
        EXPECT_EQ(last.state, battery_state::discharging);
        EXPECT_EQ(last.power_source, battery_power_source::battery);

        // Unchanged values -> deduped.
        fake->simulate_battery_info_changed();
        EXPECT_EQ(raises, 1);

        fake->set_charge_level(0.49);
        fake->simulate_battery_info_changed();
        EXPECT_EQ(raises, 2);

        EXPECT_TRUE(battery::remove_battery_info_changed(token));
        EXPECT_FALSE(fake->is_battery_listening());
    }

    TEST_F(battery_test, energy_saver_changed_is_not_deduped)
    {
        auto fake = std::make_shared<headless_battery>();
        fake->set_energy_saver_status(energy_saver_status::on);
        battery::set_default(fake);

        int raises = 0;
        enum energy_saver_status last = energy_saver_status::unknown;
        const auto token = battery::add_energy_saver_status_changed([&](const enum energy_saver_status& status) {
            ++raises;
            last = status;
        });
        EXPECT_TRUE(fake->is_energy_saver_listening());

        fake->simulate_energy_saver_changed();
        fake->simulate_energy_saver_changed();
        EXPECT_EQ(raises, 2); // no dedupe, mirroring C# OnEnergySaverChanged
        EXPECT_EQ(last, energy_saver_status::on);

        EXPECT_TRUE(battery::remove_energy_saver_status_changed(token));
        EXPECT_FALSE(fake->is_energy_saver_listening());
    }
} // namespace
