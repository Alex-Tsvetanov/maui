// compass - Apple (AppKit / macOS) platform partial. NOT SUPPORTED on macOS per the suffix oracle
// (Compass.netstandard.tvos.watchos.macos.cs): IsSupported throws, so the shared lifecycle makes
// start/stop throw too. The real CLLocationManager heading implementation lives in the ios twin.

#include <memory>

#include "maui/essentials/compass.hpp"
#include "maui/essentials/feature_not_supported.hpp"

#include "src/essentials/detail/compass_base.hpp"

namespace maui::devices::sensors
{
    namespace
    {
        class apple_compass final : public detail::compass_base
        {
        protected:
            [[nodiscard]] bool platform_is_supported() const override
            {
                throw maui::application_model::feature_not_supported(
                    "Compass is not supported on macOS (Compass.netstandard.tvos.watchos.macos.cs).");
            }

            // Unreachable - the is_supported gate throws first (netstandard PlatformStart/Stop mirror).
            void platform_start(sensor_speed /*speed*/, bool /*apply_low_pass_filter*/) override
            {
            }
            void platform_stop() override
            {
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_compass> make_compass()
        {
            return std::make_shared<apple_compass>();
        }
    } // namespace detail
} // namespace maui::devices::sensors
