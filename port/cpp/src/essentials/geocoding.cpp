// The cross-platform half of the geocoding facade: the lazily-created implementation slot behind
// Geocoding.Default / Geocoding.SetCurrent. The implementation itself is the per-backend
// partial (src/platform/<backend>/essentials_geocoding.{cpp,mm}), reached through
// detail::make_geocoding() - the C# `defaultImplementation ??= new GeocodingImplementation()`.

#include "maui/essentials/geocoding.hpp"

#include <memory>
#include <utility>

namespace maui::devices::sensors
{
    namespace
    {
        std::shared_ptr<i_geocoding>& geocoding_storage()
        {
            static std::shared_ptr<i_geocoding> storage;
            return storage;
        }
    } // namespace

    i_geocoding& geocoding::default_()
    {
        auto& storage = geocoding_storage();
        if (storage == nullptr)
        {
            storage = detail::make_geocoding();
        }
        return *storage;
    }

    void geocoding::set_current(std::shared_ptr<i_geocoding> implementation)
    {
        geocoding_storage() = std::move(implementation);
    }
} // namespace maui::devices::sensors
