// device_display - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_fakes.hpp), which unconfigured mirrors DeviceDisplay's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_device_display.mm.

#include "maui/essentials/device_display.hpp"

#include <memory>

#include "src/platform/headless/essentials_fakes.hpp"

namespace maui::devices::detail
{
    std::shared_ptr<i_device_display> make_device_display()
    {
        return std::make_shared<headless_device_display>();
    }
} // namespace maui::devices::detail
