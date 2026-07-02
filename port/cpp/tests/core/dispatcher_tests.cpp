// Tests for maui::core::manual_dispatcher (the headless virtual-clock i_dispatcher).
// Characterizes: dispatch queues until pumped, FIFO order, is_dispatch_required across threads,
// delayed ordering against the virtual clock, and timer repeat/one-shot/stop behaviour.
#include "maui/core/manual_dispatcher.hpp"

#include <chrono>
#include <memory>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

namespace
{
    using namespace std::chrono_literals;
    using maui::core::manual_dispatcher;

    TEST(dispatcher, dispatch_queues_until_pumped)
    {
        manual_dispatcher d;
        int ran = 0;
        EXPECT_TRUE(d.dispatch([&] { ++ran; }));
        EXPECT_EQ(ran, 0); // not run yet
        EXPECT_EQ(d.pending_count(), 1U);
        EXPECT_EQ(d.run_pending(), 1U);
        EXPECT_EQ(ran, 1);
        EXPECT_EQ(d.pending_count(), 0U);
    }

    TEST(dispatcher, dispatch_runs_in_fifo_order)
    {
        manual_dispatcher d;
        std::vector<int> order;
        d.dispatch([&] { order.push_back(1); });
        d.dispatch([&] { order.push_back(2); });
        d.dispatch([&] { order.push_back(3); });
        d.run_pending();
        EXPECT_EQ(order, (std::vector<int>{1, 2, 3}));
    }

    TEST(dispatcher, nested_dispatch_runs_in_same_pump)
    {
        manual_dispatcher d;
        std::vector<int> order;
        d.dispatch([&] {
            order.push_back(1);
            d.dispatch([&] { order.push_back(2); }); // queued at now_, already due
        });
        EXPECT_EQ(d.run_pending(), 2U);
        EXPECT_EQ(order, (std::vector<int>{1, 2}));
    }

    TEST(dispatcher, is_dispatch_required_false_on_owner_thread)
    {
        manual_dispatcher d;
        EXPECT_FALSE(d.is_dispatch_required());
    }

    TEST(dispatcher, is_dispatch_required_true_on_other_thread)
    {
        manual_dispatcher d;
        bool required_elsewhere = false;
        std::thread t([&] { required_elsewhere = d.is_dispatch_required(); });
        t.join();
        EXPECT_TRUE(required_elsewhere);
    }

    TEST(dispatcher, delayed_does_not_run_before_its_time)
    {
        manual_dispatcher d;
        int ran = 0;
        d.dispatch_delayed(100ms, [&] { ++ran; });
        d.run_pending(); // now_ == 0
        EXPECT_EQ(ran, 0);
        d.advance(99ms);
        EXPECT_EQ(ran, 0);
        d.advance(1ms); // now_ == 100ms
        EXPECT_EQ(ran, 1);
    }

    TEST(dispatcher, delayed_runs_in_time_then_seq_order)
    {
        manual_dispatcher d;
        std::vector<int> order;
        d.dispatch_delayed(30ms, [&] { order.push_back(30); });
        d.dispatch_delayed(10ms, [&] { order.push_back(10); });
        d.dispatch_delayed(10ms, [&] { order.push_back(11); }); // same due, later seq
        d.advance(50ms);
        EXPECT_EQ(order, (std::vector<int>{10, 11, 30}));
        EXPECT_EQ(d.now(), 50ms);
    }

    TEST(dispatcher, timer_repeats_once_per_interval_crossed)
    {
        manual_dispatcher d;
        auto timer = d.create_timer();
        ASSERT_NE(timer, nullptr);
        timer->set_interval(10ms);
        timer->set_is_repeating(true);
        int ticks = 0;
        timer->tick().connect([&] { ++ticks; });
        timer->start();
        EXPECT_TRUE(timer->is_running());
        d.advance(35ms); // ticks at 10, 20, 30
        EXPECT_EQ(ticks, 3);
    }

    TEST(dispatcher, timer_one_shot_fires_once_then_stops)
    {
        manual_dispatcher d;
        auto timer = d.create_timer();
        timer->set_interval(10ms);
        timer->set_is_repeating(false);
        int ticks = 0;
        timer->tick().connect([&] { ++ticks; });
        timer->start();
        d.advance(100ms);
        EXPECT_EQ(ticks, 1);
        EXPECT_FALSE(timer->is_running());
    }

    TEST(dispatcher, stop_prevents_further_ticks)
    {
        manual_dispatcher d;
        auto timer = d.create_timer();
        timer->set_interval(10ms);
        timer->set_is_repeating(true);
        int ticks = 0;
        timer->tick().connect([&] { ++ticks; });
        timer->start();
        d.advance(15ms); // 1 tick at 10ms
        EXPECT_EQ(ticks, 1);
        timer->stop();
        d.advance(100ms);
        EXPECT_EQ(ticks, 1); // no more
        EXPECT_FALSE(timer->is_running());
    }

    TEST(dispatcher, destroying_timer_cancels_pending_tick)
    {
        manual_dispatcher d;
        int ticks = 0;
        {
            auto timer = d.create_timer();
            timer->set_interval(10ms);
            timer->set_is_repeating(true);
            timer->tick().connect([&] { ++ticks; });
            timer->start();
        } // timer destroyed with a tick still queued
        d.advance(100ms);
        EXPECT_EQ(ticks, 0); // the stale tick is dropped, not run on a dangling timer
    }
} // namespace
