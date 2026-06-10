// The cross-platform half of the battery facade: the lazily-created implementation slot behind
// Battery.Default / Battery.SetDefault. The implementation itself is the per-backend
// partial (src/platform/<backend>/essentials_battery.{cpp,mm}), reached through
// detail::make_battery() - the C# `defaultImplementation ??= new BatteryImplementation()`.

#include "maui/essentials/battery.hpp"

#include <memory>
#include <utility>

namespace maui::devices
{
    namespace
    {
        std::shared_ptr<i_battery>& battery_storage()
        {
            static std::shared_ptr<i_battery> storage;
            return storage;
        }
    } // namespace

    i_battery& battery::default_()
    {
        auto& storage = battery_storage();
        if (storage == nullptr)
        {
            storage = detail::make_battery();
        }
        return *storage;
    }

    void battery::set_default(std::shared_ptr<i_battery> implementation)
    {
        battery_storage() = std::move(implementation);
    }
} // namespace maui::devices
