// The cross-platform half of the accelerometer: the facade slot (Accelerometer.Default /
// SetDefault) plus accelerometer_base's shake detection (AccelerometerImplementation.OnChanged +
// ProcessShakeEvent from Accelerometer.shared.cs). The platform partial is
// src/platform/<backend>/essentials_accelerometer.{cpp,mm}.

#include "maui/essentials/accelerometer.hpp"

#include <chrono>
#include <cstdint>
#include <memory>
#include <utility>

#include "maui/essentials/sensor_types.hpp"

#include "src/essentials/detail/accelerometer_base.hpp"

namespace maui::devices::sensors
{
    namespace detail
    {
        void accelerometer_base::on_changed(const accelerometer_data& reading)
        {
            raise_reading_changed(reading);

            // ProcessShakeEvent: readings are in G's; compare |a * 9.81|^2 to the threshold.
            // (C# skips this when ShakeDetected has no subscribers - an unobservable optimization.)
            const double x = reading.acceleration.x * gravity;
            const double y = reading.acceleration.y * gravity;
            const double z = reading.acceleration.z * gravity;
            const double g = (x * x) + (y * y) + (z * z);
            queue_.add(now_nanoseconds(), g > acceleration_threshold);

            if (queue_.is_shaking())
            {
                queue_.clear();
                shake_detected_.raise();
            }
        }

        std::int64_t accelerometer_base::now_nanoseconds() const
        {
            // C#: (DateTime.UtcNow.Ticks / TicksPerMillisecond) * 1_000_000 - millisecond
            // granularity expressed in nanoseconds.
            const auto now = std::chrono::system_clock::now().time_since_epoch();
            return std::chrono::duration_cast<std::chrono::milliseconds>(now).count() * 1'000'000;
        }
    } // namespace detail

    namespace
    {
        std::shared_ptr<i_accelerometer>& accelerometer_storage()
        {
            static std::shared_ptr<i_accelerometer> storage;
            return storage;
        }
    } // namespace

    i_accelerometer& accelerometer::default_()
    {
        auto& storage = accelerometer_storage();
        if (storage == nullptr)
        {
            storage = detail::make_accelerometer();
        }
        return *storage;
    }

    void accelerometer::set_default(std::shared_ptr<i_accelerometer> implementation)
    {
        accelerometer_storage() = std::move(implementation);
    }
} // namespace maui::devices::sensors
