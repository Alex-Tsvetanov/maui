#pragma once
// maui::devices::sensors::detail::accelerometer_queue  <=
// Microsoft.Maui.Devices.Sensors.AccelerometerQueue (AccelerometerQueue.shared.cs, internal)
//
// The shake detector's sliding sample window: "3/4ths of the accelerometer events in the last half
// second are accelerating" means the device is shaking / free-falling. Ported 1:1 - same window
// constants (nanoseconds), same purge rule (never below four samples), same IsShaking inequality
// (acceleratingCount >= (n >> 1) + (n >> 2)). C#'s hand-rolled linked list + object pool (a GC
// optimization) collapses to std::deque; the observable behavior is identical and the C# unit
// tests (Accelerometer_Tests.ShakingTests et al.) port against this type directly.

#include <cstdint>
#include <deque>

namespace maui::devices::sensors::detail
{
    class accelerometer_queue
    {
    public:
        // Add a sample (timestamp in nanoseconds), purging entries older than the max window.
        void add(std::int64_t timestamp, bool accelerating)
        {
            purge(timestamp - max_window_size);
            samples_.push_back({timestamp, accelerating});
            if (accelerating)
            {
                ++accelerating_count_;
            }
        }

        void clear()
        {
            samples_.clear();
            accelerating_count_ = 0;
        }

        // True when the window spans at least the minimum AND >= 3/4 of the samples accelerate.
        [[nodiscard]] bool is_shaking() const
        {
            const auto count = static_cast<int>(samples_.size());
            return !samples_.empty() && samples_.back().timestamp - samples_.front().timestamp >= min_window_size &&
                   accelerating_count_ >= (count >> 1) + (count >> 2);
        }

    private:
        struct sample
        {
            std::int64_t timestamp = 0;
            bool is_accelerating = false;
        };

        // Drop samples older than the cutoff, but keep at least the minimum queue size.
        void purge(std::int64_t cutoff)
        {
            while (static_cast<int>(samples_.size()) >= min_queue_size && !samples_.empty() &&
                   cutoff - samples_.front().timestamp > 0)
            {
                if (samples_.front().is_accelerating)
                {
                    --accelerating_count_;
                }
                samples_.pop_front();
            }
        }

        static constexpr std::int64_t max_window_size = 500'000'000; // ns
        static constexpr std::int64_t min_window_size = 250'000'000; // ns
        static constexpr int min_queue_size = 4;

        std::deque<sample> samples_;
        int accelerating_count_ = 0;
    };
} // namespace maui::devices::sensors::detail
