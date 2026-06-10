// magnetometer - Apple (AppKit / macOS) platform partial. NOT SUPPORTED on macOS per the suffix
// oracle (Magnetometer.netstandard.tvos.macos.cs): IsSupported throws, so the shared lifecycle
// makes start/stop throw too (is_monitoring stays false). The real CoreMotion/CoreLocation
// implementation lives in the ios twin.

#include <memory>

#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/magnetometer.hpp"

#include "src/essentials/detail/sensor_base.hpp"

namespace maui::devices::sensors
{
    namespace
    {
        class apple_magnetometer final : public detail::basic_sensor<i_magnetometer, magnetometer_data>
        {
        public:
            apple_magnetometer() : detail::basic_sensor<i_magnetometer, magnetometer_data>("Magnetometer")
            {
            }

        protected:
            [[nodiscard]] bool platform_is_supported() const override
            {
                throw maui::application_model::feature_not_supported(
                    "Magnetometer is not supported on macOS (Magnetometer.netstandard.tvos.macos.cs).");
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
        std::shared_ptr<i_magnetometer> make_magnetometer()
        {
            return std::make_shared<apple_magnetometer>();
        }
    } // namespace detail
} // namespace maui::devices::sensors
