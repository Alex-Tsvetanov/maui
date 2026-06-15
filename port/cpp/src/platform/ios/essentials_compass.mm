// compass - iOS (UIKit) platform partial. Ported 1:1 from Compass.ios.cs: availability via
// CLLocationManager.headingAvailable; a CLLocationManager created per start with the per-speed
// HeadingFilter / DesiredAccuracy pairs (Fastest .01/BestForNavigation, Game .5/BestForNavigation,
// Default 1/Best, UI 2/Best - the angular-distance filters aligned with Windows); each heading
// update raises CompassData(magneticHeading). IPlatformCompass.ShouldDisplayHeadingCalibration is
// ported: ios_compass tracks the bool (default false) and the listener's
// -locationManagerShouldDisplayHeadingCalibration: returns it, mirroring the C#
// LocationManagerShouldDisplayHeadingCalibration handler. The value is stored on the compass (base
// field) so it survives across start/stop, and the listener queries it through a calibration block
// torn down in platform_stop()/the destructor - same teardown discipline as headingHandler, so a
// destroyed compass never gets the callback (no UAF). The applyLowPassFilter flag is Android-only
// and ignored here, exactly like the C# partial. The SIMULATOR reports heading unavailable -
// lifecycle/property-only on-simulator tests; readings via the headless fake.

#import <CoreLocation/CoreLocation.h>
#import <Foundation/Foundation.h>

#include <memory>

#include "maui/essentials/compass.hpp"
#include "maui/essentials/sensor_types.hpp"

#include "src/essentials/detail/compass_base.hpp"

// Forwards the CLLocationManager heading callbacks to the C++ partial. Both blocks are cleared in
// platform_stop() (which the destructor also runs) before the compass can be destroyed, so neither
// callback can reach a dangling partial.
@interface MauiIosCompassListener : NSObject <CLLocationManagerDelegate>
@property(nonatomic, copy) void (^headingHandler)(double);
@property(nonatomic, copy) BOOL (^calibrationProvider)(void);
@end

@implementation MauiIosCompassListener
- (void)locationManager:(CLLocationManager*)manager didUpdateHeading:(CLHeading*)newHeading
{
    if (self.headingHandler != nil)
    {
        self.headingHandler(newHeading.magneticHeading);
    }
}
- (BOOL)locationManagerShouldDisplayHeadingCalibration:(CLLocationManager*)manager
{
    // Mirrors C# LocationManagerShouldDisplayHeadingCalibration => ShouldDisplayHeadingCalibration.
    if (self.calibrationProvider != nil)
    {
        return self.calibrationProvider();
    }
    return NO; // the C# default once the handler is torn down
}
@end

namespace maui::devices::sensors
{
    namespace
    {
        // The angular-distance filters, aligned with the Windows numbers (Compass.ios.cs).
        constexpr double fastest_filter = .01;
        constexpr double game_filter = .5;
        constexpr double normal_filter = 1;
        constexpr double ui_filter = 2;

        class ios_compass final : public detail::compass_base
        {
        public:
            ~ios_compass() override
            {
                ios_compass::platform_stop();
            }
            ios_compass() = default;
            ios_compass(const ios_compass&) = delete;
            ios_compass(ios_compass&&) = delete;
            ios_compass& operator=(const ios_compass&) = delete;
            ios_compass& operator=(ios_compass&&) = delete;

            // IPlatformCompass.ShouldDisplayHeadingCalibration get/set (Compass.ios.cs auto-prop),
            // backed by the base field so it persists across start/stop like the C# partial.
            [[nodiscard]] bool should_display_heading_calibration() const override
            {
                return platform_should_display_heading_calibration();
            }
            void set_should_display_heading_calibration(bool value) override
            {
                set_platform_should_display_heading_calibration(value);
            }

        protected:
            [[nodiscard]] bool platform_is_supported() const override
            {
                return [CLLocationManager headingAvailable];
            }

            void platform_start(sensor_speed speed, bool /*apply_low_pass_filter: Android-only*/) override
            {
                location_manager_ = [[CLLocationManager alloc] init];
                listener_ = [[MauiIosCompassListener alloc] init];
                listener_.headingHandler = ^(double magnetic_heading) {
                  this->raise_reading_changed(compass_data(magnetic_heading));
                };
                // The OS asks whether to show the calibration overlay during heading updates;
                // answer from the cached ShouldDisplayHeadingCalibration (Compass.ios.cs).
                listener_.calibrationProvider = ^BOOL {
                  return this->should_display_heading_calibration() ? YES : NO;
                };
                switch (speed)
                {
                    case sensor_speed::fastest:
                        location_manager_.headingFilter = fastest_filter;
                        location_manager_.desiredAccuracy = kCLLocationAccuracyBestForNavigation;
                        break;
                    case sensor_speed::game:
                        location_manager_.headingFilter = game_filter;
                        location_manager_.desiredAccuracy = kCLLocationAccuracyBestForNavigation;
                        break;
                    case sensor_speed::default_:
                        location_manager_.headingFilter = normal_filter;
                        location_manager_.desiredAccuracy = kCLLocationAccuracyBest;
                        break;
                    case sensor_speed::ui:
                        location_manager_.headingFilter = ui_filter;
                        location_manager_.desiredAccuracy = kCLLocationAccuracyBest;
                        break;
                }
                location_manager_.delegate = listener_;
                [location_manager_ startUpdatingHeading];
            }

            void platform_stop() override
            {
                if (location_manager_ == nil)
                {
                    return;
                }
                listener_.headingHandler = nil;
                listener_.calibrationProvider = nil;
                [location_manager_ stopUpdatingHeading];
                location_manager_.delegate = nil;
                location_manager_ = nil;
                listener_ = nil;
            }

        private:
            CLLocationManager* location_manager_ = nil; // ARC-managed strong references
            MauiIosCompassListener* listener_ = nil;
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_compass> make_compass()
        {
            return std::make_shared<ios_compass>();
        }
    } // namespace detail
} // namespace maui::devices::sensors
