// The cross-platform half of the compass facade: the lazily-created implementation slot behind
// Compass.Default / Compass.SetDefault. The implementation itself is the per-backend
// partial (src/platform/<backend>/essentials_compass.{cpp,mm}), reached through
// detail::make_compass() - the C# `defaultImplementation ??= new CompassImplementation()`.

#include "maui/essentials/compass.hpp"

#include <memory>
#include <utility>

namespace maui::devices::sensors
{
    namespace
    {
        std::shared_ptr<i_compass>& compass_storage()
        {
            static std::shared_ptr<i_compass> storage;
            return storage;
        }
    } // namespace

    i_compass& compass::default_()
    {
        auto& storage = compass_storage();
        if (storage == nullptr)
        {
            storage = detail::make_compass();
        }
        return *storage;
    }

    void compass::set_default(std::shared_ptr<i_compass> implementation)
    {
        compass_storage() = std::move(implementation);
    }
} // namespace maui::devices::sensors
