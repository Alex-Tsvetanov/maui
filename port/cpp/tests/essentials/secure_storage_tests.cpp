// secure_storage on the headless backend. The unconfigured fake mirrors SecureStorage's
// netstandard partial (Essentials.UnitTests SecureStorage_Tests: get/set fail); the shared-
// partial key validation throws std::invalid_argument for blank keys on EVERY backend; the
// configured fake runs the DeviceTests SecureStorage_Tests behavior suite (saves/loads, saves
// the same key twice, missing key reads null, remove, remove-all). The IPlatformSecureStorage knob
// (DefaultAccessible + the accessible-taking SetAsync, from SecureStorage.ios.tvos.watchos.macos.cs)
// is verified through the fake's accessible_for() seam: the default is after_first_unlock, set
// persists, the overload overrides for one op, and a changed default flows to later plain sets.

#include <array>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>
#include <string_view>
#include <utility>

#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/secure_accessible.hpp"
#include "maui/essentials/secure_storage.hpp"

#include "src/platform/headless/essentials_appmodel_fakes.hpp"

namespace
{
    using namespace maui::storage;
    using maui::application_model::feature_not_supported;

    class secure_storage_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            secure_storage::set_default(nullptr);
        }
        void TearDown() override
        {
            secure_storage::set_default(nullptr);
        }

        static std::shared_ptr<headless_secure_storage> install_configured()
        {
            auto fake = std::make_shared<headless_secure_storage>();
            fake->configure();
            secure_storage::set_default(fake);
            return fake;
        }

        // The DeviceTests read helper: GetAsync completes inline on this backend.
        static std::optional<std::string> get(std::string_view key)
        {
            std::optional<std::string> result;
            bool completed = false;
            secure_storage::get_async(key, [&](const std::optional<std::string>& value) {
                result = value;
                completed = true;
            });
            EXPECT_TRUE(completed);
            return result;
        }
    };

    // SecureStorage_LoadAsync_Fail_On_NetStandard / SecureStorage_SaveAsync_Fail_On_NetStandard.
    TEST_F(secure_storage_test, netstandard_mirror_throws_until_configured)
    {
        EXPECT_THROW(secure_storage::get_async("key", [](const std::optional<std::string>&) {}), feature_not_supported);
        EXPECT_THROW(secure_storage::set_async("key", "data"), feature_not_supported);
        EXPECT_THROW((void)secure_storage::remove("key"), feature_not_supported);
        EXPECT_THROW(secure_storage::remove_all(), feature_not_supported);
    }

    // The shared partial's IsNullOrWhiteSpace(key) gate (ArgumentNullException).
    TEST_F(secure_storage_test, blank_key_throws_invalid_argument)
    {
        install_configured();
        EXPECT_THROW(secure_storage::get_async("", [](const std::optional<std::string>&) {}), std::invalid_argument);
        EXPECT_THROW(secure_storage::get_async("   ", [](const std::optional<std::string>&) {}), std::invalid_argument);
        EXPECT_THROW(secure_storage::set_async("", "value"), std::invalid_argument);
        EXPECT_THROW(secure_storage::set_async(" \t ", "value"), std::invalid_argument);
    }

    // Saves_And_Loads (the funny-characters keys included).
    TEST_F(secure_storage_test, saves_and_loads)
    {
        install_configured();
        const std::array<std::pair<std::string_view, std::string_view>, 3> cases = {{
            {"test.txt", "data"},
            {"noextension", "data2"},
            {"funny*&$%@!._/\\chars", "data3"},
        }};
        for (const auto& [key, data] : cases)
        {
            secure_storage::set_async(key, data);
            EXPECT_EQ(get(key), data);
        }
    }

    // Saves_Same_Key_Twice: the second value wins.
    TEST_F(secure_storage_test, saves_same_key_twice)
    {
        install_configured();
        secure_storage::set_async("test.txt", "data1");
        secure_storage::set_async("test.txt", "data2");
        EXPECT_EQ(get("test.txt"), "data2");
    }

    // Non_Existent_Key_Returns_Null.
    TEST_F(secure_storage_test, non_existent_key_returns_null)
    {
        install_configured();
        EXPECT_EQ(get("THIS_KEY_SHOULD_NOT_EXIST"), std::nullopt);
    }

    // Remove_Key: true when removed, and the value reads null afterwards; false for a missing key.
    TEST_F(secure_storage_test, remove_key)
    {
        install_configured();
        secure_storage::set_async("KEY_TO_REMOVE1", "Irrelevant Data");
        EXPECT_TRUE(secure_storage::remove("KEY_TO_REMOVE1"));
        EXPECT_EQ(get("KEY_TO_REMOVE1"), std::nullopt);
        EXPECT_FALSE(secure_storage::remove("KEY_TO_REMOVE1"));
    }

    // Remove_All_Keys.
    TEST_F(secure_storage_test, remove_all_keys)
    {
        install_configured();
        secure_storage::set_async("KEYS_TO_REMOVEA1", "Irrelevant Data");
        secure_storage::set_async("KEYS_TO_REMOVEA2", "Irrelevant Data");
        secure_storage::remove_all();
        EXPECT_EQ(get("KEYS_TO_REMOVEA1"), std::nullopt);
        EXPECT_EQ(get("KEYS_TO_REMOVEA2"), std::nullopt);
    }

    // C# `DefaultAccessible { get; set; } = SecAccessible.AfterFirstUnlock;` - the default value.
    TEST_F(secure_storage_test, default_accessible_is_after_first_unlock)
    {
        install_configured();
        EXPECT_EQ(secure_storage::get_default_accessible(), secure_accessible::after_first_unlock);
    }

    // DefaultAccessible setter persists.
    TEST_F(secure_storage_test, set_default_accessible_persists)
    {
        install_configured();
        secure_storage::set_default_accessible(secure_accessible::when_unlocked_this_device_only);
        EXPECT_EQ(secure_storage::get_default_accessible(), secure_accessible::when_unlocked_this_device_only);
    }

    // SetAsync(key, value, accessible) uses the explicit value, not the default.
    TEST_F(secure_storage_test, set_async_with_accessible_overrides_default)
    {
        auto fake = install_configured();
        secure_storage::set_async_with_accessible("k", "v", secure_accessible::when_passcode_set_this_device_only);
        EXPECT_EQ(get("k"), "v");
        EXPECT_EQ(fake->accessible_for("k"), secure_accessible::when_passcode_set_this_device_only);
        // The default is untouched by the per-op override.
        EXPECT_EQ(secure_storage::get_default_accessible(), secure_accessible::after_first_unlock);
    }

    // After changing the default, a plain SetAsync records the new default.
    TEST_F(secure_storage_test, plain_set_async_uses_current_default)
    {
        auto fake = install_configured();
        secure_storage::set_async("a", "1");
        EXPECT_EQ(fake->accessible_for("a"), secure_accessible::after_first_unlock);

        secure_storage::set_default_accessible(secure_accessible::when_unlocked);
        secure_storage::set_async("b", "2");
        EXPECT_EQ(fake->accessible_for("b"), secure_accessible::when_unlocked);
        // The earlier key keeps the accessible it was stored with.
        EXPECT_EQ(fake->accessible_for("a"), secure_accessible::after_first_unlock);
    }

    // The accessible-taking SetAsync re-runs the shared blank-key gate (C# SetAsync throws first).
    TEST_F(secure_storage_test, set_async_with_accessible_validates_key)
    {
        install_configured();
        EXPECT_THROW(secure_storage::set_async_with_accessible("", "v", secure_accessible::when_unlocked),
                     std::invalid_argument);
        EXPECT_THROW(secure_storage::set_async_with_accessible("  ", "v", secure_accessible::when_unlocked),
                     std::invalid_argument);
    }
} // namespace
