// screenshot on the headless backend. The C# DeviceTests Screenshot_Tests are on-device only; this
// suite covers the cross-platform contract via the fake: the netstandard/macos mirror (both members
// throw), the facade's CaptureAsync feature-support gate (FeatureNotSupportedException when
// unsupported), and a configured fake producing a canned result whose width/height/stream are read
// through the IScreenshotResult surface.

#include <cstddef>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/screenshot.hpp"

#include "src/platform/headless/essentials_fakes.hpp"

namespace
{
    using namespace maui::media;
    using maui::application_model::feature_not_supported;

    std::vector<std::byte> make_bytes()
    {
        return {std::byte{0x89}, std::byte{'P'}, std::byte{'N'}, std::byte{'G'}};
    }

    class screenshot_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            screenshot::set_default(nullptr);
        }
        void TearDown() override
        {
            screenshot::set_default(nullptr);
        }
    };

    // The netstandard/macos mirror: IsCaptureSupported throws, and the facade CaptureAsync gate
    // throws because is_capture_supported() throws before the (false) comparison.
    TEST_F(screenshot_test, netstandard_mirror_throws_until_configured)
    {
        EXPECT_THROW((void)screenshot::is_capture_supported(), feature_not_supported);
        EXPECT_THROW(screenshot::capture_async([](const std::shared_ptr<i_screenshot_result>&) {}),
                     feature_not_supported);
    }

    // When capture is configured-unsupported, the facade throws FeatureNotSupportedException.
    TEST_F(screenshot_test, capture_gate_throws_when_unsupported)
    {
        auto fake = std::make_shared<headless_screenshot>();
        fake->set_is_capture_supported(false);
        screenshot::set_default(fake);

        EXPECT_FALSE(screenshot::is_capture_supported());
        EXPECT_THROW(screenshot::capture_async([](const std::shared_ptr<i_screenshot_result>&) {}),
                     feature_not_supported);
    }

    TEST_F(screenshot_test, configured_fake_captures_result)
    {
        auto fake = std::make_shared<headless_screenshot>();
        fake->set_is_capture_supported(true);
        fake->set_result(320, 480, make_bytes());
        screenshot::set_default(fake);

        EXPECT_TRUE(screenshot::is_capture_supported());

        std::shared_ptr<i_screenshot_result> result;
        screenshot::capture_async([&](const std::shared_ptr<i_screenshot_result>& r) { result = r; });
        ASSERT_NE(result, nullptr);
        EXPECT_EQ(result->width(), 320);
        EXPECT_EQ(result->height(), 480);

        std::vector<std::byte> read;
        result->open_read_async(screenshot_format::png, 100,
                                [&](const std::vector<std::byte>& bytes) { read = bytes; });
        EXPECT_EQ(read, make_bytes());
    }
} // namespace
