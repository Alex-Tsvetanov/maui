// browser - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_appmodel_fakes.hpp), which unconfigured mirrors Browser's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_browser.mm.

#include "maui/essentials/browser.hpp"

#include <memory>

#include "src/platform/headless/essentials_appmodel_fakes.hpp"

namespace maui::application_model::detail
{
    std::shared_ptr<i_browser> make_browser()
    {
        return std::make_shared<headless_browser>();
    }
} // namespace maui::application_model::detail
