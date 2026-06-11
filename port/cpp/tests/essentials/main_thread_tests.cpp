// main_thread on the headless backend: the unconfigured fake mirrors MainThread's netstandard
// partial (both members throw until a custom implementation is configured - the
// SetCustomImplementation slot), and the configured fake proves the C# shared gate: on the main
// thread the action runs INLINE; off it, the action is posted to the platform queue (drained by
// run_pending(), the headless main-loop stand-in).

#include <memory>

#include <gtest/gtest.h>

#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/main_thread.hpp"

#include "src/platform/headless/essentials_appmodel_fakes.hpp"

namespace
{
    using namespace maui::application_model;

    class main_thread_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            main_thread::set_current(nullptr);
        }
        void TearDown() override
        {
            main_thread::set_current(nullptr);
        }
    };

    TEST_F(main_thread_test, netstandard_mirror_throws_until_configured)
    {
        EXPECT_THROW((void)main_thread::is_main_thread(), feature_not_supported);
        EXPECT_THROW(main_thread::begin_invoke_on_main_thread([] {}), feature_not_supported);
    }

    // IsOnMainThread (the configured analog of Utils.OnMainThread + Assert.True).
    TEST_F(main_thread_test, is_main_thread_reports_configured_value)
    {
        auto fake = std::make_shared<headless_main_thread>();
        fake->set_is_main_thread(true);
        main_thread::set_current(fake);
        EXPECT_TRUE(main_thread::is_main_thread());

        fake->set_is_main_thread(false); // IsNotOnMainThread
        EXPECT_FALSE(main_thread::is_main_thread());
    }

    // BeginInvokeOnMainThread runs INLINE when already on the main thread (the C# shared gate).
    TEST_F(main_thread_test, begin_invoke_runs_inline_on_main_thread)
    {
        auto fake = std::make_shared<headless_main_thread>();
        fake->set_is_main_thread(true);
        main_thread::set_current(fake);

        bool ran = false;
        main_thread::begin_invoke_on_main_thread([&ran] { ran = true; });
        EXPECT_TRUE(ran);
        EXPECT_EQ(fake->pending_count(), 0U); // never reached the platform queue
    }

    // Off the main thread the action is POSTED, not run inline.
    TEST_F(main_thread_test, begin_invoke_posts_when_off_main_thread)
    {
        auto fake = std::make_shared<headless_main_thread>();
        fake->set_is_main_thread(false);
        main_thread::set_current(fake);

        bool ran = false;
        main_thread::begin_invoke_on_main_thread([&ran] { ran = true; });
        EXPECT_FALSE(ran);
        EXPECT_EQ(fake->pending_count(), 1U);

        fake->run_pending();
        EXPECT_TRUE(ran);
        EXPECT_EQ(fake->pending_count(), 0U);
    }
} // namespace
