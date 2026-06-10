// The cross-platform half of the barometer facade: the lazily-created implementation slot behind
// Barometer.Default / Barometer.SetDefault. The implementation itself is the per-backend
// partial (src/platform/<backend>/essentials_barometer.{cpp,mm}), reached through
// detail::make_barometer() - the C# `defaultImplementation ??= new BarometerImplementation()`.

#include "maui/essentials/barometer.hpp"

#include <memory>
#include <utility>

namespace maui::devices::sensors
{
    namespace
    {
        std::shared_ptr<i_barometer>& barometer_storage()
        {
            static std::shared_ptr<i_barometer> storage;
            return storage;
        }
    } // namespace

    i_barometer& barometer::default_()
    {
        auto& storage = barometer_storage();
        if (storage == nullptr)
        {
            storage = detail::make_barometer();
        }
        return *storage;
    }

    void barometer::set_default(std::shared_ptr<i_barometer> implementation)
    {
        barometer_storage() = std::move(implementation);
    }
} // namespace maui::devices::sensors
