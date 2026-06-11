// The cross-platform half of the app_actions facade: the lazily-created implementation slot
// behind AppActions.Current / SetCurrent. The implementation itself is the per-backend partial
// (src/platform/<backend>/essentials_app_actions.{cpp,mm}) via detail::make_app_actions().

#include "maui/essentials/app_actions.hpp"

#include <memory>
#include <utility>

namespace maui::application_model
{
    namespace
    {
        std::shared_ptr<i_app_actions>& app_actions_storage()
        {
            static std::shared_ptr<i_app_actions> storage;
            return storage;
        }
    } // namespace

    i_app_actions& app_actions::current()
    {
        auto& storage = app_actions_storage();
        if (storage == nullptr)
        {
            storage = detail::make_app_actions();
        }
        return *storage;
    }

    void app_actions::set_current(std::shared_ptr<i_app_actions> implementation)
    {
        app_actions_storage() = std::move(implementation);
    }
} // namespace maui::application_model
