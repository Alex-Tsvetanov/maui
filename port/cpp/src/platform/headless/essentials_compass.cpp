// compass - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_fakes.hpp), which unconfigured mirrors Compass's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_compass.mm.

#include "maui/essentials/compass.hpp"

#include <memory>

#include "src/platform/headless/essentials_fakes.hpp"

namespace maui::devices::sensors::detail
{
    std::shared_ptr<i_compass> make_compass()
    {
        return std::make_shared<headless_compass>();
    }
} // namespace maui::devices::sensors::detail
