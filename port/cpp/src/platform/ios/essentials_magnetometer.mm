// magnetometer - iOS (UIKit) platform partial. Ported 1:1 from Magnetometer.ios.watchos.cs: the
// per-TU CMMotionManager singleton, MagnetometerUpdateInterval from the SensorSpeed table,
// MagneticField x/y/z in microteslas. Queue choice per the accelerometer partial's note. The
// SIMULATOR reports the sensor unavailable - lifecycle-only on-simulator tests; readings via the
// headless fake.

#import <CoreMotion/CoreMotion.h>
#import <Foundation/Foundation.h>

#include <chrono>
#include <memory>

#include "maui/essentials/magnetometer.hpp"
#include "maui/essentials/sensor_types.hpp"

#include "src/essentials/detail/sensor_base.hpp"

namespace maui::devices::sensors
{
    namespace
    {
        CMMotionManager* motion_manager()
        {
            static CMMotionManager* const manager = [[CMMotionManager alloc] init];
            return manager;
        }

        NSOperationQueue* background_queue()
        {
            static NSOperationQueue* const queue = [[NSOperationQueue alloc] init];
            return queue;
        }

        class ios_magnetometer final : public detail::basic_sensor<i_magnetometer, magnetometer_data>
        {
        public:
            ios_magnetometer() : basic_sensor("Magnetometer")
            {
            }

        protected:
            [[nodiscard]] bool platform_is_supported() const override
            {
                return motion_manager().magnetometerAvailable;
            }

            void platform_start(sensor_speed speed) override
            {
                CMMotionManager* const manager = motion_manager();
                manager.magnetometerUpdateInterval = std::chrono::duration<double>(sensor_interval(speed)).count();
                NSOperationQueue* const queue = use_sync_context() ? [NSOperationQueue mainQueue] : background_queue();
                [manager startMagnetometerUpdatesToQueue:queue
                                             withHandler:^(CMMagnetometerData* data, NSError*) {
                                               if (data == nil)
                                               {
                                                   return;
                                               }
                                               const CMMagneticField field = data.magneticField;
                                               this->raise_reading_changed(magnetometer_data(
                                                   static_cast<float>(field.x), static_cast<float>(field.y),
                                                   static_cast<float>(field.z)));
                                             }];
            }

            void platform_stop() override
            {
                [motion_manager() stopMagnetometerUpdates];
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_magnetometer> make_magnetometer()
        {
            return std::make_shared<ios_magnetometer>();
        }
    } // namespace detail
} // namespace maui::devices::sensors
