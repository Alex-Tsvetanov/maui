// The cross-platform half of the preferences facade: the lazily-created implementation slot
// behind Preferences.Default / Preferences.SetDefault, plus GetPrivatePreferencesSharedName (the
// "{PackageName}.microsoft.maui.essentials.{feature}" container-name helper version_tracking and
// secure_storage build on). The implementation itself is the per-backend partial
// (src/platform/<backend>/essentials_preferences.{cpp,mm}) via detail::make_preferences().

#include "maui/essentials/preferences.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "maui/essentials/app_info.hpp"

namespace maui::storage
{
    namespace
    {
        std::shared_ptr<i_preferences>& preferences_storage()
        {
            static std::shared_ptr<i_preferences> storage;
            return storage;
        }
    } // namespace

    i_preferences& preferences::default_()
    {
        return *default_shared();
    }

    void preferences::set_default(std::shared_ptr<i_preferences> implementation)
    {
        preferences_storage() = std::move(implementation);
    }

    std::shared_ptr<i_preferences> preferences::default_shared()
    {
        auto& storage = preferences_storage();
        if (storage == nullptr)
        {
            storage = detail::make_preferences();
        }
        return storage;
    }

    namespace detail
    {
        std::string private_preferences_shared_name(std::string_view feature)
        {
            std::string result = maui::application_model::app_info::package_name();
            result += ".microsoft.maui.essentials.";
            result += feature;
            return result;
        }
    } // namespace detail
} // namespace maui::storage
