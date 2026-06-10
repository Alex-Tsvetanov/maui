// device_info - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_fakes.hpp), which unconfigured mirrors DeviceInfo's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_device_info.mm.

#include "maui/essentials/device_info.hpp"

#include <memory>

#include "src/platform/headless/essentials_fakes.hpp"

namespace maui::devices::detail
{
    std::shared_ptr<i_device_info> make_device_info()
    {
        return std::make_shared<headless_device_info>();
    }
} // namespace maui::devices::detail
