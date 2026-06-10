// orientation_sensor - iOS (UIKit) platform partial. Ported 1:1 from
// OrientationSensor.ios.watchos.cs: device-motion updates against the fixed
// XTrueNorthZVertical reference frame (X north, Z vertical into the sky); availability gates on
// GyroAvailable (exactly as C# does); each attitude quaternion is pre-multiplied by a 90-degree Z
// rotation to move from the iOS frame (X north) to MAUI's earth frame (Y north, Z vertical).
// Queue choice per the accelerometer partial's note. The SIMULATOR reports the sensor unavailable
// - lifecycle-only on-simulator tests; readings via the headless fake.

#import <CoreMotion/CoreMotion.h>
#import <Foundation/Foundation.h>

#include <chrono>
#include <cmath>
#include <memory>
#include <numbers>

#include "maui/essentials/orientation_sensor.hpp"
#include "maui/essentials/sensor_types.hpp"
#include "maui/graphics/quaternion.hpp"

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

        class ios_orientation_sensor final : public detail::basic_sensor<i_orientation_sensor, orientation_sensor_data>
        {
        public:
            ios_orientation_sensor() : basic_sensor("Orientation sensor")
            {
            }

        protected:
            [[nodiscard]] bool platform_is_supported() const override
            {
                return motion_manager().gyroAvailable; // C#: PlatformIsSupported => GyroAvailable
            }

            void platform_start(sensor_speed speed) override
            {
                CMMotionManager* const manager = motion_manager();
                manager.deviceMotionUpdateInterval = std::chrono::duration<double>(sensor_interval(speed)).count();
                NSOperationQueue* const queue = use_sync_context() ? [NSOperationQueue mainQueue] : background_queue();
                // Use a fixed reference frame where X points north and Z points vertically into the sky.
                [manager
                    startDeviceMotionUpdatesUsingReferenceFrame:CMAttitudeReferenceFrameXTrueNorthZVertical
                                                        toQueue:queue
                                                    withHandler:^(CMDeviceMotion* data, NSError*) {
                                                      if (data == nil)
                                                      {
                                                          return;
                                                      }
                                                      const CMQuaternion field = data.attitude.quaternion;
                                                      const maui::graphics::quaternion q(
                                                          static_cast<float>(field.x), static_cast<float>(field.y),
                                                          static_cast<float>(field.z), static_cast<float>(field.w));
                                                      // Rotate 90 degrees around Z to go from MAUI's frame
                                                      // (Y north) to the iOS frame (X north), then apply the
                                                      // attitude: earth-frame(MAUI) -> phone frame.
                                                      const auto half_angle =
                                                          static_cast<float>(std::numbers::pi / 4.0);
                                                      const maui::graphics::quaternion qz90(0, 0, std::sin(half_angle),
                                                                                            std::cos(half_angle));
                                                      const maui::graphics::quaternion rotated =
                                                          maui::graphics::quaternion::multiply(qz90, q);
                                                      this->raise_reading_changed(orientation_sensor_data(
                                                          rotated.x, rotated.y, rotated.z, rotated.w));
                                                    }];
            }

            void platform_stop() override
            {
                [motion_manager() stopDeviceMotionUpdates];
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_orientation_sensor> make_orientation_sensor()
        {
            return std::make_shared<ios_orientation_sensor>();
        }
    } // namespace detail
} // namespace maui::devices::sensors
