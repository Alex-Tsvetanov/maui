#pragma once
// maui::devices::sensors::detail::accelerometer_base  <=  the cross-platform half of
// Microsoft.Maui.Devices.Sensors.AccelerometerImplementation (Accelerometer.shared.cs): the
// basic_sensor lifecycle plus the shake detector. OnChanged(reading) raises ReadingChanged and
// feeds the AccelerometerQueue (readings are in G's; g = |a*9.81|^2 against the 169 threshold);
// when the queue reports shaking it is cleared and ShakeDetected raised. The "now" used for the
// shake window is a virtual seam (now_nanoseconds) so the headless fake can drive it
// deterministically - C# reads DateTime.UtcNow at millisecond granularity, which is the default.

#include "maui/core/event.hpp"
#include "maui/essentials/accelerometer.hpp"

#include "src/essentials/detail/accelerometer_queue.hpp"
#include "src/essentials/detail/sensor_base.hpp"

namespace maui::devices::sensors::detail
{
    class accelerometer_base : public basic_sensor<i_accelerometer, accelerometer_data>
    {
    public:
        maui::core::event<>& shake_detected() override
        {
            return shake_detected_;
        }

    protected:
        accelerometer_base() : basic_sensor("Accelerometer")
        {
        }

        // AccelerometerImplementation.OnChanged: raise the reading, then run shake detection.
        void on_changed(const accelerometer_data& reading);

        // The shake clock (C#: (DateTime.UtcNow.Ticks / TicksPerMillisecond) * 1_000_000).
        [[nodiscard]] virtual std::int64_t now_nanoseconds() const;

    private:
        // accelerationThreshold = 169, gravity = 9.81 (Accelerometer.shared.cs).
        static constexpr double acceleration_threshold = 169;
        static constexpr double gravity = 9.81;

        accelerometer_queue queue_;
        maui::core::event<> shake_detected_;
    };
} // namespace maui::devices::sensors::detail
