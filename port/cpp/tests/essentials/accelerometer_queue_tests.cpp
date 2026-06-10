// Ports of Accelerometer_Tests.{InitialShaking, ShakingTests, ClearQueue} - the shake-detection
// window algorithm (AccelerometerQueue.shared.cs), driven with explicit nanosecond timestamps
// exactly like the C# tests. Backend-agnostic.

#include <cstdint>

#include <gtest/gtest.h>

#include "src/essentials/detail/accelerometer_queue.hpp"

namespace
{
    using maui::devices::sensors::detail::accelerometer_queue;

    // C#: Nanoseconds(now.AddSeconds(s)) with a fixed base; the base value itself is irrelevant.
    constexpr std::int64_t base_ns = 1'700'000'000'000'000'000;

    constexpr std::int64_t shake_time(double seconds)
    {
        return base_ns + static_cast<std::int64_t>(seconds * 1'000'000'000.0);
    }

    TEST(accelerometer_queue, initial_shaking_is_false)
    {
        const accelerometer_queue q;
        EXPECT_FALSE(q.is_shaking());
    }

    TEST(accelerometer_queue, shaking_tests)
    {
        accelerometer_queue q;
        q.add(shake_time(0), false);
        q.add(shake_time(.3), false);
        q.add(shake_time(.6), false);
        q.add(shake_time(.9), false);
        EXPECT_FALSE(q.is_shaking());

        // The oldest two entries will be removed.
        q.add(shake_time(1.2), true);
        q.add(shake_time(1.5), true);
        EXPECT_FALSE(q.is_shaking());

        // Another entry should be removed, now 3 out of 4 are true.
        q.add(shake_time(1.8), true);
        EXPECT_TRUE(q.is_shaking());

        q.add(shake_time(2.1), false);
        EXPECT_TRUE(q.is_shaking());

        q.add(shake_time(2.4), false);
        EXPECT_FALSE(q.is_shaking());
    }

    TEST(accelerometer_queue, clear_queue)
    {
        accelerometer_queue q;
        q.add(shake_time(0), true);
        q.add(shake_time(.1), true);
        q.add(shake_time(.3), true);
        EXPECT_TRUE(q.is_shaking());
        q.clear();
        EXPECT_FALSE(q.is_shaking());
    }
} // namespace
