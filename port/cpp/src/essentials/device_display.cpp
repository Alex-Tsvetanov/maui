// The cross-platform half of the device_display facade: the lazily-created implementation slot behind
// DeviceDisplay.Current / DeviceDisplay.SetCurrent. The implementation itself is the per-backend
// partial (src/platform/<backend>/essentials_device_display.{cpp,mm}), reached through
// detail::make_device_display() - the C# `currentImplementation ??= new DeviceDisplayImplementation()`.

#include "maui/essentials/device_display.hpp"

#include <memory>
#include <utility>

namespace maui::devices
{
    namespace
    {
        std::shared_ptr<i_device_display>& device_display_storage()
        {
            static std::shared_ptr<i_device_display> storage;
            return storage;
        }
    } // namespace

    i_device_display& device_display::current()
    {
        auto& storage = device_display_storage();
        if (storage == nullptr)
        {
            storage = detail::make_device_display();
        }
        return *storage;
    }

    void device_display::set_current(std::shared_ptr<i_device_display> implementation)
    {
        device_display_storage() = std::move(implementation);
    }
} // namespace maui::devices
