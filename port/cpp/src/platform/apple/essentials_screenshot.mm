// screenshot - Apple (AppKit / macOS) platform partial. macOS is NOT SUPPORTED: MAUI ships no
// macOS screenshot partial, so the macos build links the netstandard mirror
// (Screenshot.netstandard.watchos.macos.cs) where IsCaptureSupported and CaptureAsync both throw.
// This partial reproduces that - is_capture_supported() throws feature_not_supported, and
// capture_async never runs because the facade's gate throws first. Pure C++ here (no AppKit needed).

#include <memory>

#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/screenshot.hpp"

namespace maui::media
{
    namespace
    {
        class apple_screenshot final : public i_screenshot
        {
        public:
            [[nodiscard]] bool is_capture_supported() const override
            {
                throw maui::application_model::feature_not_supported(); // Screenshot.macos: not supported
            }
            void capture_async(screenshot_callback /*on_complete*/) override
            {
                throw maui::application_model::feature_not_supported();
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_screenshot> make_screenshot()
        {
            return std::make_shared<apple_screenshot>();
        }
    } // namespace detail
} // namespace maui::media
