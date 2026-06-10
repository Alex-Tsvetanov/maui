// accelerometer - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_fakes.hpp), which unconfigured mirrors Accelerometer's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_accelerometer.mm.

#include "maui/essentials/accelerometer.hpp"

#include <memory>

#include "src/platform/headless/essentials_fakes.hpp"

namespace maui::devices::sensors::detail
{
    std::shared_ptr<i_accelerometer> make_accelerometer()
    {
        return std::make_shared<headless_accelerometer>();
    }
} // namespace maui::devices::sensors::detail
