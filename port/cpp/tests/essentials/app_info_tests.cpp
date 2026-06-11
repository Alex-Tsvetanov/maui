// app_info on the headless backend: the unconfigured fake mirrors AppInfo's netstandard partial
// (package/name/version/build/settings-ui/packaging-model throw; requested theme Unspecified and
// layout direction Unknown are RETURNED, not thrown), the configured fake exposes the full
// surface through the facade (the DeviceTests AppInfo_Tests assertions against staged manifest
// values), and version() parses version_string() exactly like Utils.ParseVersion.

#include <memory>

#include <gtest/gtest.h>

#include "maui/core/app_theme.hpp"
#include "maui/essentials/app_info.hpp"
#include "maui/essentials/feature_not_supported.hpp"

#include "src/platform/headless/essentials_appmodel_fakes.hpp"

namespace
{
    using namespace maui::application_model;
    using maui::application_model::feature_not_supported;
    using maui::core::app_theme;
    using maui::devices::version_info;

    class app_info_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            app_info::set_current(nullptr);
        }
        void TearDown() override
        {
            app_info::set_current(nullptr);
        }
    };

    TEST_F(app_info_test, netstandard_mirror_throws_until_configured)
    {
        EXPECT_THROW((void)app_info::package_name(), feature_not_supported);
        EXPECT_THROW((void)app_info::name(), feature_not_supported);
        EXPECT_THROW((void)app_info::version_string(), feature_not_supported);
        EXPECT_THROW((void)app_info::version(), feature_not_supported); // parses VersionString
        EXPECT_THROW((void)app_info::build_string(), feature_not_supported);
        EXPECT_THROW(app_info::show_settings_ui(), feature_not_supported);
        EXPECT_THROW((void)app_info::packaging_model(), feature_not_supported);
        // Returned defaults, not throws (the netstandard partial).
        EXPECT_EQ(app_info::requested_theme(), app_theme::unspecified);
        EXPECT_EQ(app_info::requested_layout_direction(), layout_direction::unknown);
    }

    // The DeviceTests assertions (AppName/PackageName/Build/Versions_Are_Correct) against the
    // staged manifest values.
    TEST_F(app_info_test, configured_fake_flows_through_facade)
    {
        auto fake = std::make_shared<headless_app_info>();
        fake->set_package_name("com.microsoft.maui.essentials.devicetests");
        fake->set_name("Essentials Tests");
        fake->set_version_string("1.0");
        fake->set_build_string("1");
        fake->set_requested_theme(app_theme::dark);
        fake->set_packaging_model(app_packaging_model::packaged);
        fake->set_requested_layout_direction(layout_direction::left_to_right);
        app_info::set_current(fake);

        EXPECT_EQ(app_info::package_name(), "com.microsoft.maui.essentials.devicetests");
        EXPECT_EQ(app_info::name(), "Essentials Tests");
        EXPECT_EQ(app_info::version_string(), "1.0");
        EXPECT_EQ(app_info::version(), (version_info{1, 0, -1, -1}));
        EXPECT_EQ(app_info::build_string(), "1");
        EXPECT_EQ(app_info::requested_theme(), app_theme::dark);
        EXPECT_EQ(app_info::packaging_model(), app_packaging_model::packaged);
        EXPECT_EQ(app_info::requested_layout_direction(), layout_direction::left_to_right);
    }

    TEST_F(app_info_test, show_settings_ui_records_when_supported)
    {
        auto fake = std::make_shared<headless_app_info>();
        fake->set_show_settings_ui_supported(true);
        app_info::set_current(fake);

        app_info::show_settings_ui();
        app_info::show_settings_ui();
        EXPECT_EQ(fake->settings_ui_shown_count(), 2);
    }

    TEST_F(app_info_test, set_current_null_restores_lazy_default)
    {
        auto fake = std::make_shared<headless_app_info>();
        fake->set_name("custom");
        app_info::set_current(fake);
        EXPECT_EQ(app_info::name(), "custom");

        app_info::set_current(nullptr);
        EXPECT_THROW((void)app_info::name(), feature_not_supported);
    }
} // namespace
