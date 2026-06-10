// magnetometer - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_fakes.hpp), which unconfigured mirrors Magnetometer's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_magnetometer.mm.

#include "maui/essentials/magnetometer.hpp"

#include <memory>

#include "src/platform/headless/essentials_fakes.hpp"

namespace maui::devices::sensors::detail
{
    std::shared_ptr<i_magnetometer> make_magnetometer()
    {
        return std::make_shared<headless_magnetometer>();
    }
} // namespace maui::devices::sensors::detail
