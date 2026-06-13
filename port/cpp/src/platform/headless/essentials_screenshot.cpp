// screenshot - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_fakes.hpp), which unconfigured mirrors Screenshot's netstandard/macos partial (both
// members throw). The real-device twin is src/platform/ios/essentials_screenshot.mm; macOS is NOT
// supported (no shared partial), so the apple backend reuses this not-supported behavior via
// src/platform/apple/essentials_screenshot.mm.

#include "maui/essentials/screenshot.hpp"

#include <memory>

#include "src/platform/headless/essentials_fakes.hpp"

namespace maui::media::detail
{
    std::shared_ptr<i_screenshot> make_screenshot()
    {
        return std::make_shared<headless_screenshot>();
    }
} // namespace maui::media::detail
