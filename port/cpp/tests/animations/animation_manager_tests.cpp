// Tests for maui::animations::animation_manager + the deterministic manual_ticker, capturing the
// behavior of src/Core/src/Animations/AnimationManager.cs (auto start/stop of the ticker, removal of
// finished animations, the system-disabled force-finish path) the way the C# Controls suites drive
// it: a manager whose adjust_speed pins every tick to 16 ms (TestAnimationManager.cs) over a
// deterministic ticker — here the manual_ticker pumped through the manual_dispatcher's virtual clock
// instead of C#'s thread-blocking/async test tickers.
#include "maui/animations/animation_manager.hpp"

#include <chrono>
#include <cstdint>
#include <memory>

#include "maui/animations/animation.hpp"
#include "maui/animations/easing.hpp"
#include "maui/animations/manual_ticker.hpp"
#include "maui/core/manual_dispatcher.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::animations::animation;
    using maui::animations::animation_manager;
    using maui::animations::easing;
    using maui::animations::manual_ticker;
    using maui::core::manual_dispatcher;

    // The C# TestAnimationManager: every fire counts as 16 ms so runs are predictable.
    class test_animation_manager final : public animation_manager
    {
    public:
        using animation_manager::animation_manager;

    protected:
        [[nodiscard]] double adjust_speed(double /*elapsed_milliseconds*/) const override
        {
            return 16;
        }
    };

    // A manager with a virtual clock, for exercising the DEFAULT adjust_speed (speed_modifier) path.
    class virtual_clock_manager final : public animation_manager
    {
    public:
        using animation_manager::animation_manager;
        std::int64_t now_milliseconds = 0;

    protected:
        [[nodiscard]] std::int64_t current_tick_milliseconds() const override
        {
            return now_milliseconds;
        }
    };

    struct manager_fixture
    {
        manual_dispatcher dispatcher;
        std::shared_ptr<manual_ticker> ticker = std::make_shared<manual_ticker>(dispatcher);
        std::shared_ptr<test_animation_manager> manager = std::make_shared<test_animation_manager>(ticker);

        void advance(int milliseconds)
        {
            dispatcher.advance(std::chrono::milliseconds(milliseconds));
        }
    };

    TEST(animation_manager, add_auto_starts_the_ticker_and_finish_stops_it)
    {
        manager_fixture fx;
        EXPECT_FALSE(fx.ticker->is_running());

        auto anim = std::make_shared<animation>([](double) {}, 0.0, 1.0, easing::linear());
        anim->commit(fx.manager);
        EXPECT_TRUE(fx.ticker->is_running());
        EXPECT_EQ(anim->animation_manager().get(), fx.manager.get());

        // 1000 ms of animation at 16 ms per fire (the manual ticker fires every 16 ms of virtual
        // time): generously pump past the end.
        fx.advance(2000);
        EXPECT_TRUE(anim->has_finished());
        EXPECT_FALSE(fx.ticker->is_running()); // removed + ended
    }

    TEST(animation_manager, add_respects_auto_start_ticker_opt_out)
    {
        manager_fixture fx;
        fx.manager->set_auto_start_ticker(false);
        auto anim = std::make_shared<animation>([](double) {}, 0.0, 1.0, easing::linear());
        anim->commit(fx.manager);
        EXPECT_FALSE(fx.ticker->is_running());
        fx.ticker->start(); // manually started, animations then run
        fx.advance(2000);
        EXPECT_TRUE(anim->has_finished());
    }

    TEST(animation_manager, remove_stops_the_ticker_when_no_animations_remain)
    {
        const manager_fixture fx;
        auto anim = std::make_shared<animation>([](double) {}, 0.0, 10.0, easing::linear());
        anim->commit(fx.manager);
        EXPECT_TRUE(fx.ticker->is_running());
        fx.manager->remove(*anim);
        EXPECT_FALSE(fx.ticker->is_running());
    }

    TEST(animation_manager, pause_removes_and_resume_re_adds)
    {
        manager_fixture fx;
        double last = -1;
        auto anim = std::make_shared<animation>([&last](double v) { last = v; }, 0.0, 1.0, easing::linear());
        anim->commit(fx.manager);
        fx.advance(160); // 10 fires = 160 ms of animation time
        const double at_pause = last;
        EXPECT_GT(at_pause, 0.0);

        anim->pause();
        EXPECT_FALSE(fx.ticker->is_running()); // the only animation left the manager
        fx.advance(160);
        EXPECT_EQ(last, at_pause);

        anim->resume();
        EXPECT_TRUE(fx.ticker->is_running());
        fx.advance(2000);
        EXPECT_TRUE(anim->has_finished());
        EXPECT_DOUBLE_EQ(last, 1.0);
    }

    TEST(animation_manager, add_is_a_no_op_while_the_system_disabled_animations)
    {
        manager_fixture fx;
        fx.ticker->set_system_enabled(false);
        auto anim = std::make_shared<animation>([](double) {}, 0.0, 1.0, easing::linear());
        anim->commit(fx.manager);
        EXPECT_FALSE(fx.ticker->is_running());
        fx.advance(2000);
        EXPECT_FALSE(anim->has_finished());
    }

    TEST(animation_manager, disabling_the_system_mid_run_force_finishes_animations)
    {
        manager_fixture fx;
        double last = -1;
        auto anim = std::make_shared<animation>([&last](double v) { last = v; }, 0.0, 1.0, easing::linear());
        anim->commit(fx.manager);
        fx.advance(160);
        EXPECT_LT(last, 1.0);

        // C# Ticker.OnSystemEnabledChanged fires once more while running; the manager force-finishes
        // everything and stops the ticker.
        fx.ticker->set_system_enabled(false);
        EXPECT_TRUE(anim->has_finished());
        EXPECT_DOUBLE_EQ(last, 1.0);
        EXPECT_FALSE(fx.ticker->is_running());
    }

    TEST(animation_manager, speed_modifier_scales_the_elapsed_time)
    {
        manual_dispatcher dispatcher;
        auto ticker = std::make_shared<manual_ticker>(dispatcher);
        auto manager = std::make_shared<virtual_clock_manager>(ticker);
        manager->set_speed_modifier(2.0);

        double last = -1;
        auto anim = std::make_shared<animation>([&last](double v) { last = v; }, 0.0, 1.0, easing::linear());
        anim->commit(manager);
        manager->now_milliseconds = 0;
        // One fire after 250 virtual-clock ms: elapsed 250 * 2.0 = 500 ms => progress 0.5.
        manager->now_milliseconds = 250;
        dispatcher.advance(std::chrono::milliseconds(16));
        EXPECT_DOUBLE_EQ(last, 0.5);
    }

    TEST(animation_manager, finished_animations_are_removed_on_the_next_fire)
    {
        manager_fixture fx;
        auto anim = std::make_shared<animation>([](double) {}, 0.0, 1.0, easing::linear());
        anim->commit(fx.manager);
        fx.advance(2000);
        EXPECT_TRUE(anim->has_finished());
        // Re-adding a finished animation: the manager drops it again on the very next fire and ends.
        fx.manager->add(anim);
        EXPECT_TRUE(fx.ticker->is_running());
        fx.advance(16);
        EXPECT_FALSE(fx.ticker->is_running());
    }

    // ---- the manual ticker itself ----

    TEST(manual_ticker, fires_once_per_interval_crossed_and_stops_cleanly)
    {
        manual_dispatcher dispatcher;
        manual_ticker ticker(dispatcher);
        int fires = 0;
        ticker.set_fire([&fires] { ++fires; });

        EXPECT_FALSE(ticker.is_running());
        ticker.start();
        EXPECT_TRUE(ticker.is_running());
        ticker.start(); // idempotent

        dispatcher.advance(std::chrono::milliseconds(160)); // 10 intervals at 1000/60 -> 16 ms
        EXPECT_EQ(fires, 10);

        ticker.stop();
        EXPECT_FALSE(ticker.is_running());
        ticker.stop(); // idempotent
        dispatcher.advance(std::chrono::milliseconds(160));
        EXPECT_EQ(fires, 10);
    }

    TEST(manual_ticker, max_fps_drives_the_interval)
    {
        manual_dispatcher dispatcher;
        manual_ticker ticker(dispatcher);
        ticker.set_max_fps(10); // 100 ms interval
        int fires = 0;
        ticker.set_fire([&fires] { ++fires; });
        ticker.start();
        dispatcher.advance(std::chrono::milliseconds(550));
        EXPECT_EQ(fires, 5);
    }
} // namespace
