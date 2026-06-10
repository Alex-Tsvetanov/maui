// The cross-platform half of the orientation_sensor facade: the lazily-created implementation slot behind
// OrientationSensor.Default / OrientationSensor.SetDefault. The implementation itself is the per-backend
// partial (src/platform/<backend>/essentials_orientation_sensor.{cpp,mm}), reached through
// detail::make_orientation_sensor() - the C# `defaultImplementation ??= new OrientationSensorImplementation()`.

#include "maui/essentials/orientation_sensor.hpp"

#include <memory>
#include <utility>

namespace maui::devices::sensors
{
    namespace
    {
        std::shared_ptr<i_orientation_sensor>& orientation_sensor_storage()
        {
            static std::shared_ptr<i_orientation_sensor> storage;
            return storage;
        }
    } // namespace

    i_orientation_sensor& orientation_sensor::default_()
    {
        auto& storage = orientation_sensor_storage();
        if (storage == nullptr)
        {
            storage = detail::make_orientation_sensor();
        }
        return *storage;
    }

    void orientation_sensor::set_default(std::shared_ptr<i_orientation_sensor> implementation)
    {
        orientation_sensor_storage() = std::move(implementation);
    }
} // namespace maui::devices::sensors
