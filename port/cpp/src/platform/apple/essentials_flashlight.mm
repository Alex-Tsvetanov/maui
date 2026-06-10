// flashlight - Apple (AppKit / macOS) platform partial. NOT SUPPORTED on macOS per the suffix
// oracle (Flashlight.netstandard.tvos.watchos.macos.cs): IsSupportedAsync resolves false and
// TurnOn/TurnOffAsync throw - mirrored synchronously here. The real torch lives in the ios twin.

#include <memory>

#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/flashlight.hpp"

namespace maui::devices
{
    namespace
    {
        class apple_flashlight final : public i_flashlight
        {
        public:
            [[nodiscard]] bool is_supported() override
            {
                return false; // Task.FromResult(false)
            }
            void turn_on() override
            {
                throw maui::application_model::feature_not_supported(
                    "Flashlight is not supported on macOS (Flashlight.netstandard.*.macos.cs).");
            }
            void turn_off() override
            {
                throw maui::application_model::feature_not_supported(
                    "Flashlight is not supported on macOS (Flashlight.netstandard.*.macos.cs).");
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_flashlight> make_flashlight()
        {
            return std::make_shared<apple_flashlight>();
        }
    } // namespace detail
} // namespace maui::devices
