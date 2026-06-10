// geolocation - Apple (AppKit / macOS) platform partial. Ported from Geolocation.ios.macos.cs
// (the partial covers both Apple backends; this is the macOS compilation - the __IOS__-only
// branches live in the ios twin):
//   - is_enabled: CLLocationManager.locationServicesEnabled.
//   - get_last_known_location_async: a fresh manager's cached .location (no permission flow - the
//     port has no Permissions subsystem; the host app must hold location authorization).
//   - get_location_async: a single-shot delegate (SingleLocationListener: first update wins),
//     honoring the request timeout (dispatch_after on the main queue -> stop + nullopt) and the
//     poll-style cancellation token; the FeatureNotEnabledException fold throws before any work.
//   - start/stop_listening_foreground: a continuous delegate (ContinuousLocationListener); a
//     network failure (kCLErrorNetwork) or a Denied/Restricted authorization change stops
//     listening FIRST and then raises listening_failed.
// CLLocation -> maui location per LocationExtensions.ToLocation (macOS branch: no Course/Speed).

#import <CoreLocation/CoreLocation.h>
#import <Foundation/Foundation.h>
#import <objc/runtime.h>

#include <chrono>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>

#include "maui/core/cancellation_token.hpp"
#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/geolocation.hpp"

#include "src/essentials/detail/geolocation_base.hpp"

namespace
{
    // GeolocationAccuracyExtensionMethods.PlatformDesiredAccuracy.
    CLLocationAccuracy to_platform_accuracy(maui::devices::sensors::geolocation_accuracy accuracy)
    {
        using maui::devices::sensors::geolocation_accuracy;
        switch (accuracy)
        {
            case geolocation_accuracy::lowest:
                return kCLLocationAccuracyThreeKilometers;
            case geolocation_accuracy::low:
                return kCLLocationAccuracyKilometer;
            case geolocation_accuracy::default_:
            case geolocation_accuracy::medium:
                return kCLLocationAccuracyHundredMeters;
            case geolocation_accuracy::high:
                return kCLLocationAccuracyNearestTenMeters;
            case geolocation_accuracy::best:
                return kCLLocationAccuracyBestForNavigation;
        }
        return kCLLocationAccuracyHundredMeters;
    }

    // LocationExtensions.ToLocation(CLLocation, reducedAccuracy) - macOS branch (no Course/Speed).
    maui::devices::sensors::location to_location(CLLocation* platform_location)
    {
        maui::devices::sensors::location result;
        result.latitude = platform_location.coordinate.latitude;
        result.longitude = platform_location.coordinate.longitude;
        if (platform_location.verticalAccuracy >= 0)
        {
            result.altitude = platform_location.altitude;
        }
        result.accuracy = platform_location.horizontalAccuracy;
        result.vertical_accuracy = platform_location.verticalAccuracy;
        result.timestamp = std::chrono::system_clock::from_time_t(0) +
                           std::chrono::milliseconds(
                               static_cast<long long>([platform_location.timestamp timeIntervalSince1970] * 1000.0));
        result.altitude_reference_system = maui::devices::sensors::altitude_reference_system::geoid;
        return result;
    }
} // namespace

// SingleLocationListener: the first delivered location wins; further updates are ignored.
@interface MauiAppleSingleLocationListener : NSObject <CLLocationManagerDelegate>
@property(nonatomic, copy) void (^locationHandler)(CLLocation*);
@property(nonatomic) BOOL wasRaised;
@end

@implementation MauiAppleSingleLocationListener
- (void)locationManager:(CLLocationManager*)manager didUpdateLocations:(NSArray<CLLocation*>*)locations
{
    if (self.wasRaised)
    {
        return;
    }
    self.wasRaised = YES;
    CLLocation* location = locations.lastObject;
    if (location != nil && self.locationHandler != nil)
    {
        self.locationHandler(location);
    }
}
@end

// ContinuousLocationListener: forwards every update; network errors and a Denied/Restricted
// authorization change report through the error handler.
@interface MauiAppleContinuousLocationListener : NSObject <CLLocationManagerDelegate>
@property(nonatomic, copy) void (^locationHandler)(CLLocation*);
@property(nonatomic, copy) void (^errorHandler)(maui::devices::sensors::geolocation_error);
@end

@implementation MauiAppleContinuousLocationListener
- (void)locationManager:(CLLocationManager*)manager didUpdateLocations:(NSArray<CLLocation*>*)locations
{
    CLLocation* location = locations.lastObject;
    if (location != nil && self.locationHandler != nil)
    {
        self.locationHandler(location);
    }
}
- (void)locationManager:(CLLocationManager*)manager didFailWithError:(NSError*)error
{
    if (error.code == kCLErrorNetwork && self.errorHandler != nil)
    {
        self.errorHandler(maui::devices::sensors::geolocation_error::position_unavailable);
    }
}
- (void)locationManager:(CLLocationManager*)manager didChangeAuthorizationStatus:(CLAuthorizationStatus)status
{
    if ((status == kCLAuthorizationStatusDenied || status == kCLAuthorizationStatusRestricted) &&
        self.errorHandler != nil)
    {
        self.errorHandler(maui::devices::sensors::geolocation_error::unauthorized);
    }
}
@end

namespace maui::devices::sensors
{
    namespace
    {
        // The associated-object key anchoring a single-shot query's delegate to its manager.
        const void* k_single_shot_listener_key()
        {
            static const char key = 0;
            return &key;
        }

        [[noreturn]] void throw_not_enabled()
        {
            // FeatureNotEnabledException, folded into the lib's single error type.
            throw maui::application_model::feature_not_supported("Location services are not enabled on device.");
        }

        class apple_geolocation final : public detail::geolocation_base
        {
        public:
            ~apple_geolocation() override
            {
                apple_geolocation::stop_listening_foreground();
            }
            apple_geolocation() = default;
            apple_geolocation(const apple_geolocation&) = delete;
            apple_geolocation(apple_geolocation&&) = delete;
            apple_geolocation& operator=(const apple_geolocation&) = delete;
            apple_geolocation& operator=(apple_geolocation&&) = delete;

            [[nodiscard]] bool is_enabled() const override
            {
                return [CLLocationManager locationServicesEnabled] != NO;
            }

            [[nodiscard]] bool is_listening_foreground() const override
            {
                return listening_manager_ != nil;
            }

            void get_last_known_location_async(location_callback on_complete) override
            {
                if (!is_enabled())
                {
                    throw_not_enabled();
                }
                CLLocationManager* const manager = [[CLLocationManager alloc] init];
                CLLocation* const cached = manager.location;
                on_complete(cached != nil ? std::optional<location>(to_location(cached)) : std::nullopt);
            }

            void get_location_async(const geolocation_request& request, maui::core::cancellation_token token,
                                    location_callback on_complete) override
            {
                if (!is_enabled())
                {
                    throw_not_enabled();
                }

                CLLocationManager* const manager = [[CLLocationManager alloc] init];
                MauiAppleSingleLocationListener* const listener = [[MauiAppleSingleLocationListener alloc] init];
                manager.desiredAccuracy = to_platform_accuracy(request.desired_accuracy);
                manager.delegate = listener;

                // CLLocationManager.delegate is an ASSIGN property: retain the single-shot
                // listener on the manager via an associated object (the repo's trampoline idiom) so
                // it survives until resolution; the completion paths clear it, breaking the
                // deliberate manager -> listener -> handler-block -> manager cycle.
                objc_setAssociatedObject(manager, k_single_shot_listener_key(), listener,
                                         OBJC_ASSOCIATION_RETAIN_NONATOMIC);

                // The callback is move-only; blocks copy their captures, so share it.
                const auto shared_callback = std::make_shared<location_callback>(std::move(on_complete));
                const auto completed = std::make_shared<bool>(false);

                listener.locationHandler = ^(CLLocation* platform_location) {
                  if (*completed)
                  {
                      return;
                  }
                  *completed = true;
                  [manager stopUpdatingLocation];
                  manager.delegate = nil;
                  // Defer releasing the listener to the next main-queue tick: this very block is
                  // owned by it and is still executing (and CL may still unwind the delegate call).
                  dispatch_async(dispatch_get_main_queue(), ^{
                    objc_setAssociatedObject(manager, k_single_shot_listener_key(), nil,
                                             OBJC_ASSOCIATION_RETAIN_NONATOMIC);
                  });
                  if (token.is_cancelled())
                  {
                      (*shared_callback)(std::nullopt); // the Cancel path completes with null
                  }
                  else
                  {
                      (*shared_callback)(to_location(platform_location));
                  }
                };

                [manager startUpdatingLocation];

                // Utils.TimeoutToken: a positive request timeout cancels the query.
                if (request.timeout.count() > 0)
                {
                    const auto deadline = dispatch_time(DISPATCH_TIME_NOW, request.timeout.count() * 1'000'000LL);
                    dispatch_after(deadline, dispatch_get_main_queue(), ^{
                      if (*completed)
                      {
                          return;
                      }
                      *completed = true;
                      [manager stopUpdatingLocation];
                      manager.delegate = nil;
                      dispatch_async(dispatch_get_main_queue(), ^{
                        objc_setAssociatedObject(manager, k_single_shot_listener_key(), nil,
                                                 OBJC_ASSOCIATION_RETAIN_NONATOMIC);
                      });
                      (*shared_callback)(std::nullopt);
                    });
                }
            }

            bool start_listening_foreground(const geolocation_listening_request& request) override
            {
                if (is_listening_foreground())
                {
                    throw std::logic_error("Already listening to location changes.");
                }
                if (!is_enabled())
                {
                    throw_not_enabled();
                }

                CLLocationManager* const manager = [[CLLocationManager alloc] init];
                MauiAppleContinuousLocationListener* const listener =
                    [[MauiAppleContinuousLocationListener alloc] init];
                listener.locationHandler = ^(CLLocation* platform_location) {
                  this->on_location_changed(to_location(platform_location));
                };
                listener.errorHandler = ^(geolocation_error error) {
                  this->stop_listening_foreground();
                  this->on_location_error(error);
                };

                manager.desiredAccuracy = to_platform_accuracy(request.desired_accuracy);
                manager.delegate = listener;
                [manager startUpdatingLocation];

                listening_manager_ = manager;
                listening_listener_ = listener;
                return true;
            }

            void stop_listening_foreground() override
            {
                if (listening_manager_ == nil)
                {
                    return;
                }
                [listening_manager_ stopUpdatingLocation];
                listening_listener_.locationHandler = nil;
                listening_listener_.errorHandler = nil;
                listening_manager_.delegate = nil;
                listening_manager_ = nil;
                listening_listener_ = nil;
            }

        private:
            CLLocationManager* listening_manager_ = nil; // ARC-managed strong members
            MauiAppleContinuousLocationListener* listening_listener_ = nil;
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_geolocation> make_geolocation()
        {
            return std::make_shared<apple_geolocation>();
        }
    } // namespace detail
} // namespace maui::devices::sensors
