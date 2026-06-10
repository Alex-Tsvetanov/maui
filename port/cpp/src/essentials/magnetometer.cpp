// The cross-platform half of the magnetometer facade: the lazily-created implementation slot behind
// Magnetometer.Default / Magnetometer.SetDefault. The implementation itself is the per-backend
// partial (src/platform/<backend>/essentials_magnetometer.{cpp,mm}), reached through
// detail::make_magnetometer() - the C# `defaultImplementation ??= new MagnetometerImplementation()`.

#include "maui/essentials/magnetometer.hpp"

#include <memory>
#include <utility>

namespace maui::devices::sensors
{
    namespace
    {
        std::shared_ptr<i_magnetometer>& magnetometer_storage()
        {
            static std::shared_ptr<i_magnetometer> storage;
            return storage;
        }
    } // namespace

    i_magnetometer& magnetometer::default_()
    {
        auto& storage = magnetometer_storage();
        if (storage == nullptr)
        {
            storage = detail::make_magnetometer();
        }
        return *storage;
    }

    void magnetometer::set_default(std::shared_ptr<i_magnetometer> implementation)
    {
        magnetometer_storage() = std::move(implementation);
    }
} // namespace maui::devices::sensors
