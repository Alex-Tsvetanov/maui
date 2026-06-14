// clipboard - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_comms_fakes.hpp), which unconfigured mirrors Clipboard's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_clipboard.mm.

#include "maui/essentials/clipboard.hpp"

#include <memory>

#include "src/platform/headless/essentials_comms_fakes.hpp"

namespace maui::application_model::detail
{
    std::shared_ptr<i_clipboard> make_clipboard()
    {
        return std::make_shared<headless_clipboard>();
    }
} // namespace maui::application_model::detail
