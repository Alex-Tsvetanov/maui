// main_thread - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_appmodel_fakes.hpp), which unconfigured mirrors MainThread's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_main_thread.mm.

#include "maui/essentials/main_thread.hpp"

#include <memory>

#include "src/platform/headless/essentials_appmodel_fakes.hpp"

namespace maui::application_model::detail
{
    std::shared_ptr<i_main_thread> make_main_thread()
    {
        return std::make_shared<headless_main_thread>();
    }
} // namespace maui::application_model::detail
