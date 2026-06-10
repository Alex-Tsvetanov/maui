// accelerometer - Apple (AppKit / macOS) platform partial. NOT SUPPORTED on macOS per the suffix
// oracle (Accelerometer.netstandard.tvos.macos.cs): IsSupported throws, so the shared lifecycle
// makes start/stop throw too (is_monitoring stays false). The real CoreMotion/CoreLocation
// implementation lives in the ios twin.

#include <memory>

#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/accelerometer.hpp"

#include "src/essentials/detail/accelerometer_base.hpp"

namespace maui::devices::sensors
{
    namespace
    {
        class apple_accelerometer final : public detail::accelerometer_base
        {
        protected:
            [[nodiscard]] bool platform_is_supported() const override
            {
                throw maui::application_model::feature_not_supported(
                    "Accelerometer is not supported on macOS (Accelerometer.netstandard.tvos.macos.cs).");
            }

            // Unreachable - the is_supported gate throws first (netstandard PlatformStart/Stop mirror).
            void platform_start(sensor_speed /*speed*/) override
            {
            }
            void platform_stop() override
            {
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_accelerometer> make_accelerometer()
        {
            return std::make_shared<apple_accelerometer>();
        }
    } // namespace detail
} // namespace maui::devices::sensors
