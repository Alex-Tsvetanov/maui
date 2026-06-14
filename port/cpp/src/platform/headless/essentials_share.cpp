// share - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_comms_fakes.hpp), which unconfigured mirrors Share's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_share.mm.

#include "maui/essentials/share.hpp"

#include <memory>

#include "src/platform/headless/essentials_comms_fakes.hpp"

namespace maui::application_model::data_transfer::detail
{
    std::shared_ptr<i_share> make_share()
    {
        return std::make_shared<headless_share>();
    }
} // namespace maui::application_model::data_transfer::detail
