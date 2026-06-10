// accelerometer - iOS (UIKit) platform partial. Ported 1:1 from Accelerometer.ios.watchos.cs:
// a per-TU CMMotionManager singleton (the C# static), AccelerometerUpdateInterval from the
// SensorSpeed timing table, and the CoreMotion callback negates each axis (CoreMotion reports the
// inverse of MAUI's G convention). C# delivers on NSOperationQueue.CurrentQueue ?? new and then
// marshals Default/UI raises to the main thread; the port delivers straight on the main queue for
// those speeds and on a dedicated background queue otherwise - same observable delivery. NOTE the
// SIMULATOR reports the sensor unavailable and delivers no data; readings are tested through the
// headless fake, the on-simulator tests assert the lifecycle contract only.

#import <CoreMotion/CoreMotion.h>
#import <Foundation/Foundation.h>

#include <chrono>
#include <memory>

#include "maui/essentials/accelerometer.hpp"
#include "maui/essentials/sensor_types.hpp"

#include "src/essentials/detail/accelerometer_base.hpp"

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

        class ios_accelerometer final : public detail::accelerometer_base
        {
        protected:
            [[nodiscard]] bool platform_is_supported() const override
            {
                return motion_manager().accelerometerAvailable;
            }

            void platform_start(sensor_speed speed) override
            {
                CMMotionManager* const manager = motion_manager();
                manager.accelerometerUpdateInterval = std::chrono::duration<double>(sensor_interval(speed)).count();
                NSOperationQueue* const queue = use_sync_context() ? [NSOperationQueue mainQueue] : background_queue();
                [manager startAccelerometerUpdatesToQueue:queue
                                              withHandler:^(CMAccelerometerData* data, NSError*) {
                                                if (data == nil)
                                                {
                                                    return;
                                                }
                                                const CMAcceleration field = data.acceleration;
                                                this->on_changed(accelerometer_data(static_cast<float>(field.x * -1),
                                                                                    static_cast<float>(field.y * -1),
                                                                                    static_cast<float>(field.z * -1)));
                                              }];
            }

            void platform_stop() override
            {
                [motion_manager() stopAccelerometerUpdates];
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_accelerometer> make_accelerometer()
        {
            return std::make_shared<ios_accelerometer>();
        }
    } // namespace detail
} // namespace maui::devices::sensors
