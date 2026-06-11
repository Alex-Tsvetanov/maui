// secure_storage - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_appmodel_fakes.hpp), which unconfigured mirrors SecureStorage's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_secure_storage.mm.

#include "maui/essentials/secure_storage.hpp"

#include <memory>

#include "src/platform/headless/essentials_appmodel_fakes.hpp"

namespace maui::storage::detail
{
    std::shared_ptr<i_secure_storage> make_secure_storage()
    {
        return std::make_shared<headless_secure_storage>();
    }
} // namespace maui::storage::detail
