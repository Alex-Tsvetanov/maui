// sms - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_comms_fakes.hpp), which unconfigured mirrors Sms's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_sms.mm.

#include "maui/essentials/sms.hpp"

#include <memory>

#include "src/platform/headless/essentials_comms_fakes.hpp"

namespace maui::application_model::communication::detail
{
    std::shared_ptr<i_sms> make_sms()
    {
        return std::make_shared<headless_sms>();
    }
} // namespace maui::application_model::communication::detail
