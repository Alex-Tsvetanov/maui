// The cross-platform half of the geolocation facade: the lazily-created implementation slot behind
// Geolocation.Default / Geolocation.SetDefault. The implementation itself is the per-backend
// partial (src/platform/<backend>/essentials_geolocation.{cpp,mm}), reached through
// detail::make_geolocation() - the C# `defaultImplementation ??= new GeolocationImplementation()`.

#include "maui/essentials/geolocation.hpp"

#include <memory>
#include <utility>

namespace maui::devices::sensors
{
    namespace
    {
        std::shared_ptr<i_geolocation>& geolocation_storage()
        {
            static std::shared_ptr<i_geolocation> storage;
            return storage;
        }
    } // namespace

    i_geolocation& geolocation::default_()
    {
        auto& storage = geolocation_storage();
        if (storage == nullptr)
        {
            storage = detail::make_geolocation();
        }
        return *storage;
    }

    void geolocation::set_default(std::shared_ptr<i_geolocation> implementation)
    {
        geolocation_storage() = std::move(implementation);
    }
} // namespace maui::devices::sensors
