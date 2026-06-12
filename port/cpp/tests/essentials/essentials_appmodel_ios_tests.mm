// The W1-17 app-model + storage suite against the REAL iOS partials, run ON the simulator (via
// tools/ios-sim-run.sh): true NSUserDefaults preferences (uniquely-keyed entries, removed in
// teardown), the simulator KEYCHAIN secure storage (the link-embedded
// tools/ios-sim-entitlements.plist entitles SecItem* on iOS 26.5+; the full DeviceTests
// round-trip runs), NSSearchPath file-system directories, the GCD main-thread
// facade, and the documented NO-UIAPPLICATION surface: the spawned gtest process has no
// UIApplication instance, so launcher queries/open complete false, the browser has no view
// controller to present from (SystemPreferred -> false) and External routes to the false-
// completing launcher, app_actions get/set hit the nil-app no-op paths, and the location
// permission's ensure_declared throws for the missing Info.plist key (the unbundled process has
// none) - exactly the status-query architecture the headless fakes exercise behaviorally.

#import <UIKit/UIKit.h>

#include <chrono>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "maui/core/app_theme.hpp"
#include "maui/essentials/app_actions.hpp"
#include "maui/essentials/app_info.hpp"
#include "maui/essentials/browser.hpp"
#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/file_system.hpp"
#include "maui/essentials/launcher.hpp"
#include "maui/essentials/main_thread.hpp"
#include "maui/essentials/permissions.hpp"
#include "maui/essentials/preferences.hpp"
#include "maui/essentials/secure_storage.hpp"

namespace
{
    using namespace maui::application_model;
    using maui::core::app_theme;
    using maui::storage::date_time;
    using maui::storage::preferences;
    using maui::storage::secure_storage;

    // Per-TEST key prefix + suite: ctest runs each case as a parallel process against the SAME
    // persistent NSUserDefaults domains, so shared keys would race a sibling's teardown.
    std::string test_scope()
    {
        return std::string("maui.port.w117.ios.") + ::testing::UnitTest::GetInstance()->current_test_info()->name();
    }
    std::string suite_name()
    {
        return test_scope() + ".suite";
    }

    class appmodel_ios_preferences : public ::testing::Test
    {
    protected:
        void TearDown() override
        {
            for (const std::string_view name : {"string1", "int1", "bool1", "double1", "float1", "long1", "datetime1"})
            {
                preferences::remove(key(name));
                preferences::remove(key(name), suite_name());
            }
            preferences::clear(suite_name());
        }

        [[nodiscard]] static std::string key(std::string_view name)
        {
            return test_scope() + "." + std::string(name);
        }
    };

    TEST_F(appmodel_ios_preferences, typed_round_trips_in_standard_defaults)
    {
        preferences::set_string(key("string1"), "TEST");
        EXPECT_EQ(preferences::get_string(key("string1"), std::nullopt), "TEST");

        preferences::set_int(key("int1"), 2147483646);
        EXPECT_EQ(preferences::get_int(key("int1"), 0), 2147483646);

        preferences::set_bool(key("bool1"), true);
        EXPECT_TRUE(preferences::get_bool(key("bool1"), false));

        preferences::set_double(key("double1"), 1234.5);
        EXPECT_EQ(preferences::get_double(key("double1"), 0.0), 1234.5);

        preferences::set_float(key("float1"), 2.5F);
        EXPECT_EQ(preferences::get_float(key("float1"), 0.0F), 2.5F);

        preferences::set_long(key("long1"), 9223372036854775806LL);
        EXPECT_EQ(preferences::get_long(key("long1"), 0), 9223372036854775806LL);

        const date_time stamp =
            std::chrono::sys_days{std::chrono::May / 7 / 2018} + std::chrono::hours{8} + std::chrono::minutes{30};
        preferences::set_date_time(key("datetime1"), stamp);
        EXPECT_EQ(preferences::get_date_time(key("datetime1"), date_time{}), stamp);
    }

    TEST_F(appmodel_ios_preferences, missing_keys_defaults_contains_key_and_suite)
    {
        preferences::remove(key("string1"));
        EXPECT_EQ(preferences::get_string(key("string1"), std::nullopt), std::nullopt);
        EXPECT_EQ(preferences::get_string(key("string1"), "text"), "text");
        EXPECT_FALSE(preferences::contains_key(key("string1")));

        preferences::set_int(key("int1"), 1);
        preferences::set_int(key("int1"), 2, suite_name());
        EXPECT_EQ(preferences::get_int(key("int1"), 0), 1);
        EXPECT_EQ(preferences::get_int(key("int1"), 0, suite_name()), 2);

        preferences::set_string(key("string1"), "value");
        preferences::set_string(key("string1"), std::nullopt); // the null set removes
        EXPECT_FALSE(preferences::contains_key(key("string1")));
    }

    // ONE keychain case (not parallel fixtures): remove_all() is service-wide, so sibling test
    // PROCESSES under ctest -j would wipe each other's records mid-assert.
    std::optional<std::string> secure_get(std::string_view key)
    {
        std::optional<std::string> result;
        secure_storage::get_async(key, [&result](const std::optional<std::string>& value) { result = value; });
        return result;
    }

    // Saves_And_Loads + Saves_Same_Key_Twice + Non_Existent_Key_Returns_Null + Remove_Key +
    // Remove_All_Keys, on the REAL simulator keychain.
    TEST(appmodel_ios_secure_storage, keychain_round_trip_and_lifecycle)
    {
        // The link-embedded tools/ios-sim-entitlements.plist entitles SecItem* for this spawned
        // binary (iOS 26.5+ enforces it: errSecMissingEntitlement -34018 otherwise). Keep a
        // defensive probe-and-skip for runtimes where the embedding stops being honored — the
        // macOS suite independently covers the real keychain round trip.
        try
        {
            secure_storage::set_async("ENTITLEMENT_PROBE", "probe");
        }
        catch (const std::exception& error)
        {
            if (std::string_view{error.what()}.find("-34018") != std::string_view::npos)
            {
                GTEST_SKIP() << "no keychain entitlement under simctl spawn: " << error.what();
            }
            throw;
        }
        secure_storage::remove_all(); // the DeviceTests ctor: a clean keychain service

        const std::pair<std::string_view, std::string_view> cases[] = {
            {"test.txt", "data"},
            {"noextension", "data2"},
            {"funny*&$%@!._/\\chars", "data3"},
        };
        for (const auto& [key, data] : cases)
        {
            secure_storage::set_async(key, data);
            EXPECT_EQ(secure_get(key), data);
        }

        secure_storage::set_async("test.txt", "data2"); // same key twice - the second value wins
        EXPECT_EQ(secure_get("test.txt"), "data2");

        EXPECT_EQ(secure_get("THIS_KEY_SHOULD_NOT_EXIST"), std::nullopt);

        EXPECT_TRUE(secure_storage::remove("test.txt"));
        EXPECT_EQ(secure_get("test.txt"), std::nullopt);
        EXPECT_FALSE(secure_storage::remove("test.txt"));

        secure_storage::set_async("KEYS_TO_REMOVEA1", "Irrelevant Data");
        secure_storage::set_async("KEYS_TO_REMOVEA2", "Irrelevant Data");
        secure_storage::remove_all();
        EXPECT_EQ(secure_get("KEYS_TO_REMOVEA1"), std::nullopt);
        EXPECT_EQ(secure_get("KEYS_TO_REMOVEA2"), std::nullopt);
    }

    TEST(appmodel_ios_app_info, real_surface_is_callable)
    {
        // The spawned simulator process is unbundled - the bundle reads are "" (C#'s null).
        EXPECT_NO_THROW((void)app_info::package_name());
        EXPECT_NO_THROW((void)app_info::name());
        EXPECT_NO_THROW((void)app_info::build_string());
        EXPECT_EQ(app_info::version(), maui::devices::version_info::parse(app_info::version_string()));

        // iOS >= 13 always reports a concrete style (App_Theme_Is_Correct).
        const app_theme theme = app_info::requested_theme();
        EXPECT_TRUE(theme == app_theme::light || theme == app_theme::dark);
        EXPECT_EQ(app_info::packaging_model(), app_packaging_model::packaged);
        // App_RequestedLayoutDirection_Is_Correct (a nil sharedApplication reads LTR).
        EXPECT_EQ(app_info::requested_layout_direction(), layout_direction::left_to_right);
    }

    TEST(appmodel_ios_file_system, directories_and_package_probe)
    {
        EXPECT_FALSE(maui::storage::file_system::cache_directory().empty());
        EXPECT_FALSE(maui::storage::file_system::app_data_directory().empty());
        EXPECT_FALSE(maui::storage::file_system::app_package_file_exists("maui_port_definitely_missing.txt"));
        EXPECT_THROW((void)maui::storage::file_system::open_app_package_file("maui_port_definitely_missing.txt"),
                     std::runtime_error);
    }

    TEST(appmodel_ios_main_thread, is_main_thread_tracks_the_calling_thread)
    {
        EXPECT_TRUE(main_thread::is_main_thread());

        bool off_main = true;
        std::thread worker([&off_main] { off_main = main_thread::is_main_thread(); });
        worker.join();
        EXPECT_FALSE(off_main);

        bool ran = false;
        main_thread::begin_invoke_on_main_thread([&ran] { ran = true; }); // inline on main
        EXPECT_TRUE(ran);
    }

    // The documented no-UIApplication surface: every launcher path completes false inline.
    TEST(appmodel_ios_launcher, no_application_surface_reports_false)
    {
        ASSERT_TRUE([UIApplication sharedApplication] == nil); // the premise of this suite

        bool can_open = true;
        launcher::can_open_async("https://example.com", [&can_open](bool value) { can_open = value; });
        EXPECT_FALSE(can_open);

        bool opened = true;
        launcher::open_async("https://example.com", [&opened](bool value) { opened = value; });
        EXPECT_FALSE(opened);

        bool tried = true;
        launcher::try_open_async("https://example.com", [&tried](bool value) { tried = value; });
        EXPECT_FALSE(tried);
    }

    // SystemPreferred has no view controller to present from; External routes to the launcher.
    TEST(appmodel_ios_browser, no_application_surface_reports_false)
    {
        bool in_app = true;
        browser::open_async("https://example.com", browser_launch_mode::system_preferred,
                            [&in_app](bool value) { in_app = value; });
        EXPECT_FALSE(in_app);

        bool external = true;
        browser::open_async("https://example.com", browser_launch_mode::external,
                            [&external](bool value) { external = value; });
        EXPECT_FALSE(external);
    }

    // IsSupported (DeviceTests: true on iOS); the nil-app get/set surface; the event accessors.
    TEST(appmodel_ios_app_actions, supported_with_nil_app_surface)
    {
        EXPECT_TRUE(app_actions::is_supported());

        EXPECT_NO_THROW(app_actions::set_async({app_action("TEST1", "Test 1", "This is a test", "myapp://test1")}));
        std::vector<app_action> read;
        app_actions::get_async([&read](const std::vector<app_action>& value) { read = value; });
        EXPECT_TRUE(read.empty()); // nil sharedApplication - the store is unreachable

        const auto token = app_actions::add_on_app_action([](const app_action&) {});
        EXPECT_TRUE(app_actions::remove_on_app_action(token));
    }

    // The ios permission architecture: base auto-grant + the location plist gate + status query.
    TEST(appmodel_ios_permissions, base_auto_grant_and_location_declaration_gate)
    {
        std::optional<permission_status> battery_status;
        permissions::check_status_async<permissions::battery>(
            [&battery_status](permission_status value) { battery_status = value; });
        EXPECT_EQ(battery_status, permission_status::granted);

        std::optional<permission_status> network_status;
        permissions::request_async<permissions::network_state>(
            [&network_status](permission_status value) { network_status = value; });
        EXPECT_EQ(network_status, permission_status::granted);

        EXPECT_NO_THROW(permissions::ensure_declared<permissions::battery>());
        EXPECT_FALSE(permissions::should_show_rationale<permissions::camera>());

        // The unbundled test process declares no NSLocationWhenInUseUsageDescription - the
        // EnsureDeclared gate throws (Ensure_Declared passes in the C# DeviceTests because the
        // device-test APP declares the key).
        EXPECT_THROW(permissions::ensure_declared<permissions::location_when_in_use>(), permission_error);
        EXPECT_THROW(permissions::check_status_async<permissions::location_when_in_use>([](permission_status) {}),
                     permission_error);
    }
} // namespace
