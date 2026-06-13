// The cross-platform half of the screenshot facade: the lazily-created implementation slot behind
// Screenshot.Default / Screenshot.SetDefault, plus the CaptureAsync feature-support gate. The
// implementation itself is the per-backend partial
// (src/platform/<backend>/essentials_screenshot.{cpp,mm}), reached through detail::make_screenshot()
// - the C# `defaultImplementation ??= new ScreenshotImplementation()`.

#include "maui/essentials/screenshot.hpp"

#include <memory>
#include <utility>

#include "maui/essentials/feature_not_supported.hpp"

namespace maui::media
{
    namespace
    {
        std::shared_ptr<i_screenshot>& screenshot_storage()
        {
            static std::shared_ptr<i_screenshot> storage;
            return storage;
        }
    } // namespace

    void screenshot::capture_async(screenshot_callback on_complete)
    {
        // Screenshot.CaptureAsync: throw before delegating when capture is unsupported.
        if (!is_capture_supported())
        {
            throw maui::application_model::feature_not_supported();
        }
        default_().capture_async(std::move(on_complete));
    }

    i_screenshot& screenshot::default_()
    {
        auto& storage = screenshot_storage();
        if (storage == nullptr)
        {
            storage = detail::make_screenshot();
        }
        return *storage;
    }

    void screenshot::set_default(std::shared_ptr<i_screenshot> implementation)
    {
        screenshot_storage() = std::move(implementation);
    }
} // namespace maui::media
