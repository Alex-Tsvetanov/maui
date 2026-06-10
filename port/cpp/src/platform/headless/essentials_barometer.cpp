// barometer - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_fakes.hpp), which unconfigured mirrors Barometer's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_barometer.mm.

#include "maui/essentials/barometer.hpp"

#include <memory>

#include "src/platform/headless/essentials_fakes.hpp"

namespace maui::devices::sensors::detail
{
    std::shared_ptr<i_barometer> make_barometer()
    {
        return std::make_shared<headless_barometer>();
    }
} // namespace maui::devices::sensors::detail
