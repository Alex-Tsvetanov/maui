// battery - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_fakes.hpp), which unconfigured mirrors Battery's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_battery.mm.

#include "maui/essentials/battery.hpp"

#include <memory>

#include "src/platform/headless/essentials_fakes.hpp"

namespace maui::devices::detail
{
    std::shared_ptr<i_battery> make_battery()
    {
        return std::make_shared<headless_battery>();
    }
} // namespace maui::devices::detail
