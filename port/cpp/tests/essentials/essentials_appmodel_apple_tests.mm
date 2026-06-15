// The W1-17 app-model + storage suite against the REAL macOS partials (the apple backend's lazy
// defaults): true NSUserDefaults preferences (uniquely-keyed entries, removed in teardown - the
// store PERSISTS between runs), the real login-keychain secure storage (the simulator note in
// secure_storage.hpp applies to ios; macOS test binaries write the login keychain silently),
// NSSearchPath file-system directories, the GCD main-thread facade, NSWorkspace launcher
// queries, the macOS app_actions NOT-SUPPORTED matrix, and the desktop auto-grant permission
// stub. Deliberately NOT exercised here: launcher/browser OPEN (would open real apps/windows on
// the dev machine - the headless fake covers the contract) and show_settings_ui (would open
// System Settings); app_info's bundle values read as "" in the unbundled gtest process, so only
// the no-throw surface + theme/layout enums are asserted.

#import <AppKit/AppKit.h>

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
#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/file_system.hpp"
#include "maui/essentials/launcher.hpp"
#include "maui/essentials/main_thread.hpp"
#include "maui/essentials/permissions.hpp"
#include "maui/essentials/preferences.hpp"
#include "maui/essentials/secure_storage.hpp"

#include "src/platform/apple_shared/essentials_url.hpp"

namespace
{
    using namespace maui::application_model;
    using maui::core::app_theme;
    using maui::storage::date_time;
    using maui::storage::preferences;
    using maui::storage::secure_storage;

    // Uniquely-keyed test entries (NSUserDefaults persists across runs); the suite container
    // keeps clear() away from the process's standard defaults domain.
    // Per-TEST key prefix + suite: ctest runs each case as a parallel process against the SAME
    // persistent NSUserDefaults domains, so shared keys would race a sibling's teardown.
    std::string test_scope()
    {
        return std::string("maui.port.w117.apple.") + ::testing::UnitTest::GetInstance()->current_test_info()->name();
    }
    std::string suite_name()
    {
        return test_scope() + ".suite";
    }

    class appmodel_apple_preferences : public ::testing::Test
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

    TEST_F(appmodel_apple_preferences, typed_round_trips_in_standard_defaults)
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

        // long + DateTime ride the C# string encodings.
        preferences::set_long(key("long1"), 9223372036854775806LL);
        EXPECT_EQ(preferences::get_long(key("long1"), 0), 9223372036854775806LL);

        const date_time stamp =
            std::chrono::sys_days{std::chrono::May / 7 / 2018} + std::chrono::hours{8} + std::chrono::minutes{30};
        preferences::set_date_time(key("datetime1"), stamp);
        EXPECT_EQ(preferences::get_date_time(key("datetime1"), date_time{}), stamp);
    }

    TEST_F(appmodel_apple_preferences, missing_keys_report_defaults_and_contains_key)
    {
        preferences::remove(key("string1"));
        EXPECT_EQ(preferences::get_string(key("string1"), std::nullopt), std::nullopt);
        EXPECT_EQ(preferences::get_string(key("string1"), "text"), "text");
        EXPECT_EQ(preferences::get_int(key("int1"), 5), 5);
        EXPECT_FALSE(preferences::contains_key(key("string1")));

        preferences::set_string(key("string1"), "value");
        EXPECT_TRUE(preferences::contains_key(key("string1")));
        preferences::set_string(key("string1"), std::nullopt); // the null set removes
        EXPECT_FALSE(preferences::contains_key(key("string1")));
    }

    TEST_F(appmodel_apple_preferences, shared_suite_is_distinct_and_clearable)
    {
        preferences::set_int(key("int1"), 1);
        preferences::set_int(key("int1"), 2, suite_name());
        EXPECT_EQ(preferences::get_int(key("int1"), 0), 1);
        EXPECT_EQ(preferences::get_int(key("int1"), 0, suite_name()), 2);

        preferences::clear(suite_name());
        EXPECT_EQ(preferences::get_int(key("int1"), 0, suite_name()), 0);
        EXPECT_EQ(preferences::get_int(key("int1"), 0), 1); // the standard domain survives
    }

    class appmodel_apple_secure_storage : public ::testing::Test
    {
    protected:
        void TearDown() override
        {
            secure_storage::remove("maui.port.w117.apple.secure1");
            secure_storage::remove("maui.port.w117.apple.secure2");
        }

        static std::optional<std::string> get(std::string_view key)
        {
            std::optional<std::string> result;
            secure_storage::get_async(key, [&result](const std::optional<std::string>& value) { result = value; });
            return result;
        }
    };

    // Saves_And_Loads + Saves_Same_Key_Twice + Non_Existent_Key_Returns_Null + Remove_Key, on the
    // REAL keychain.
    TEST_F(appmodel_apple_secure_storage, keychain_round_trip)
    {
        const std::string key = "maui.port.w117.apple.secure1";
        secure_storage::set_async(key, "data");
        EXPECT_EQ(get(key), "data");

        secure_storage::set_async(key, "data2"); // same key twice - the second value wins
        EXPECT_EQ(get(key), "data2");

        EXPECT_EQ(get("maui.port.w117.THIS_KEY_SHOULD_NOT_EXIST"), std::nullopt);

        EXPECT_TRUE(secure_storage::remove(key));
        EXPECT_EQ(get(key), std::nullopt);
        EXPECT_FALSE(secure_storage::remove(key));
    }

    TEST(appmodel_apple_app_info, real_surface_is_callable)
    {
        // The unbundled gtest process has no Info.plist - the bundle reads are "" (C#'s null).
        EXPECT_NO_THROW((void)app_info::package_name());
        EXPECT_NO_THROW((void)app_info::name());
        EXPECT_NO_THROW((void)app_info::build_string());
        EXPECT_EQ(app_info::version(), maui::devices::version_info::parse(app_info::version_string()));

        const app_theme theme = app_info::requested_theme();
        EXPECT_TRUE(theme == app_theme::light || theme == app_theme::dark || theme == app_theme::unspecified);
        EXPECT_EQ(app_info::packaging_model(), app_packaging_model::packaged);
        EXPECT_EQ(app_info::requested_layout_direction(), layout_direction::left_to_right);
    }

    TEST(appmodel_apple_file_system, directories_and_package_probe)
    {
        EXPECT_FALSE(maui::storage::file_system::cache_directory().empty());
        EXPECT_FALSE(maui::storage::file_system::app_data_directory().empty());

        // The unbundled binary's bundlePath has no Contents/Resources payload.
        EXPECT_FALSE(maui::storage::file_system::app_package_file_exists("maui_port_definitely_missing.txt"));
        EXPECT_THROW((void)maui::storage::file_system::open_app_package_file("maui_port_definitely_missing.txt"),
                     std::runtime_error);
    }

    // MainThread_Tests.IsOnMainThread / IsNotOnMainThread over the GCD facade.
    TEST(appmodel_apple_main_thread, is_main_thread_tracks_the_calling_thread)
    {
        EXPECT_TRUE(main_thread::is_main_thread()); // gtest main runs on the process main thread

        bool off_main = true;
        std::thread worker([&off_main] { off_main = main_thread::is_main_thread(); });
        worker.join();
        EXPECT_FALSE(off_main);

        // Posting from the main thread runs INLINE (the C# shared gate).
        bool ran = false;
        main_thread::begin_invoke_on_main_thread([&ran] { ran = true; });
        EXPECT_TRUE(ran);
    }

    TEST(appmodel_apple_launcher, can_open_queries_nsworkspace)
    {
        bool https_supported = false;
        launcher::can_open_async("https://example.com", [&https_supported](bool value) { https_supported = value; });
        EXPECT_TRUE(https_supported); // a browser handles https on every Mac

        bool bogus_supported = true;
        launcher::can_open_async("maui-port-no-such-scheme:probe",
                                 [&bogus_supported](bool value) { bogus_supported = value; });
        EXPECT_FALSE(bogus_supported);

        // TryOpen with an unsupported scheme reports false WITHOUT opening anything.
        bool try_result = true;
        launcher::try_open_async("maui-port-no-such-scheme:probe", [&try_result](bool value) { try_result = value; });
        EXPECT_FALSE(try_result);
    }

    // U17: the WebUtils.GetNativeUrl OriginalString->AbsoluteUri fallback. A URI with a literal space
    // in the authority is rejected by [NSURL URLWithString:] raw but parses once normalized (the space
    // -> %20), so the launcher must NOT return nil for it. Asserted at the get_native_url helper the
    // partials route through (testing the open path itself would launch a real app on the dev machine).
    TEST(appmodel_apple_launcher, get_native_url_normalizes_when_raw_parse_fails)
    {
        using maui::platform::apple_shared::get_native_url;

        // Sanity: the raw form really is unparseable, so the fallback is what rescues it.
        EXPECT_TRUE([NSURL URLWithString:@"https://exa mple.com/path"] == nil);

        NSURL* const url = get_native_url("https://exa mple.com/path");
        ASSERT_TRUE(url != nil);
        EXPECT_TRUE([[url scheme] isEqualToString:@"https"]);
        EXPECT_TRUE([[url absoluteString] isEqualToString:@"https://exa%20mple.com/path"]);

        // An already-valid URI is returned as-is (no spurious normalization - idempotent on %XX).
        NSURL* const plain = get_native_url("https://example.com/already/encoded%20ok");
        ASSERT_TRUE(plain != nil);
        EXPECT_TRUE([[plain absoluteString] isEqualToString:@"https://example.com/already/encoded%20ok"]);
    }

    // The suffix oracle: macOS app actions are the netstandard partial - everything throws; the
    // event accessors stay subscribable.
    TEST(appmodel_apple_app_actions, not_supported_on_macos)
    {
        EXPECT_THROW((void)app_actions::is_supported(), feature_not_supported);
        EXPECT_THROW(app_actions::get_async([](const std::vector<app_action>&) {}), feature_not_supported);
        EXPECT_THROW(app_actions::set_async({}), feature_not_supported);

        const auto token = app_actions::add_on_app_action([](const app_action&) {});
        EXPECT_TRUE(app_actions::remove_on_app_action(token));
    }

    // Permissions_Tests Check_Status/Request/Ensure_Declared: the desktop auto-grant stub.
    // A valid permission_status is any of the C# PermissionStatus enumerators the macOS
    // GetLocationStatus can yield (the helper never returns limited - that is iOS-only).
    bool is_valid_location_status(permission_status value)
    {
        switch (value)
        {
            case permission_status::unknown:
            case permission_status::denied:
            case permission_status::disabled:
            case permission_status::granted:
            case permission_status::restricted:
                return true;
            case permission_status::limited:
                return false;
        }
        return false;
    }

    TEST(appmodel_apple_permissions, location_queries_corelocation_others_auto_grant)
    {
        // NON-location permissions stay auto-granted (BasePlatformPermission), check and request.
        std::optional<permission_status> battery_status;
        permissions::check_status_async<permissions::battery>(
            [&battery_status](permission_status value) { battery_status = value; });
        EXPECT_EQ(battery_status, permission_status::granted);

        std::optional<permission_status> network_status;
        permissions::request_async<permissions::network_state>(
            [&network_status](permission_status value) { network_status = value; });
        EXPECT_EQ(network_status, permission_status::granted);

        // LocationAlways is an empty macOS partial - it keeps the base auto-grant (NOT routed
        // through CoreLocation, mirroring Permissions.macos.cs).
        std::optional<permission_status> location_always_status;
        permissions::check_status_async<permissions::location_always>(
            [&location_always_status](permission_status value) { location_always_status = value; });
        EXPECT_EQ(location_always_status, permission_status::granted);

        // macOS LocationWhenInUse overrides with real CoreLocation: the queried status is
        // system-dependent (NotDetermined/Denied in the unbundled gtest process), so assert it is
        // a VALID permission_status rather than a hardcoded grant. EnsureDeclared does NOT throw:
        // macOS only RECOMMENDS the usage key (a Console nudge), unlike the iOS Required gate.
        EXPECT_NO_THROW(permissions::ensure_declared<permissions::location_when_in_use>());
        std::optional<permission_status> location_status;
        permissions::check_status_async<permissions::location_when_in_use>(
            [&location_status](permission_status value) { location_status = value; });
        ASSERT_TRUE(location_status.has_value());
        EXPECT_TRUE(is_valid_location_status(location_status.value()));

        // RequestAsync routes location through the same query (no interactive prompt on macOS):
        // its result matches the check.
        std::optional<permission_status> requested_status;
        permissions::request_async<permissions::location_when_in_use>(
            [&requested_status](permission_status value) { requested_status = value; });
        ASSERT_TRUE(requested_status.has_value());
        EXPECT_EQ(requested_status.value(), location_status.value());

        EXPECT_NO_THROW(permissions::ensure_declared<permissions::battery>());
        EXPECT_FALSE(permissions::should_show_rationale<permissions::camera>());
    }
} // namespace
