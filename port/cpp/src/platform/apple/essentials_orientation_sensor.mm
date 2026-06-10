// orientation_sensor - Apple (AppKit / macOS) platform partial. NOT SUPPORTED on macOS per the suffix
// oracle (OrientationSensor.netstandard.tvos.macos.cs): IsSupported throws, so the shared lifecycle
// makes start/stop throw too (is_monitoring stays false). The real CoreMotion/CoreLocation
// implementation lives in the ios twin.

#include <memory>

#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/orientation_sensor.hpp"

#include "src/essentials/detail/sensor_base.hpp"

namespace maui::devices::sensors
{
    namespace
    {
        class apple_orientation_sensor final : public detail::basic_sensor<i_orientation_sensor, orientation_sensor_data>
        {
        public:
            apple_orientation_sensor() : detail::basic_sensor<i_orientation_sensor, orientation_sensor_data>("OrientationSensor")
            {
            }

        protected:
            [[nodiscard]] bool platform_is_supported() const override
            {
                throw maui::application_model::feature_not_supported(
                    "OrientationSensor is not supported on macOS (OrientationSensor.netstandard.tvos.macos.cs).");
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
        std::shared_ptr<i_orientation_sensor> make_orientation_sensor()
        {
            return std::make_shared<apple_orientation_sensor>();
        }
    } // namespace detail
} // namespace maui::devices::sensors
