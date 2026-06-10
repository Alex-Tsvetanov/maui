// barometer - iOS (UIKit) platform partial. Ported 1:1 from Barometer.ios.watchos.cs: a CMAltimeter
// created per start (and disposed on stop), availability via IsRelativeAltitudeAvailable, and each
// CMAltitudeData pressure (kPa) converted to hectopascals (UnitConverters.KilopascalsToHectopascals
// = * 10). Queue choice per the accelerometer partial's note. The SIMULATOR reports the altimeter
// unavailable - lifecycle-only on-simulator tests; readings via the headless fake.

#import <CoreMotion/CoreMotion.h>
#import <Foundation/Foundation.h>

#include <memory>

#include "maui/essentials/barometer.hpp"
#include "maui/essentials/sensor_types.hpp"

#include "src/essentials/detail/sensor_base.hpp"

namespace maui::devices::sensors
{
    namespace
    {
        NSOperationQueue* background_queue()
        {
            static NSOperationQueue* const queue = [[NSOperationQueue alloc] init];
            return queue;
        }

        class ios_barometer final : public detail::basic_sensor<i_barometer, barometer_data>
        {
        public:
            ios_barometer() : basic_sensor("Barometer")
            {
            }

        protected:
            [[nodiscard]] bool platform_is_supported() const override
            {
                return [CMAltimeter isRelativeAltitudeAvailable];
            }

            void platform_start(sensor_speed speed) override
            {
                static_cast<void>(speed); // CMAltimeter has no configurable rate (C# ignores it too)
                altitude_manager_ = [[CMAltimeter alloc] init];
                NSOperationQueue* const queue = use_sync_context() ? [NSOperationQueue mainQueue] : background_queue();
                [altitude_manager_ startRelativeAltitudeUpdatesToQueue:queue
                                                           withHandler:^(CMAltitudeData* data, NSError*) {
                                                             if (data == nil)
                                                             {
                                                                 return;
                                                             }
                                                             // KilopascalsToHectopascals(pressure).
                                                             this->raise_reading_changed(
                                                                 barometer_data(data.pressure.doubleValue * 10.0));
                                                           }];
            }

            void platform_stop() override
            {
                if (altitude_manager_ == nil)
                {
                    return;
                }
                [altitude_manager_ stopRelativeAltitudeUpdates];
                altitude_manager_ = nil;
            }

        private:
            CMAltimeter* altitude_manager_ = nil; // ARC-managed strong reference
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_barometer> make_barometer()
        {
            return std::make_shared<ios_barometer>();
        }
    } // namespace detail
} // namespace maui::devices::sensors
