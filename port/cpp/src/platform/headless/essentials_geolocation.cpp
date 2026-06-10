// geolocation - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_fakes.hpp), which unconfigured mirrors Geolocation's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_geolocation.mm.

#include "maui/essentials/geolocation.hpp"

#include <memory>

#include "src/platform/headless/essentials_fakes.hpp"

namespace maui::devices::sensors::detail
{
    std::shared_ptr<i_geolocation> make_geolocation()
    {
        return std::make_shared<headless_geolocation>();
    }
} // namespace maui::devices::sensors::detail
