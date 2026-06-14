// email - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_comms_fakes.hpp), which unconfigured mirrors Email's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_email.mm.

#include "maui/essentials/email.hpp"

#include <memory>

#include "src/platform/headless/essentials_comms_fakes.hpp"

namespace maui::application_model::communication::detail
{
    std::shared_ptr<i_email> make_email()
    {
        return std::make_shared<headless_email>();
    }
} // namespace maui::application_model::communication::detail
