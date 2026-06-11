// app_info - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_appmodel_fakes.hpp), which unconfigured mirrors AppInfo's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_app_info.mm.

#include "maui/essentials/app_info.hpp"

#include <memory>

#include "src/platform/headless/essentials_appmodel_fakes.hpp"

namespace maui::application_model::detail
{
    std::shared_ptr<i_app_info> make_app_info()
    {
        return std::make_shared<headless_app_info>();
    }
} // namespace maui::application_model::detail
