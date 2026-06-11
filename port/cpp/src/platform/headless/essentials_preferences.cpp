// preferences - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_appmodel_fakes.hpp), which unconfigured mirrors Preferences's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_preferences.mm.

#include "maui/essentials/preferences.hpp"

#include <memory>

#include "src/platform/headless/essentials_appmodel_fakes.hpp"

namespace maui::storage::detail
{
    std::shared_ptr<i_preferences> make_preferences()
    {
        return std::make_shared<headless_preferences>();
    }
} // namespace maui::storage::detail
