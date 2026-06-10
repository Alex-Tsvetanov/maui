// flashlight - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_fakes.hpp), which unconfigured mirrors Flashlight's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_flashlight.mm.

#include "maui/essentials/flashlight.hpp"

#include <memory>

#include "src/platform/headless/essentials_fakes.hpp"

namespace maui::devices::detail
{
    std::shared_ptr<i_flashlight> make_flashlight()
    {
        return std::make_shared<headless_flashlight>();
    }
} // namespace maui::devices::detail
