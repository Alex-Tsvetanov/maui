// vibration - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_fakes.hpp), which unconfigured mirrors Vibration's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_vibration.mm.

#include "maui/essentials/vibration.hpp"

#include <memory>

#include "src/platform/headless/essentials_fakes.hpp"

namespace maui::devices::detail
{
    std::shared_ptr<i_vibration> make_vibration()
    {
        return std::make_shared<headless_vibration>();
    }
} // namespace maui::devices::detail
