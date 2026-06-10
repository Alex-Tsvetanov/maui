// orientation_sensor - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_fakes.hpp), which unconfigured mirrors OrientationSensor's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_orientation_sensor.mm.

#include "maui/essentials/orientation_sensor.hpp"

#include <memory>

#include "src/platform/headless/essentials_fakes.hpp"

namespace maui::devices::sensors::detail
{
    std::shared_ptr<i_orientation_sensor> make_orientation_sensor()
    {
        return std::make_shared<headless_orientation_sensor>();
    }
} // namespace maui::devices::sensors::detail
