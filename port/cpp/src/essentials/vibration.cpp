// The cross-platform half of the vibration facade: the lazily-created implementation slot behind
// Vibration.Default / Vibration.SetDefault. The implementation itself is the per-backend
// partial (src/platform/<backend>/essentials_vibration.{cpp,mm}), reached through
// detail::make_vibration() - the C# `defaultImplementation ??= new VibrationImplementation()`.

#include "maui/essentials/vibration.hpp"

#include <memory>
#include <utility>

namespace maui::devices
{
    namespace
    {
        std::shared_ptr<i_vibration>& vibration_storage()
        {
            static std::shared_ptr<i_vibration> storage;
            return storage;
        }
    } // namespace

    i_vibration& vibration::default_()
    {
        auto& storage = vibration_storage();
        if (storage == nullptr)
        {
            storage = detail::make_vibration();
        }
        return *storage;
    }

    void vibration::set_default(std::shared_ptr<i_vibration> implementation)
    {
        vibration_storage() = std::move(implementation);
    }
} // namespace maui::devices
