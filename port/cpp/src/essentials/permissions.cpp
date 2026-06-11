// The cross-platform half of the permissions facade: the lazily-created backend slot behind
// permissions::backend() / set_backend (the port's injectable replacement for C#'s
// per-platform BasePlatformPermission partials - and the fake-grantable test seam). The backend
// itself is the per-backend partial (src/platform/<backend>/essentials_permissions.{cpp,mm}) via
// detail::make_permission_backend().

#include "maui/essentials/permissions.hpp"

#include <memory>
#include <utility>

namespace maui::application_model
{
    namespace
    {
        std::shared_ptr<i_permission_backend>& permission_backend_storage()
        {
            static std::shared_ptr<i_permission_backend> storage;
            return storage;
        }
    } // namespace

    i_permission_backend& permissions::backend()
    {
        auto& storage = permission_backend_storage();
        if (storage == nullptr)
        {
            storage = detail::make_permission_backend();
        }
        return *storage;
    }

    void permissions::set_backend(std::shared_ptr<i_permission_backend> implementation)
    {
        permission_backend_storage() = std::move(implementation);
    }
} // namespace maui::application_model
