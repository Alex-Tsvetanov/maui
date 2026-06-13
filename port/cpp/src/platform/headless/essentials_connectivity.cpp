// connectivity - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_fakes.hpp), which unconfigured mirrors Connectivity's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_connectivity.mm.

#include "maui/essentials/connectivity.hpp"

#include <memory>

#include "src/platform/headless/essentials_fakes.hpp"

namespace maui::networking::detail
{
    std::shared_ptr<i_connectivity> make_connectivity()
    {
        return std::make_shared<headless_connectivity>();
    }
} // namespace maui::networking::detail
