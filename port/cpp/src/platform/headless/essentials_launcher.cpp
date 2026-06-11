// launcher - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_appmodel_fakes.hpp), which unconfigured mirrors Launcher's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_launcher.mm.

#include "maui/essentials/launcher.hpp"

#include <memory>

#include "src/platform/headless/essentials_appmodel_fakes.hpp"

namespace maui::application_model::detail
{
    std::shared_ptr<i_launcher> make_launcher()
    {
        return std::make_shared<headless_launcher>();
    }
} // namespace maui::application_model::detail
