// app_actions - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_appmodel_fakes.hpp), which unconfigured mirrors AppActions's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_app_actions.mm.

#include "maui/essentials/app_actions.hpp"

#include <memory>

#include "src/platform/headless/essentials_appmodel_fakes.hpp"

namespace maui::application_model::detail
{
    std::shared_ptr<i_app_actions> make_app_actions()
    {
        return std::make_shared<headless_app_actions>();
    }
} // namespace maui::application_model::detail
