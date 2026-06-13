// media_picker - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_fakes.hpp), which unconfigured mirrors MediaPicker's netstandard partial (every
// member throws). The real-device twins are src/platform/{apple,ios}/essentials_media_picker.mm
// (a service seam - the picker UI is not drivable in the spawned sim process).

#include "maui/essentials/media_picker.hpp"

#include <memory>

#include "src/platform/headless/essentials_fakes.hpp"

namespace maui::media::detail
{
    std::shared_ptr<i_media_picker> make_media_picker()
    {
        return std::make_shared<headless_media_picker>();
    }
} // namespace maui::media::detail
