// permissions - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_appmodel_fakes.hpp), which unconfigured mirrors Permissions' netstandard partial
// (every member throws). The real-device twins are
// src/platform/{apple,ios}/essentials_permissions.mm.

#include "maui/essentials/permissions.hpp"

#include <memory>

#include "src/platform/headless/essentials_appmodel_fakes.hpp"

namespace maui::application_model::detail
{
    std::shared_ptr<i_permission_backend> make_permission_backend()
    {
        return std::make_shared<headless_permission_backend>();
    }
} // namespace maui::application_model::detail
