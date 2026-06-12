// version_tracking - PURE managed, fully headless-tested over injected fakes (the DeviceTests
// VersionTracking_Tests scenarios; the C# suite can't mock the app version, the port CAN - the
// injected app-info seam pins version "1.0" / build "1" like the device app's manifest). Each
// scenario stages the persisted trails through the preferences fake, reloads via
// init_version_tracking() (the C# InitVersionTracking), and asserts the launch flags + history
// accessors.

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "maui/essentials/app_info.hpp"
#include "maui/essentials/preferences.hpp"
#include "maui/essentials/version_tracking.hpp"

#include "src/platform/headless/essentials_appmodel_fakes.hpp"

namespace
{
    using namespace maui::application_model;
    using maui::storage::preferences;

    constexpr std::string_view current_version = "1.0";
    constexpr std::string_view current_build = "1";
    constexpr std::string_view versions_key = "VersionTracking.Versions";
    constexpr std::string_view builds_key = "VersionTracking.Builds";

    class version_tracking_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            auto fake_preferences = std::make_shared<maui::storage::headless_preferences>();
            fake_preferences->configure();
            preferences::set_default(fake_preferences);

            auto fake_app_info = std::make_shared<headless_app_info>();
            fake_app_info->set_package_name("com.maui.port.tests");
            fake_app_info->set_version_string(std::string(current_version));
            fake_app_info->set_build_string(std::string(current_build));
            app_info::set_current(fake_app_info);

            version_tracking::set_default(nullptr);
            shared_name_ = maui::storage::detail::private_preferences_shared_name("versiontracking");
        }

        void TearDown() override
        {
            version_tracking::set_default(nullptr);
            preferences::set_default(nullptr);
            app_info::set_current(nullptr);
        }

        [[nodiscard]] const std::string& shared_name() const
        {
            return shared_name_;
        }

    private:
        std::string shared_name_;
    };

    // First_Launch_Ever.
    TEST_F(version_tracking_test, first_launch_ever)
    {
        version_tracking::track();
        preferences::clear(shared_name());

        version_tracking::init_version_tracking();

        EXPECT_EQ(version_tracking::current_version(), current_version);
        EXPECT_TRUE(version_tracking::is_first_launch_ever());
        EXPECT_TRUE(version_tracking::is_first_launch_for_current_version());
        EXPECT_TRUE(version_tracking::is_first_launch_for_current_build());
    }

    // First_Launch_For_Version (and the second init flips the flag off).
    TEST_F(version_tracking_test, first_launch_for_version)
    {
        version_tracking::track();
        preferences::set_string(versions_key, "0.8.0|0.9.0|1.0.0", shared_name());
        preferences::set_string(builds_key, std::string(current_build), shared_name());

        version_tracking::init_version_tracking();

        EXPECT_EQ(version_tracking::current_version(), current_version);
        EXPECT_EQ(version_tracking::previous_version(), "1.0.0");
        EXPECT_EQ(version_tracking::first_installed_version(), "0.8.0");
        EXPECT_FALSE(version_tracking::is_first_launch_ever());
        EXPECT_TRUE(version_tracking::is_first_launch_for_current_version());
        EXPECT_FALSE(version_tracking::is_first_launch_for_current_build());
        EXPECT_TRUE(version_tracking::is_first_launch_for_version(current_version));
        EXPECT_FALSE(version_tracking::is_first_launch_for_version("0.9.0"));

        version_tracking::init_version_tracking();

        EXPECT_EQ(version_tracking::current_version(), current_version);
        EXPECT_EQ(version_tracking::previous_version(), "1.0.0");
        EXPECT_EQ(version_tracking::first_installed_version(), "0.8.0");
        EXPECT_FALSE(version_tracking::is_first_launch_ever());
        EXPECT_FALSE(version_tracking::is_first_launch_for_current_version());
        EXPECT_FALSE(version_tracking::is_first_launch_for_current_build());
    }

    // First_Launch_For_Build.
    TEST_F(version_tracking_test, first_launch_for_build)
    {
        version_tracking::track();
        preferences::set_string(versions_key, std::string(current_version), shared_name());
        preferences::set_string(builds_key, "10|20", shared_name());

        version_tracking::init_version_tracking();

        EXPECT_EQ(version_tracking::current_version(), current_version);
        EXPECT_EQ(version_tracking::previous_build(), "20");
        EXPECT_EQ(version_tracking::first_installed_build(), "10");
        EXPECT_FALSE(version_tracking::is_first_launch_ever());
        EXPECT_FALSE(version_tracking::is_first_launch_for_current_version());
        EXPECT_TRUE(version_tracking::is_first_launch_for_current_build());
        EXPECT_TRUE(version_tracking::is_first_launch_for_build(current_build));

        version_tracking::init_version_tracking();

        EXPECT_EQ(version_tracking::previous_build(), "20");
        EXPECT_EQ(version_tracking::first_installed_build(), "10");
        EXPECT_FALSE(version_tracking::is_first_launch_ever());
        EXPECT_FALSE(version_tracking::is_first_launch_for_current_version());
        EXPECT_FALSE(version_tracking::is_first_launch_for_current_build());
    }

    // First_Launch_After_Downgrade: the current version re-enters at the END of the trail.
    TEST_F(version_tracking_test, first_launch_after_downgrade)
    {
        version_tracking::track();
        preferences::set_string(versions_key, std::string(current_version) + "|1.0.2|1.0.3", shared_name());

        version_tracking::init_version_tracking();

        EXPECT_EQ(version_tracking::current_version(), current_version);
        EXPECT_EQ(version_tracking::previous_version(), "1.0.3");
        EXPECT_EQ(version_tracking::first_installed_version(), "1.0.2");
        EXPECT_FALSE(version_tracking::is_first_launch_ever());
        EXPECT_TRUE(version_tracking::is_first_launch_for_current_version());
        EXPECT_EQ(version_tracking::version_history(),
                  (std::vector<std::string>{"1.0.2", "1.0.3", std::string(current_version)}));

        version_tracking::init_version_tracking();

        EXPECT_EQ(version_tracking::previous_version(), "1.0.3");
        EXPECT_EQ(version_tracking::first_installed_version(), "1.0.2");
        EXPECT_FALSE(version_tracking::is_first_launch_ever());
        EXPECT_FALSE(version_tracking::is_first_launch_for_current_version());
    }

    // First_Launch_After_Build_Downgrade.
    TEST_F(version_tracking_test, first_launch_after_build_downgrade)
    {
        version_tracking::track();
        preferences::set_string(versions_key, std::string(current_version), shared_name());
        preferences::set_string(builds_key, std::string(current_build) + "|10|20", shared_name());

        version_tracking::init_version_tracking();

        EXPECT_EQ(version_tracking::current_build(), current_build);
        EXPECT_EQ(version_tracking::previous_build(), "20");
        EXPECT_EQ(version_tracking::first_installed_build(), "10");
        EXPECT_FALSE(version_tracking::is_first_launch_ever());
        EXPECT_FALSE(version_tracking::is_first_launch_for_current_version());
        EXPECT_TRUE(version_tracking::is_first_launch_for_current_build());
        EXPECT_EQ(version_tracking::build_history(),
                  (std::vector<std::string>{"10", "20", std::string(current_build)}));

        version_tracking::init_version_tracking();

        EXPECT_EQ(version_tracking::previous_build(), "20");
        EXPECT_EQ(version_tracking::first_installed_build(), "10");
        EXPECT_FALSE(version_tracking::is_first_launch_ever());
        EXPECT_FALSE(version_tracking::is_first_launch_for_current_version());
        EXPECT_FALSE(version_tracking::is_first_launch_for_current_build());
    }

    // The ctor tracks immediately (the C# ctor calls Track()) and persists both trails.
    TEST_F(version_tracking_test, construction_tracks_and_persists)
    {
        version_tracking::track();
        EXPECT_TRUE(preferences::contains_key(versions_key, shared_name()));
        EXPECT_TRUE(preferences::contains_key(builds_key, shared_name()));
        EXPECT_EQ(preferences::get_string(versions_key, std::nullopt, shared_name()), current_version);
        EXPECT_EQ(preferences::get_string(builds_key, std::nullopt, shared_name()), current_build);

        // track() is idempotent once loaded (the C# versionTrail != null early-out).
        version_tracking::track();
        EXPECT_TRUE(version_tracking::is_first_launch_ever());
    }
} // namespace
