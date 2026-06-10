// gyroscope - iOS (UIKit) platform partial. Ported 1:1 from Gyroscope.ios.watchos.cs: the per-TU
// CMMotionManager singleton, GyroUpdateInterval from the SensorSpeed table, RotationRate x/y/z in
// rad/s. Queue choice per the accelerometer partial's note. The SIMULATOR reports the sensor
// unavailable - lifecycle-only on-simulator tests; readings via the headless fake.

#import <CoreMotion/CoreMotion.h>
#import <Foundation/Foundation.h>

#include <chrono>
#include <memory>

#include "maui/essentials/gyroscope.hpp"
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

        class ios_gyroscope final : public detail::basic_sensor<i_gyroscope, gyroscope_data>
        {
        public:
            ios_gyroscope() : basic_sensor("Gyroscope")
            {
            }

        protected:
            [[nodiscard]] bool platform_is_supported() const override
            {
                return motion_manager().gyroAvailable;
            }

            void platform_start(sensor_speed speed) override
            {
                CMMotionManager* const manager = motion_manager();
                manager.gyroUpdateInterval = std::chrono::duration<double>(sensor_interval(speed)).count();
                NSOperationQueue* const queue = use_sync_context() ? [NSOperationQueue mainQueue] : background_queue();
                [manager startGyroUpdatesToQueue:queue
                                     withHandler:^(CMGyroData* data, NSError*) {
                                       if (data == nil)
                                       {
                                           return;
                                       }
                                       const CMRotationRate field = data.rotationRate;
                                       this->raise_reading_changed(gyroscope_data(static_cast<float>(field.x),
                                                                                  static_cast<float>(field.y),
                                                                                  static_cast<float>(field.z)));
                                     }];
            }

            void platform_stop() override
            {
                [motion_manager() stopGyroUpdates];
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_gyroscope> make_gyroscope()
        {
            return std::make_shared<ios_gyroscope>();
        }
    } // namespace detail
} // namespace maui::devices::sensors
