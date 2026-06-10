// Tests for maui::core::gcd_dispatcher (the GCD main-queue i_dispatcher + its dispatch-source timer).
// ONE .mm shared by the apple AND ios presets — the dispatcher under test is shared the same way
// (src/platform/apple_shared/gcd_dispatcher.mm), and everything here is Foundation/CF (identical on
// AppKit/UIKit). gtest runs on the process main thread, whose CFRunLoop services the GCD main queue, so
// each test pumps CFRunLoopRunInMode until the dispatched work (or timer tick) lands — wall-clock bound,
// with generous deadlines so a busy simulator cannot flake the suite. Mirrors the manual_dispatcher
// characterization (tests/core/dispatcher_tests.cpp) where wall-clock semantics allow, plus the C#
// DispatcherTimer contract (Dispatcher.iOS.cs): IsRepeating default true, one-shot stops after the
// first tick, Stop() prevents further ticks.
#import <Foundation/Foundation.h>

#include "maui/core/gcd_dispatcher.hpp"

#include <pthread.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

namespace
{
    using namespace std::chrono_literals;
    using maui::core::gcd_dispatcher;

    // Pump the main run loop (which drains the GCD main queue) until `done` or the deadline. Returns
    // whether `done` turned true.
    bool pump_until(const std::function<bool()>& done, std::chrono::milliseconds deadline = 5000ms)
    {
        const NSTimeInterval seconds = static_cast<NSTimeInterval>(deadline.count()) / 1000.0;
        NSDate* const limit = [NSDate dateWithTimeIntervalSinceNow:seconds];
        while (!done() && [limit timeIntervalSinceNow] > 0)
        {
            CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.01, static_cast<Boolean>(true));
        }
        return done();
    }

    TEST(gcd_dispatcher, is_dispatch_required_false_on_the_main_thread)
    {
        const gcd_dispatcher dispatcher;
        EXPECT_FALSE(dispatcher.is_dispatch_required()); // gtest runs on the main thread
    }

    TEST(gcd_dispatcher, is_dispatch_required_true_off_the_main_thread)
    {
        const gcd_dispatcher dispatcher;
        bool required_elsewhere = false;
        std::thread worker([&] { required_elsewhere = dispatcher.is_dispatch_required(); });
        worker.join();
        EXPECT_TRUE(required_elsewhere);
    }

    TEST(gcd_dispatcher, dispatch_runs_on_the_pumped_main_queue)
    {
        gcd_dispatcher dispatcher;
        bool ran = false;
        EXPECT_TRUE(dispatcher.dispatch([&ran] { ran = true; }));
        EXPECT_TRUE(pump_until([&ran] { return ran; }));
    }

    TEST(gcd_dispatcher, dispatch_runs_in_fifo_order)
    {
        gcd_dispatcher dispatcher;
        std::vector<int> order;
        dispatcher.dispatch([&order] { order.push_back(1); });
        dispatcher.dispatch([&order] { order.push_back(2); });
        dispatcher.dispatch([&order] { order.push_back(3); });
        ASSERT_TRUE(pump_until([&order] { return order.size() == 3U; }));
        EXPECT_EQ(order, (std::vector<int>{1, 2, 3}));
    }

    TEST(gcd_dispatcher, dispatch_from_a_background_thread_marshals_to_the_main_thread)
    {
        gcd_dispatcher dispatcher;
        std::atomic<bool> ran{false};
        std::atomic<bool> ran_on_main{false};
        std::thread worker([&] {
            dispatcher.dispatch([&] {
                ran_on_main = pthread_main_np() != 0;
                ran = true;
            });
        });
        worker.join();
        ASSERT_TRUE(pump_until([&] { return ran.load(); }));
        EXPECT_TRUE(ran_on_main.load());
    }

    TEST(gcd_dispatcher, dispatch_delayed_respects_the_delay)
    {
        gcd_dispatcher dispatcher;
        bool ran = false;
        const auto start = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point fired_at;
        EXPECT_TRUE(dispatcher.dispatch_delayed(100ms, [&] {
            fired_at = std::chrono::steady_clock::now();
            ran = true;
        }));
        ASSERT_TRUE(pump_until([&ran] { return ran; }));
        // GCD guarantees "no earlier than" — allow a small scheduler tolerance on the lower bound.
        EXPECT_GE(fired_at - start, 90ms);
    }

    TEST(gcd_dispatcher_timer, defaults_match_the_csharp_timer)
    {
        gcd_dispatcher dispatcher;
        const auto timer = dispatcher.create_timer();
        ASSERT_NE(timer, nullptr);
        EXPECT_TRUE(timer->is_repeating()); // C# IsRepeating default
        EXPECT_FALSE(timer->is_running());
        EXPECT_EQ(timer->interval(), 0ms);
    }

    TEST(gcd_dispatcher_timer, repeating_timer_ticks_until_stopped)
    {
        gcd_dispatcher dispatcher;
        const auto timer = dispatcher.create_timer();
        timer->set_interval(20ms);
        int ticks = 0;
        timer->tick().connect([&] {
            ++ticks;
            if (ticks == 3)
            {
                timer->stop();
            }
        });
        timer->start();
        EXPECT_TRUE(timer->is_running());
        ASSERT_TRUE(pump_until([&ticks] { return ticks >= 3; }));
        EXPECT_FALSE(timer->is_running()); // stopped from inside the tick

        // No further delivery after Stop: drain a couple more intervals and recheck.
        bool settled = false;
        dispatcher.dispatch_delayed(80ms, [&settled] { settled = true; });
        ASSERT_TRUE(pump_until([&settled] { return settled; }));
        EXPECT_EQ(ticks, 3);
    }

    TEST(gcd_dispatcher_timer, one_shot_timer_ticks_once_then_stops)
    {
        gcd_dispatcher dispatcher;
        const auto timer = dispatcher.create_timer();
        timer->set_interval(20ms);
        timer->set_is_repeating(false);
        int ticks = 0;
        timer->tick().connect([&ticks] { ++ticks; });
        timer->start();
        ASSERT_TRUE(pump_until([&ticks] { return ticks >= 1; }));
        EXPECT_FALSE(timer->is_running()); // OnTimerTick -> Stop() when !IsRepeating

        bool settled = false;
        dispatcher.dispatch_delayed(80ms, [&settled] { settled = true; });
        ASSERT_TRUE(pump_until([&settled] { return settled; }));
        EXPECT_EQ(ticks, 1);
    }

    TEST(gcd_dispatcher_timer, stop_before_the_first_tick_prevents_it)
    {
        gcd_dispatcher dispatcher;
        const auto timer = dispatcher.create_timer();
        timer->set_interval(50ms);
        int ticks = 0;
        timer->tick().connect([&ticks] { ++ticks; });
        timer->start();
        timer->stop(); // cancelled before any delivery
        EXPECT_FALSE(timer->is_running());

        bool settled = false;
        dispatcher.dispatch_delayed(120ms, [&settled] { settled = true; });
        ASSERT_TRUE(pump_until([&settled] { return settled; }));
        EXPECT_EQ(ticks, 0);
    }

    TEST(gcd_dispatcher_timer, start_is_idempotent_while_running)
    {
        gcd_dispatcher dispatcher;
        const auto timer = dispatcher.create_timer();
        timer->set_interval(20ms);
        int ticks = 0;
        timer->tick().connect([&ticks] { ++ticks; });
        timer->start();
        timer->start(); // C# Start: no-op when already running (no double schedule)
        ASSERT_TRUE(pump_until([&ticks] { return ticks >= 1; }));
        timer->stop();
        EXPECT_FALSE(timer->is_running());
    }
} // namespace
