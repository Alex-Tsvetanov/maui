// gyroscope - Apple (AppKit / macOS) platform partial. NOT SUPPORTED on macOS per the suffix
// oracle (Gyroscope.netstandard.tvos.macos.cs): IsSupported throws, so the shared lifecycle
// makes start/stop throw too (is_monitoring stays false). The real CoreMotion/CoreLocation
// implementation lives in the ios twin.

#include <memory>

#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/gyroscope.hpp"

#include "src/essentials/detail/sensor_base.hpp"

namespace maui::devices::sensors
{
    namespace
    {
        class apple_gyroscope final : public detail::basic_sensor<i_gyroscope, gyroscope_data>
        {
        public:
            apple_gyroscope() : detail::basic_sensor<i_gyroscope, gyroscope_data>("Gyroscope")
            {
            }

        protected:
            [[nodiscard]] bool platform_is_supported() const override
            {
                throw maui::application_model::feature_not_supported(
                    "Gyroscope is not supported on macOS (Gyroscope.netstandard.tvos.macos.cs).");
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
        std::shared_ptr<i_gyroscope> make_gyroscope()
        {
            return std::make_shared<apple_gyroscope>();
        }
    } // namespace detail
} // namespace maui::devices::sensors
