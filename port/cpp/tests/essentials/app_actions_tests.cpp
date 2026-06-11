// app_actions on the headless backend: the unconfigured fake mirrors AppActions' netstandard
// partial (Essentials.UnitTests AppActions_Tests: IsSupported/Get/Set ALL throw), the
// supported=false gate mirrors the C# ios partial's FeatureNotSupportedException, and the
// supported fake runs the DeviceTests GetSetItems round-trip plus the OnAppAction activation
// event (simulate_activated = the PerformActionForShortcutItem seam).

#include <memory>
#include <optional>
#include <vector>

#include <gtest/gtest.h>

#include "maui/essentials/app_actions.hpp"
#include "maui/essentials/feature_not_supported.hpp"

#include "src/platform/headless/essentials_appmodel_fakes.hpp"

namespace
{
    using namespace maui::application_model;

    class app_actions_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            app_actions::set_current(nullptr);
        }
        void TearDown() override
        {
            app_actions::set_current(nullptr);
        }

        static std::shared_ptr<headless_app_actions> install(bool supported)
        {
            auto fake = std::make_shared<headless_app_actions>();
            fake->set_is_supported(supported);
            app_actions::set_current(fake);
            return fake;
        }
    };

    // AppActions_IsSupported / AppActions_GetActions / AppActions_SetActions (UnitTests).
    TEST_F(app_actions_test, netstandard_mirror_throws_until_configured)
    {
        EXPECT_THROW((void)app_actions::is_supported(), feature_not_supported);
        EXPECT_THROW(app_actions::get_async([](const std::vector<app_action>&) {}), feature_not_supported);
        EXPECT_THROW(app_actions::set_async({}), feature_not_supported);
    }

    // The C# ios partial's unsupported gate (get/set throw FeatureNotSupportedException).
    TEST_F(app_actions_test, unsupported_gate_throws)
    {
        install(false);
        EXPECT_FALSE(app_actions::is_supported());
        EXPECT_THROW(app_actions::get_async([](const std::vector<app_action>&) {}), feature_not_supported);
        EXPECT_THROW(app_actions::set_async({}), feature_not_supported);
    }

    // GetSetItems (DeviceTests): the set actions read back; Contains(a => a.Id == "TEST1").
    TEST_F(app_actions_test, get_set_items)
    {
        install(true);
        EXPECT_TRUE(app_actions::is_supported());

        const std::vector<app_action> actions = {
            app_action("TEST1", "Test 1", "This is a test", "myapp://test1"),
            app_action("TEST2", "Test 2", "This is a test 2", "myapp://test2"),
        };
        app_actions::set_async(actions);

        std::vector<app_action> read;
        app_actions::get_async([&read](const std::vector<app_action>& value) { read = value; });
        ASSERT_EQ(read.size(), 2U);
        EXPECT_EQ(read.front().id(), "TEST1");
        EXPECT_EQ(read.front().title(), "Test 1");
        EXPECT_EQ(read.front().subtitle(), "This is a test");
        EXPECT_EQ(read.front().icon(), "myapp://test1");
        EXPECT_EQ(read, actions);
    }

    // OnAppAction: the activation event round-trips through the facade accessors.
    TEST_F(app_actions_test, on_app_action_event)
    {
        auto fake = install(true);

        std::optional<app_action> received;
        const auto token = app_actions::add_on_app_action([&received](const app_action& action) { received = action; });

        fake->simulate_activated(app_action("TEST1", "Test 1"));
        ASSERT_TRUE(received.has_value());
        EXPECT_EQ(received->id(), "TEST1");
        EXPECT_EQ(received->subtitle(), std::nullopt);

        EXPECT_TRUE(app_actions::remove_on_app_action(token));
        EXPECT_FALSE(app_actions::remove_on_app_action(token));

        received.reset();
        fake->simulate_activated(app_action("TEST2", "Test 2"));
        EXPECT_FALSE(received.has_value());
    }
} // namespace
