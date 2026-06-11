// The cross-platform half of the app_info facade: the lazily-created implementation slot behind
// AppInfo.Current / AppInfo.SetCurrent. The implementation itself is the per-backend partial
// (src/platform/<backend>/essentials_app_info.{cpp,mm}) via detail::make_app_info().

#include "maui/essentials/app_info.hpp"

#include <memory>
#include <utility>

namespace maui::application_model
{
    namespace
    {
        std::shared_ptr<i_app_info>& app_info_storage()
        {
            static std::shared_ptr<i_app_info> storage;
            return storage;
        }
    } // namespace

    i_app_info& app_info::current()
    {
        return *current_shared();
    }

    void app_info::set_current(std::shared_ptr<i_app_info> implementation)
    {
        app_info_storage() = std::move(implementation);
    }

    std::shared_ptr<i_app_info> app_info::current_shared()
    {
        auto& storage = app_info_storage();
        if (storage == nullptr)
        {
            storage = detail::make_app_info();
        }
        return storage;
    }
} // namespace maui::application_model
