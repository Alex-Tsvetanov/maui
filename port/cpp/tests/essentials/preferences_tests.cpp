// preferences on the headless backend. The unconfigured fake mirrors Preferences' netstandard
// partial (every member throws - Essentials.UnitTests Preferences_Tests); the configured fake
// runs the DeviceTests Preferences_Tests behavior suite (typed set/get round-trips, the shared
// container split, null-string set removes, remove/clear/contains-key, DateTime round-trip) plus
// GetPrivatePreferencesSharedName's container-name shape.

#include <chrono>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>

#include <gtest/gtest.h>

#include "maui/essentials/app_info.hpp"
#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/preferences.hpp"

#include "src/platform/headless/essentials_appmodel_fakes.hpp"

namespace
{
    using namespace maui::storage;
    using maui::application_model::feature_not_supported;

    constexpr std::string_view shared_name_test_data = "Shared";

    class preferences_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            preferences::set_default(nullptr);
        }
        void TearDown() override
        {
            preferences::set_default(nullptr);
            maui::application_model::app_info::set_current(nullptr);
        }

        // Install a configured fake and run the suite body against the facade.
        static std::shared_ptr<headless_preferences> install_configured()
        {
            auto fake = std::make_shared<headless_preferences>();
            fake->configure();
            preferences::set_default(fake);
            return fake;
        }
    };

    // Preferences_Tests (UnitTests): every facade member fails on the netstandard mirror.
    TEST_F(preferences_test, netstandard_mirror_throws_until_configured)
    {
        EXPECT_THROW(preferences::set_string("anything", "fails"), feature_not_supported);
        EXPECT_THROW((void)preferences::get_string("anything", "fails"), feature_not_supported);
        EXPECT_THROW((void)preferences::get_string("anything", "fails", "shared"), feature_not_supported);
        EXPECT_THROW((void)preferences::contains_key("anything"), feature_not_supported);
        EXPECT_THROW((void)preferences::contains_key("anything", "shared"), feature_not_supported);
        EXPECT_THROW(preferences::remove("anything"), feature_not_supported);
        EXPECT_THROW(preferences::remove("anything", "shared"), feature_not_supported);
        EXPECT_THROW(preferences::clear(), feature_not_supported);
        EXPECT_THROW(preferences::clear("shared"), feature_not_supported);
    }

    // Set_Get_String (default + shared container).
    TEST_F(preferences_test, set_get_string)
    {
        install_configured();
        for (const std::string_view shared : {std::string_view{}, shared_name_test_data})
        {
            preferences::set_string("string1", "TEST", shared);
            EXPECT_EQ(preferences::get_string("string1", std::nullopt, shared), "TEST");
        }
    }

    // Set_Set_Null_Get_String: a null set removes the key.
    TEST_F(preferences_test, set_set_null_get_string)
    {
        install_configured();
        for (const std::string_view shared : {std::string_view{}, shared_name_test_data})
        {
            preferences::set_string("string1", "TEST", shared);
            preferences::set_string("string1", std::nullopt, shared);
            EXPECT_EQ(preferences::get_string("string1", std::nullopt, shared), std::nullopt);
            EXPECT_FALSE(preferences::contains_key("string1", shared));
        }
    }

    // Set_Get_Int / Set_Get_Long / Set_Get_Float / Set_Get_Double / Set_Get_Bool.
    TEST_F(preferences_test, set_get_numeric_and_bool)
    {
        install_configured();
        for (const std::string_view shared : {std::string_view{}, shared_name_test_data})
        {
            preferences::set_int("int1", std::numeric_limits<std::int32_t>::max() - 1, shared);
            EXPECT_EQ(preferences::get_int("int1", 0, shared), std::numeric_limits<std::int32_t>::max() - 1);
            preferences::set_int("sint1", std::numeric_limits<std::int32_t>::min() + 1, shared);
            EXPECT_EQ(preferences::get_int("sint1", 0, shared), std::numeric_limits<std::int32_t>::min() + 1);

            preferences::set_long("long1", std::numeric_limits<std::int64_t>::max() - 1, shared);
            EXPECT_EQ(preferences::get_long("long1", 0, shared), std::numeric_limits<std::int64_t>::max() - 1);
            preferences::set_long("slong1", std::numeric_limits<std::int64_t>::min() + 1, shared);
            EXPECT_EQ(preferences::get_long("slong1", 0, shared), std::numeric_limits<std::int64_t>::min() + 1);

            preferences::set_float("float1", std::numeric_limits<float>::max() - 1, shared);
            EXPECT_EQ(preferences::get_float("float1", 0.0F, shared), std::numeric_limits<float>::max() - 1);

            preferences::set_double("double1", std::numeric_limits<double>::max() - 1, shared);
            EXPECT_EQ(preferences::get_double("double1", 0.0, shared), std::numeric_limits<double>::max() - 1);

            preferences::set_bool("bool1", true, shared);
            EXPECT_TRUE(preferences::get_bool("bool1", false, shared));
        }
    }

    // Set_Get_DateTime + DateTimePreservesKind's instant round-trip.
    TEST_F(preferences_test, set_get_date_time)
    {
        install_configured();
        const date_time test_date =
            std::chrono::sys_days{std::chrono::May / 7 / 2018} + std::chrono::hours{8} + std::chrono::minutes{30};
        for (const std::string_view shared : {std::string_view{}, shared_name_test_data})
        {
            preferences::set_date_time("datetime1", test_date, shared);
            EXPECT_EQ(preferences::get_date_time("datetime1", date_time{}, shared), test_date);
        }
    }

    // Remove.
    TEST_F(preferences_test, remove)
    {
        install_configured();
        for (const std::string_view shared : {std::string_view{}, shared_name_test_data})
        {
            preferences::set_string("RemoveKey1", "value", shared);
            EXPECT_EQ(preferences::get_string("RemoveKey1", std::nullopt, shared), "value");
            preferences::remove("RemoveKey1", shared);
            EXPECT_EQ(preferences::get_string("RemoveKey1", std::nullopt, shared), std::nullopt);
        }
    }

    // Remove_Get_* : a removed key reports the caller's default for every type.
    TEST_F(preferences_test, remove_get_reports_defaults)
    {
        install_configured();
        for (const std::string_view shared : {std::string_view{}, shared_name_test_data})
        {
            preferences::remove("RemoveGetKey1", shared);
            EXPECT_TRUE(preferences::get_bool("RemoveGetKey1", true, shared));
            EXPECT_FALSE(preferences::get_bool("RemoveGetKey1", false, shared));
            EXPECT_EQ(preferences::get_int("RemoveGetKey1", 5, shared), 5);
            EXPECT_EQ(preferences::get_int("RemoveGetKey1", 0, shared), 0);
            EXPECT_EQ(preferences::get_long("RemoveGetKey1", 5, shared), 5);
            EXPECT_EQ(preferences::get_float("RemoveGetKey1", 5.0F, shared), 5.0F);
            EXPECT_EQ(preferences::get_double("RemoveGetKey1", 5.0, shared), 5.0);
            EXPECT_EQ(preferences::get_string("RemoveGetKey1", "text", shared), "text");
            EXPECT_EQ(preferences::get_string("RemoveGetKey1", std::nullopt, shared), std::nullopt);
        }
    }

    // Clear.
    TEST_F(preferences_test, clear)
    {
        install_configured();
        for (const std::string_view shared : {std::string_view{}, shared_name_test_data})
        {
            preferences::set_string("ClearKey1", "value", shared);
            preferences::set_int("ClearKey2", 2, shared);
            EXPECT_EQ(preferences::get_int("ClearKey2", 0, shared), 2);
            preferences::clear(shared);
            EXPECT_NE(preferences::get_string("ClearKey1", std::nullopt, shared), "value");
            EXPECT_NE(preferences::get_int("ClearKey2", 0, shared), 2);
        }
    }

    // Does_ContainsKey / Not_ContainsKey.
    TEST_F(preferences_test, contains_key)
    {
        install_configured();
        for (const std::string_view shared : {std::string_view{}, shared_name_test_data})
        {
            preferences::set_string("DoesContainsKey1", "One", shared);
            EXPECT_TRUE(preferences::contains_key("DoesContainsKey1", shared));
            preferences::remove("NotContainsKey1", shared);
            EXPECT_FALSE(preferences::contains_key("NotContainsKey1", shared));
        }
    }

    // The default container and a named container are distinct stores.
    TEST_F(preferences_test, shared_container_is_distinct)
    {
        install_configured();
        preferences::set_int("split", 1);
        preferences::set_int("split", 2, shared_name_test_data);
        EXPECT_EQ(preferences::get_int("split", 0), 1);
        EXPECT_EQ(preferences::get_int("split", 0, shared_name_test_data), 2);
        preferences::clear(shared_name_test_data);
        EXPECT_EQ(preferences::get_int("split", 0), 1);
        EXPECT_EQ(preferences::get_int("split", 0, shared_name_test_data), 0);
    }

    // GetPrivatePreferencesSharedName: "{PackageName}.microsoft.maui.essentials.{feature}".
    TEST_F(preferences_test, private_preferences_shared_name_shape)
    {
        auto fake_app_info = std::make_shared<maui::application_model::headless_app_info>();
        fake_app_info->set_package_name("com.maui.port");
        maui::application_model::app_info::set_current(fake_app_info);

        EXPECT_EQ(detail::private_preferences_shared_name("versiontracking"),
                  "com.maui.port.microsoft.maui.essentials.versiontracking");
    }

    // set_default(nullptr) restores the lazy (netstandard-mirroring) default.
    TEST_F(preferences_test, set_default_null_restores_lazy_default)
    {
        install_configured();
        preferences::set_string("custom", "value");
        EXPECT_EQ(preferences::get_string("custom", std::nullopt), "value");

        preferences::set_default(nullptr);
        EXPECT_THROW((void)preferences::get_string("custom", std::nullopt), feature_not_supported);
    }
} // namespace
