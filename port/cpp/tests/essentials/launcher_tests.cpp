// launcher on the headless backend: the unconfigured fake mirrors Launcher's netstandard partial
// (Essentials.UnitTests Launcher_Tests: can-open/open fail), the facade's string entry points
// carry the C# `new Uri(uri)` format gate (std::invalid_argument for malformed URIs), and the
// configured fake records what was opened with try-open composing can-open + open exactly like
// the platform partials.

#include <memory>
#include <stdexcept>

#include <gtest/gtest.h>
#include <string_view>

#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/launcher.hpp"

#include "src/platform/headless/essentials_appmodel_fakes.hpp"

namespace
{
    using namespace maui::application_model;

    class launcher_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            launcher::set_default(nullptr);
        }
        void TearDown() override
        {
            launcher::set_default(nullptr);
        }

        static std::shared_ptr<headless_launcher> install_configured(bool can_open)
        {
            auto fake = std::make_shared<headless_launcher>();
            fake->set_can_open(can_open);
            launcher::set_default(fake);
            return fake;
        }

        static bool result_of(void (*entry)(std::string_view, launch_callback), std::string_view uri)
        {
            bool result = false;
            bool completed = false;
            entry(uri, [&](bool value) {
                result = value;
                completed = true;
            });
            EXPECT_TRUE(completed);
            return result;
        }
    };

    // CanOpen/Open_String_NetStandard.
    TEST_F(launcher_test, netstandard_mirror_throws_until_configured)
    {
        EXPECT_THROW(launcher::can_open_async("http://www.xamarin.com", [](bool) {}), feature_not_supported);
        EXPECT_THROW(launcher::open_async("http://www.xamarin.com", [](bool) {}), feature_not_supported);
        EXPECT_THROW(launcher::try_open_async("http://www.xamarin.com", [](bool) {}), feature_not_supported);
    }

    // The `new Uri(uri)` format gate of the C# string overloads.
    TEST_F(launcher_test, malformed_uri_throws_invalid_argument)
    {
        install_configured(true);
        EXPECT_THROW(launcher::can_open_async("", [](bool) {}), std::invalid_argument);
        EXPECT_THROW(launcher::open_async("not a uri", [](bool) {}), std::invalid_argument);
        EXPECT_THROW(launcher::try_open_async("1http://nope", [](bool) {}), std::invalid_argument);
        EXPECT_THROW(launcher::open_async(":missing-scheme", [](bool) {}), std::invalid_argument);
    }

    // The DeviceTests scheme set is accepted by the format gate.
    TEST_F(launcher_test, device_test_uris_pass_validation)
    {
        install_configured(true);
        for (const std::string_view uri :
             {"http://www.example.com", "http://example.com/?query=blah", "https://example.com/?query=blah",
              "mailto://someone@microsoft.com", "mailto://someone@microsoft.com?subject=test", "tel:+1 555 010 9999",
              "sms:5550109999"})
        {
            EXPECT_TRUE(result_of(&launcher::can_open_async, uri)) << uri;
        }
    }

    // U17: a URI whose raw form NSURL cannot parse (a literal space) but whose normalized form can
    // (the WebUtils.GetNativeUrl OriginalString->AbsoluteUri fallback the apple/ios partials now
    // route through). The format gate is unaffected here - it accepts the space-bearing URI and the
    // headless fake never touches NSURL, so the gate-then-fake path stays green. The real fallback is
    // exercised by the apple/ios appmodel suites.
    TEST_F(launcher_test, normalizable_uri_passes_format_gate)
    {
        auto fake = install_configured(true);
        EXPECT_TRUE(result_of(&launcher::can_open_async, "https://example.com/a b"));
        EXPECT_EQ(fake->last_queried_uri(), "https://example.com/a b");
    }

    TEST_F(launcher_test, can_open_reports_staged_answer)
    {
        auto fake = install_configured(false);
        EXPECT_FALSE(result_of(&launcher::can_open_async, "myapp://nothing"));
        EXPECT_EQ(fake->last_queried_uri(), "myapp://nothing");
    }

    TEST_F(launcher_test, open_records_uri)
    {
        auto fake = install_configured(true);
        EXPECT_TRUE(result_of(&launcher::open_async, "http://www.example.com"));
        ASSERT_EQ(fake->opened_uris().size(), 1U);
        EXPECT_EQ(fake->opened_uris().front(), "http://www.example.com");
    }

    // TryOpen: opens only when the scheme can be opened (no open attempt otherwise).
    TEST_F(launcher_test, try_open_composes_can_open_and_open)
    {
        auto fake = install_configured(true);
        EXPECT_TRUE(result_of(&launcher::try_open_async, "myapp://test"));
        EXPECT_EQ(fake->opened_uris().size(), 1U);

        fake->set_can_open(false);
        EXPECT_FALSE(result_of(&launcher::try_open_async, "myapp://test2"));
        EXPECT_EQ(fake->opened_uris().size(), 1U); // unchanged - nothing was opened
    }
} // namespace
