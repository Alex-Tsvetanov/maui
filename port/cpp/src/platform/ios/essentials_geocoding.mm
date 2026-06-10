// geocoding - iOS (UIKit) platform partial. Ported 1:1 from
// Geocoding.ios.tvos.watchos.macos.cs: reverse geocoding via CLGeocoder
// reverseGeocodeLocation:completionHandler: and forward geocoding via
// geocodeAddressString:completionHandler: (both need NETWORK access - the headless fake is the
// test path). CLPlacemark -> placemark per PlacemarkExtensions.ToPlacemarks; CLPlacemark ->
// location per LocationExtensions.ToLocation(CLPlacemark) (Geoid altitude, UtcNow timestamp). A
// nil result array completes with an empty vector (the C# `?? Array.Empty<T>()`); completion
// handlers arrive on the main queue.

#import <CoreLocation/CoreLocation.h>
#import <Foundation/Foundation.h>

#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/essentials/geocoding.hpp"

namespace maui::devices::sensors
{
    namespace
    {
        std::string to_std_string(NSString* value)
        {
            const char* const utf8 = [value UTF8String]; // messaging nil yields nullptr
            return utf8 != nullptr ? std::string(utf8) : std::string();
        }

        // LocationExtensions.ToLocation(CLPlacemark).
        location placemark_location(CLPlacemark* placemark_value)
        {
            location result;
            if (placemark_value.location != nil)
            {
                result.latitude = placemark_value.location.coordinate.latitude;
                result.longitude = placemark_value.location.coordinate.longitude;
                result.altitude = placemark_value.location.altitude;
            }
            result.altitude_reference_system = altitude_reference_system::geoid;
            result.timestamp = std::chrono::system_clock::now(); // DateTimeOffset.UtcNow
            result.reduced_accuracy = false;
            return result;
        }

        // PlacemarkExtensions.ToPlacemarks.
        placemark to_placemark(CLPlacemark* value)
        {
            placemark result;
            result.location = placemark_location(value);
            result.feature_name = to_std_string(value.name);
            result.postal_code = to_std_string(value.postalCode);
            result.sub_locality = to_std_string(value.subLocality);
            result.country_code = to_std_string(value.ISOcountryCode);
            result.country_name = to_std_string(value.country);
            result.thoroughfare = to_std_string(value.thoroughfare);
            result.sub_thoroughfare = to_std_string(value.subThoroughfare);
            result.locality = to_std_string(value.locality);
            result.admin_area = to_std_string(value.administrativeArea);
            result.sub_admin_area = to_std_string(value.subAdministrativeArea);
            return result;
        }

        class ios_geocoding final : public i_geocoding
        {
        public:
            void get_placemarks_async(double latitude, double longitude, placemarks_callback on_complete) override
            {
                CLGeocoder* const geocoder = [[CLGeocoder alloc] init];
                CLLocation* const target = [[CLLocation alloc] initWithLatitude:latitude longitude:longitude];
                const auto shared_callback = std::make_shared<placemarks_callback>(std::move(on_complete));
                [geocoder reverseGeocodeLocation:target
                               completionHandler:^(NSArray<CLPlacemark*>* placemarks, NSError*) {
                                 std::vector<placemark> result;
                                 result.reserve(placemarks.count);
                                 for (NSUInteger i = 0; i < placemarks.count; ++i)
                                 {
                                     result.push_back(to_placemark(placemarks[i]));
                                 }
                                 (void)geocoder; // keep alive until completion
                                 (*shared_callback)(result);
                               }];
            }

            void get_locations_async(std::string_view address, locations_callback on_complete) override
            {
                CLGeocoder* const geocoder = [[CLGeocoder alloc] init];
                NSString* const target = [[NSString alloc] initWithBytes:address.data()
                                                                  length:address.size()
                                                                encoding:NSUTF8StringEncoding];
                const auto shared_callback = std::make_shared<locations_callback>(std::move(on_complete));
                [geocoder geocodeAddressString:target
                             completionHandler:^(NSArray<CLPlacemark*>* placemarks, NSError*) {
                               std::vector<location> result;
                               result.reserve(placemarks.count);
                               for (NSUInteger i = 0; i < placemarks.count; ++i)
                               {
                                   result.push_back(placemark_location(placemarks[i]));
                               }
                               (void)geocoder; // keep alive until completion
                               (*shared_callback)(result);
                             }];
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_geocoding> make_geocoding()
        {
            return std::make_shared<ios_geocoding>();
        }
    } // namespace detail
} // namespace maui::devices::sensors
