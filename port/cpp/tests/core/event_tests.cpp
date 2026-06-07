// Tests for maui::core::event<Args...> + scoped_connection (PROFILE.md §5).
// Characterization of the .NET-multicast-mirroring semantics: connection-order dispatch, token
// disconnect, and snapshot-on-raise (connect/disconnect inside a handler applies next raise).
#include "maui/core/event.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

namespace
{
    using maui::core::connection_token;
    using maui::core::event;
    using maui::core::scoped_connection;

    TEST(event_basics, raise_invokes_connected_handler_with_args)
    {
        event<int, std::string> e;
        int seen_n = 0;
        std::string seen_s;
        e.connect([&](int n, const std::string& s) {
            seen_n = n;
            seen_s = s;
        });
        e.raise(42, "hi");
        EXPECT_EQ(seen_n, 42);
        EXPECT_EQ(seen_s, "hi");
    }

    TEST(event_basics, zero_arg_event)
    {
        event<> e;
        int count = 0;
        e.connect([&] { ++count; });
        e.raise();
        e.raise();
        EXPECT_EQ(count, 2);
    }

    TEST(event_basics, no_handlers_is_a_noop)
    {
        event<> const e;
        EXPECT_TRUE(e.empty());
        EXPECT_EQ(e.handler_count(), 0U);
        e.raise(); // must not crash
    }

    TEST(event_basics, handlers_run_in_connection_order)
    {
        event<> e;
        std::vector<int> order;
        e.connect([&] { order.push_back(1); });
        e.connect([&] { order.push_back(2); });
        e.connect([&] { order.push_back(3); });
        e.raise();
        EXPECT_EQ(order, (std::vector<int>{1, 2, 3}));
    }

    TEST(event_basics, multiple_handlers_all_fire)
    {
        event<int> e;
        int sum = 0;
        e.connect([&](int n) { sum += n; });
        e.connect([&](int n) { sum += n * 10; });
        e.raise(7);
        EXPECT_EQ(sum, 77);
        EXPECT_EQ(e.handler_count(), 2U);
    }

    TEST(event_disconnect, stops_a_handler_and_reports_removal)
    {
        event<> e;
        int count = 0;
        connection_token const t = e.connect([&] { ++count; });
        e.raise();
        EXPECT_TRUE(e.disconnect(t));
        e.raise();
        EXPECT_EQ(count, 1);
        EXPECT_TRUE(e.empty());
    }

    TEST(event_disconnect, unknown_token_returns_false)
    {
        event<> e;
        EXPECT_FALSE(e.disconnect(12345));
        connection_token const t = e.connect([] {});
        EXPECT_TRUE(e.disconnect(t));
        EXPECT_FALSE(e.disconnect(t)); // already gone
    }

    TEST(event_disconnect, only_the_named_handler_is_removed)
    {
        event<> e;
        int a = 0;
        int b = 0;
        connection_token const ta = e.connect([&] { ++a; });
        e.connect([&] { ++b; });
        e.disconnect(ta);
        e.raise();
        EXPECT_EQ(a, 0);
        EXPECT_EQ(b, 1);
    }

    // --- snapshot semantics (the subtle part) ---

    TEST(event_snapshot, handler_disconnecting_a_peer_mid_raise_still_runs_peer_this_round)
    {
        event<> e;
        std::vector<int> order;
        connection_token t3 = 0;
        e.connect([&] { order.push_back(1); });
        e.connect([&] {
            order.push_back(2);
            e.disconnect(t3); // remove h3 during dispatch
        });
        t3 = e.connect([&] { order.push_back(3); });

        e.raise(); // snapshot taken at entry => h3 still runs this round
        EXPECT_EQ(order, (std::vector<int>{1, 2, 3}));

        order.clear();
        e.raise(); // h3 is gone now
        EXPECT_EQ(order, (std::vector<int>{1, 2}));
    }

    TEST(event_snapshot, handler_can_disconnect_itself_one_shot)
    {
        event<> e;
        int count = 0;
        connection_token self = 0;
        self = e.connect([&] {
            ++count;
            e.disconnect(self);
        });
        e.raise();
        e.raise();
        e.raise();
        EXPECT_EQ(count, 1); // fired exactly once, then unsubscribed
        EXPECT_TRUE(e.empty());
    }

    TEST(event_snapshot, handler_connected_during_raise_fires_only_next_round)
    {
        event<> e;
        int outer = 0;
        int inner = 0;
        e.connect([&] {
            ++outer;
            if (outer == 1)
            {
                e.connect([&] { ++inner; });
            }
        });
        e.raise(); // adds inner; inner not in this snapshot
        EXPECT_EQ(outer, 1);
        EXPECT_EQ(inner, 0);
        e.raise(); // now both run
        EXPECT_EQ(outer, 2);
        EXPECT_EQ(inner, 1);
    }

    // --- move-only handlers ---

    TEST(event_handlers, supports_move_only_captured_state)
    {
        event<int> e;
        auto sink = std::make_unique<int>(0);
        int const* observed = sink.get();
        e.connect([captured = std::move(sink)](int n) { *captured += n; });
        e.raise(5);
        e.raise(5);
        EXPECT_EQ(*observed, 10);
    }

    // --- scoped_connection ---

    TEST(scoped, disconnects_on_scope_exit)
    {
        event<> e;
        int count = 0;
        {
            scoped_connection const conn(e, e.connect([&] { ++count; }));
            EXPECT_TRUE(conn.connected());
            e.raise();
            EXPECT_EQ(e.handler_count(), 1U);
        }
        EXPECT_TRUE(e.empty()); // auto-disconnected
        e.raise();
        EXPECT_EQ(count, 1);
    }

    TEST(scoped, connect_scoped_helper)
    {
        event<int> e;
        int total = 0;
        {
            auto conn = maui::core::connect_scoped(e, [&](int n) { total += n; });
            e.raise(3);
        }
        e.raise(100); // handler already gone
        EXPECT_EQ(total, 3);
    }

    TEST(scoped, move_transfers_ownership_and_moved_from_does_not_disconnect)
    {
        event<> e;
        scoped_connection b;
        {
            scoped_connection a(e, e.connect([] {}));
            EXPECT_EQ(e.handler_count(), 1U);
            b = std::move(a);
            // a is now moved-from: its disconnect is null (move_only_function null-after-move), so
            // when a goes out of scope at the end of this block it does NOT disconnect the handler.
        }
        EXPECT_TRUE(b.connected());
        EXPECT_EQ(e.handler_count(), 1U); // still exactly one — moved-from a's destruction did not disconnect
        b.reset();
        EXPECT_TRUE(e.empty());
    }

    TEST(scoped, reset_is_idempotent)
    {
        event<> e;
        scoped_connection conn(e, e.connect([] {}));
        conn.reset();
        EXPECT_TRUE(e.empty());
        conn.reset(); // no double-disconnect, no crash
        EXPECT_FALSE(conn.connected());
    }
} // namespace
