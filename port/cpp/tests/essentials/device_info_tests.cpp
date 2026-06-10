// device_info on the headless backend: the unconfigured fake mirrors DeviceInfo's netstandard
// partial (model/manufacturer/name/version throw; platform/idiom/device-type unknown), the
// configured fake exposes the full surface through the static facade, and set_current is the
// DeviceInfo.SetCurrent seam (nullptr restores the lazy default).

#include <memory>

#include <gtest/gtest.h>

#include "maui/essentials/device_info.hpp"
#include "maui/essentials/feature_not_supported.hpp"

#include "src/platform/headless/essentials_fakes.hpp"

namespace
{
    using namespace maui::devices;
    using maui::application_model::feature_not_supported;

    class device_info_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            device_info::set_current(nullptr);
        }
        void TearDown() override
        {
            device_info::set_current(nullptr);
        }
    };

    TEST_F(device_info_test, netstandard_mirror_throws_until_configured)
    {
        EXPECT_THROW((void)device_info::model(), feature_not_supported);
        EXPECT_THROW((void)device_info::manufacturer(), feature_not_supported);
        EXPECT_THROW((void)device_info::name(), feature_not_supported);
        EXPECT_THROW((void)device_info::version_string(), feature_not_supported);
        EXPECT_THROW((void)device_info::version(), feature_not_supported); // parses VersionString
        EXPECT_TRUE(device_info::platform() == device_platform::unknown());
        EXPECT_TRUE(device_info::idiom() == device_idiom::unknown());
        EXPECT_EQ(device_info::device_type(), device_type::unknown);
    }

    TEST_F(device_info_test, configured_fake_flows_through_facade)
    {
        auto fake = std::make_shared<headless_device_info>();
        fake->set_model("HeadlessBook Pro");
        fake->set_manufacturer("MAUI C++");
        fake->set_name("unit-test device");
        fake->set_version_string("14.2.1");
        fake->set_platform(device_platform::create("Headless"));
        fake->set_idiom(device_idiom::desktop());
        fake->set_device_type(device_type::virtual_);
        device_info::set_current(fake);

        EXPECT_EQ(device_info::model(), "HeadlessBook Pro");
        EXPECT_EQ(device_info::manufacturer(), "MAUI C++");
        EXPECT_EQ(device_info::name(), "unit-test device");
        EXPECT_EQ(device_info::version_string(), "14.2.1");
        EXPECT_EQ(device_info::version(), (version_info{14, 2, 1, -1}));
        EXPECT_TRUE(device_info::platform() == device_platform::create("Headless"));
        EXPECT_TRUE(device_info::idiom() == device_idiom::desktop());
        EXPECT_EQ(device_info::device_type(), device_type::virtual_);
    }

    TEST_F(device_info_test, set_current_null_restores_lazy_default)
    {
        auto fake = std::make_shared<headless_device_info>();
        fake->set_model("custom");
        device_info::set_current(fake);
        EXPECT_EQ(device_info::model(), "custom");

        device_info::set_current(nullptr);
        EXPECT_THROW((void)device_info::model(), feature_not_supported);
    }
} // namespace
