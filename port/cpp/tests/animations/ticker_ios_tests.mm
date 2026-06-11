// On-simulator assertion for the iOS platform ticker (src/platform/ios/platform_ticker.mm): the
// CADisplayLink-driven create_platform_ticker actually fires on the pumped run loop (the simulator
// delivers display-link frames to spawned processes), start/stop are idempotent, and stopping really
// silences it. Real-time bound with GENEROUS deadlines and no exact frame counts (per the W1-14
// anti-flake rule); the deterministic coverage lives in the backend-agnostic suites.
#import <Foundation/Foundation.h>

#include <chrono>
#include <functional>

#include "maui/animations/platform_ticker.hpp"
#include "maui/animations/ticker.hpp"
#include "maui/core/manual_dispatcher.hpp"
#include <gtest/gtest.h>

namespace
{
    using namespace std::chrono_literals;

    // Pump the main run loop until `done` or the deadline (the gcd_dispatcher_tests idiom).
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

    TEST(platform_ticker_ios, display_link_fires_on_the_pumped_run_loop_and_stop_silences_it)
    {
        maui::core::manual_dispatcher dispatcher; // the CADisplayLink ticker ignores the dispatcher
        const auto ticker = maui::animations::create_platform_ticker(dispatcher);
        ASSERT_NE(ticker, nullptr);
        EXPECT_TRUE(ticker->system_enabled());
        EXPECT_EQ(ticker->max_fps(), 60);

        int fires = 0;
        ticker->set_fire([&fires] { ++fires; });
        EXPECT_FALSE(ticker->is_running());

        ticker->start();
        EXPECT_TRUE(ticker->is_running()); // C# IsRunning => _link != null
        ticker->start();                   // idempotent
        EXPECT_TRUE(pump_until([&fires] { return fires >= 2; }));

        ticker->stop();
        EXPECT_FALSE(ticker->is_running());
        ticker->stop(); // idempotent
        const int at_stop = fires;
        (void)pump_until([] { return false; }, 100ms); // invalidated link: nothing more lands
        EXPECT_EQ(fires, at_stop);
    }
} // namespace
