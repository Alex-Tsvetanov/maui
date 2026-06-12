// permissions on the headless backend: the unconfigured fake backend mirrors Permissions'
// netstandard partial (every member of every permission type throws), and the fake-grantable
// seam runs the DeviceTests Permissions_Tests architecture device-free: staged check statuses,
// distinct request answers (the request flow that cannot be UI-driven in tests), the
// missing-manifest ensure_declared throw (permission_error), and the Android-only rationale
// flag. The generic surface (`permissions::check_status_async<permissions::battery>`) mirrors
// C#'s Permissions.CheckStatusAsync<Permissions.Battery>().

#include <memory>
#include <optional>

#include <gtest/gtest.h>

#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/permissions.hpp"

#include "src/platform/headless/essentials_appmodel_fakes.hpp"

namespace
{
    using namespace maui::application_model;

    class permissions_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            permissions::set_backend(nullptr);
        }
        void TearDown() override
        {
            permissions::set_backend(nullptr);
        }

        static std::shared_ptr<headless_permission_backend> install()
        {
            auto fake = std::make_shared<headless_permission_backend>();
            permissions::set_backend(fake);
            return fake;
        }

        template <class TPermission> static permission_status check_status()
        {
            std::optional<permission_status> result;
            permissions::check_status_async<TPermission>([&result](permission_status value) { result = value; });
            EXPECT_TRUE(result.has_value());
            return result.value_or(permission_status::unknown);
        }

        template <class TPermission> static permission_status request()
        {
            std::optional<permission_status> result;
            permissions::request_async<TPermission>([&result](permission_status value) { result = value; });
            EXPECT_TRUE(result.has_value());
            return result.value_or(permission_status::unknown);
        }
    };

    TEST_F(permissions_test, netstandard_mirror_throws_until_configured)
    {
        EXPECT_THROW(permissions::check_status_async<permissions::camera>([](permission_status) {}),
                     feature_not_supported);
        EXPECT_THROW(permissions::request_async<permissions::camera>([](permission_status) {}), feature_not_supported);
        EXPECT_THROW(permissions::ensure_declared<permissions::camera>(), feature_not_supported);
        EXPECT_THROW((void)permissions::should_show_rationale<permissions::camera>(), feature_not_supported);
    }

    // Check_Status (DeviceTests): staged statuses come back per kind.
    TEST_F(permissions_test, check_status_reports_staged_status)
    {
        auto fake = install();
        fake->set_status(permission_kind::battery, permission_status::granted);
        fake->set_status(permission_kind::network_state, permission_status::granted);
        fake->set_status(permission_kind::location_when_in_use, permission_status::denied);

        EXPECT_EQ(check_status<permissions::battery>(), permission_status::granted);
        EXPECT_EQ(check_status<permissions::network_state>(), permission_status::granted);
        EXPECT_EQ(check_status<permissions::location_when_in_use>(), permission_status::denied);
    }

    // Request (DeviceTests): without a distinct staged answer the request reports the status...
    TEST_F(permissions_test, request_falls_back_to_status)
    {
        auto fake = install();
        fake->set_status(permission_kind::battery, permission_status::granted);
        EXPECT_EQ(request<permissions::battery>(), permission_status::granted);
        EXPECT_EQ(fake->last_requested(), permission_kind::battery);
    }

    // ...and a staged request answer wins (the fake-grantable request flow).
    TEST_F(permissions_test, request_uses_staged_request_result)
    {
        auto fake = install();
        fake->set_status(permission_kind::camera, permission_status::denied);
        fake->set_request_result(permission_kind::camera, permission_status::granted);
        EXPECT_EQ(check_status<permissions::camera>(), permission_status::denied);
        EXPECT_EQ(request<permissions::camera>(), permission_status::granted);
    }

    // Ensure_Declared (DeviceTests): declared kinds pass; a missing manifest entry throws
    // PermissionException -> permission_error.
    TEST_F(permissions_test, ensure_declared)
    {
        auto fake = install();
        fake->set_status(permission_kind::battery, permission_status::granted);
        EXPECT_NO_THROW(permissions::ensure_declared<permissions::battery>());

        fake->set_undeclared(permission_kind::location_when_in_use);
        EXPECT_THROW(permissions::ensure_declared<permissions::location_when_in_use>(), permission_error);
    }

    // ShouldShowRationale: false unless staged (the Android-only behavior).
    TEST_F(permissions_test, should_show_rationale)
    {
        auto fake = install();
        fake->set_status(permission_kind::camera, permission_status::denied);
        EXPECT_FALSE(permissions::should_show_rationale<permissions::camera>());

        fake->set_should_show_rationale(permission_kind::camera);
        EXPECT_TRUE(permissions::should_show_rationale<permissions::camera>());
    }
} // namespace
