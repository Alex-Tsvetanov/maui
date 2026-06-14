// web_authenticator - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_comms_fakes.hpp), which unconfigured mirrors WebAuthenticator's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_web_authenticator.mm.

#include "maui/essentials/web_authenticator.hpp"

#include <memory>

#include "src/platform/headless/essentials_comms_fakes.hpp"

namespace maui::authentication::detail
{
    std::shared_ptr<i_web_authenticator> make_web_authenticator()
    {
        return std::make_shared<headless_web_authenticator>();
    }
} // namespace maui::authentication::detail
