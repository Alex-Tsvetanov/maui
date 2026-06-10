// gyroscope - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_fakes.hpp), which unconfigured mirrors Gyroscope's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_gyroscope.mm.

#include "maui/essentials/gyroscope.hpp"

#include <memory>

#include "src/platform/headless/essentials_fakes.hpp"

namespace maui::devices::sensors::detail
{
    std::shared_ptr<i_gyroscope> make_gyroscope()
    {
        return std::make_shared<headless_gyroscope>();
    }
} // namespace maui::devices::sensors::detail
