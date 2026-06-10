// The cross-platform half of the flashlight facade: the lazily-created implementation slot behind
// Flashlight.Default / Flashlight.SetDefault. The implementation itself is the per-backend
// partial (src/platform/<backend>/essentials_flashlight.{cpp,mm}), reached through
// detail::make_flashlight() - the C# `defaultImplementation ??= new FlashlightImplementation()`.

#include "maui/essentials/flashlight.hpp"

#include <memory>
#include <utility>

namespace maui::devices
{
    namespace
    {
        std::shared_ptr<i_flashlight>& flashlight_storage()
        {
            static std::shared_ptr<i_flashlight> storage;
            return storage;
        }
    } // namespace

    i_flashlight& flashlight::default_()
    {
        auto& storage = flashlight_storage();
        if (storage == nullptr)
        {
            storage = detail::make_flashlight();
        }
        return *storage;
    }

    void flashlight::set_default(std::shared_ptr<i_flashlight> implementation)
    {
        flashlight_storage() = std::move(implementation);
    }
} // namespace maui::devices
