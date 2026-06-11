// browser on the headless backend: the unconfigured fake mirrors Browser's netstandard partial
// (Essentials.UnitTests Browser_Tests: every open overload fails), the facade carries the same
// `new Uri(uri)` format gate as the launcher, and the configured fake records the uri + options
// each overload resolves to (plain -> defaults, launch-mode -> mode only, options -> verbatim).

#include <memory>
#include <stdexcept>

#include <gtest/gtest.h>

#include "maui/essentials/browser.hpp"
#include "maui/essentials/feature_not_supported.hpp"
#include "maui/graphics/color.hpp"

#include "src/platform/headless/essentials_appmodel_fakes.hpp"

namespace
{
    using namespace maui::application_model;

    class browser_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            browser::set_default(nullptr);
        }
        void TearDown() override
        {
            browser::set_default(nullptr);
        }

        static std::shared_ptr<headless_browser> install_configured(bool open_result = true)
        {
            auto fake = std::make_shared<headless_browser>();
            fake->set_open_result(open_result);
            browser::set_default(fake);
            return fake;
        }
    };

    // Open_Uri_String_NetStandard + Open_Uri_String_Launch_NetStandard.
    TEST_F(browser_test, netstandard_mirror_throws_until_configured)
    {
        EXPECT_THROW(browser::open_async("http://xamarin.com", [](bool) {}), feature_not_supported);
        EXPECT_THROW(browser::open_async("http://xamarin.com", browser_launch_mode::system_preferred, [](bool) {}),
                     feature_not_supported);
        EXPECT_THROW(browser::open_async("http://xamarin.com", browser_launch_options{}, [](bool) {}),
                     feature_not_supported);
    }

    TEST_F(browser_test, malformed_uri_throws_invalid_argument)
    {
        install_configured();
        EXPECT_THROW(browser::open_async("not a uri", [](bool) {}), std::invalid_argument);
    }

    // The plain overload opens with default options (SystemPreferred / Default / None).
    TEST_F(browser_test, open_uses_default_options)
    {
        auto fake = install_configured();
        bool result = false;
        browser::open_async("http://xamarin.com", [&result](bool value) { result = value; });
        EXPECT_TRUE(result);
        EXPECT_EQ(fake->last_uri(), "http://xamarin.com");
        ASSERT_TRUE(fake->last_options().has_value());
        EXPECT_EQ(fake->last_options()->launch_mode, browser_launch_mode::system_preferred);
        EXPECT_EQ(fake->last_options()->title_mode, browser_title_mode::default_);
        EXPECT_EQ(fake->last_options()->flags, browser_launch_flags::none);
        EXPECT_FALSE(fake->last_options()->preferred_toolbar_color.has_value());
        EXPECT_FALSE(fake->last_options()->preferred_control_color.has_value());
    }

    // The launch-mode overload wraps the mode into options (BrowserExtensions.OpenAsync).
    TEST_F(browser_test, open_with_launch_mode)
    {
        auto fake = install_configured();
        browser::open_async("https://example.com", browser_launch_mode::external, [](bool) {});
        ASSERT_TRUE(fake->last_options().has_value());
        EXPECT_EQ(fake->last_options()->launch_mode, browser_launch_mode::external);
    }

    // The options overload passes everything through; has_flag mirrors BrowserLaunchOptions.HasFlag.
    TEST_F(browser_test, open_with_full_options)
    {
        auto fake = install_configured(false);
        browser_launch_options options;
        options.launch_mode = browser_launch_mode::system_preferred;
        options.title_mode = browser_title_mode::show;
        options.flags = browser_launch_flags::present_as_page_sheet | browser_launch_flags::launch_adjacent;
        options.preferred_toolbar_color = maui::graphics::color(1.0F, 0.0F, 0.0F);

        bool result = true;
        browser::open_async("https://example.com", options, [&result](bool value) { result = value; });
        EXPECT_FALSE(result); // the staged open answer

        ASSERT_TRUE(fake->last_options().has_value());
        EXPECT_TRUE(fake->last_options()->has_flag(browser_launch_flags::present_as_page_sheet));
        EXPECT_TRUE(fake->last_options()->has_flag(browser_launch_flags::launch_adjacent));
        EXPECT_FALSE(fake->last_options()->has_flag(browser_launch_flags::present_as_form_sheet));
        EXPECT_EQ(fake->last_options()->title_mode, browser_title_mode::show);
        EXPECT_TRUE(fake->last_options()->preferred_toolbar_color.has_value());
    }
} // namespace
