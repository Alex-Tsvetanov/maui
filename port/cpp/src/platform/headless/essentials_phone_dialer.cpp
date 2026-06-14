// phone_dialer - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_comms_fakes.hpp), which unconfigured mirrors PhoneDialer's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_phone_dialer.mm.

#include "maui/essentials/phone_dialer.hpp"

#include <memory>

#include "src/platform/headless/essentials_comms_fakes.hpp"

namespace maui::application_model::communication::detail
{
    std::shared_ptr<i_phone_dialer> make_phone_dialer()
    {
        return std::make_shared<headless_phone_dialer>();
    }
} // namespace maui::application_model::communication::detail
