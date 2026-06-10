// vibration - Apple (AppKit / macOS) platform partial. NOT SUPPORTED on macOS per the suffix
// oracle (Vibration.netstandard.tvos.watchos.macos.cs): IsSupported throws, so the shared gate in
// vibration_base makes every member throw. The real vibrate lives in the ios twin.

#include <chrono>
#include <memory>

#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/vibration.hpp"

#include "src/essentials/detail/vibration_base.hpp"

namespace maui::devices
{
    namespace
    {
        class apple_vibration final : public detail::vibration_base
        {
        public:
            [[nodiscard]] bool is_supported() const override
            {
                throw maui::application_model::feature_not_supported(
                    "Vibration is not supported on macOS (Vibration.netstandard.*.macos.cs).");
            }

        protected:
            // Unreachable - the is_supported gate throws first (netstandard PlatformVibrate mirror).
            void platform_vibrate() override
            {
            }
            void platform_vibrate(std::chrono::milliseconds /*duration*/) override
            {
            }
            void platform_cancel() override
            {
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_vibration> make_vibration()
        {
            return std::make_shared<apple_vibration>();
        }
    } // namespace detail
} // namespace maui::devices
