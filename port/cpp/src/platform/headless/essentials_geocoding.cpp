// geocoding - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_fakes.hpp), which unconfigured mirrors Geocoding's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_geocoding.mm.

#include "maui/essentials/geocoding.hpp"

#include <memory>

#include "src/platform/headless/essentials_fakes.hpp"

namespace maui::devices::sensors::detail
{
    std::shared_ptr<i_geocoding> make_geocoding()
    {
        return std::make_shared<headless_geocoding>();
    }
} // namespace maui::devices::sensors::detail
