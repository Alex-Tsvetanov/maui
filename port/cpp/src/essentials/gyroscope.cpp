// The cross-platform half of the gyroscope facade: the lazily-created implementation slot behind
// Gyroscope.Default / Gyroscope.SetDefault. The implementation itself is the per-backend
// partial (src/platform/<backend>/essentials_gyroscope.{cpp,mm}), reached through
// detail::make_gyroscope() - the C# `defaultImplementation ??= new GyroscopeImplementation()`.

#include "maui/essentials/gyroscope.hpp"

#include <memory>
#include <utility>

namespace maui::devices::sensors
{
    namespace
    {
        std::shared_ptr<i_gyroscope>& gyroscope_storage()
        {
            static std::shared_ptr<i_gyroscope> storage;
            return storage;
        }
    } // namespace

    i_gyroscope& gyroscope::default_()
    {
        auto& storage = gyroscope_storage();
        if (storage == nullptr)
        {
            storage = detail::make_gyroscope();
        }
        return *storage;
    }

    void gyroscope::set_default(std::shared_ptr<i_gyroscope> implementation)
    {
        gyroscope_storage() = std::move(implementation);
    }
} // namespace maui::devices::sensors
