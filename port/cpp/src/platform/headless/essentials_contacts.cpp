// contacts - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_comms_fakes.hpp), which unconfigured mirrors Contacts's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_contacts.mm.

#include "maui/essentials/contacts.hpp"

#include <memory>

#include "src/platform/headless/essentials_comms_fakes.hpp"

namespace maui::application_model::communication::detail
{
    std::shared_ptr<i_contacts> make_contacts()
    {
        return std::make_shared<headless_contacts>();
    }
} // namespace maui::application_model::communication::detail
