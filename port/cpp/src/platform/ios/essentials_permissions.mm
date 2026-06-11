// permissions - iOS (UIKit) platform partial. Ported from Permissions.ios.tvos.watchos.cs: the
// BasePlatformPermission base auto-grants (check/request resolve Granted inline, rationale is
// false) and EnsureDeclared enforces RequiredInfoPlistKeys - of the ported kinds the location
// pair requires them on iOS (NSLocationWhenInUseUsageDescription; LocationAlways additionally
// NSLocationAlwaysAndWhenInUseUsageDescription). location_when_in_use carries the real
// STATUS-QUERY architecture (Permissions.ios.cs GetLocationStatus): location services off ->
// disabled, else CLLocationManager.authorizationStatus maps AuthorizedAlways/WhenInUse ->
// granted, Denied -> denied, Restricted -> restricted, NotDetermined -> unknown
// (location_always: WhenInUse-only authorization maps to denied). DOCUMENTED DEVIATION
// (permissions.hpp): request_async for the location pair ensures declaration and reports the
// QUERIED status - the interactive requestWhenInUseAuthorization prompt cannot be driven from
// the spawned test process; a real app's prompt round-trips through the OS and is then visible
// to the query. Compiled as Objective-C++ with ARC for the ios backend.

#import <CoreLocation/CoreLocation.h>
#import <Foundation/Foundation.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/essentials/permissions.hpp"

namespace maui::application_model
{
    namespace
    {
        // Permissions.IsKeyDeclaredInInfoPlist.
        bool is_key_declared_in_info_plist(NSString* usage_key)
        {
            return [NSBundle mainBundle].infoDictionary[usage_key] != nil;
        }

        // RequiredInfoPlistKeys per ported kind (iOS).
        std::vector<NSString*> required_info_plist_keys(permission_kind kind)
        {
            switch (kind)
            {
                case permission_kind::location_when_in_use:
                    return {@"NSLocationWhenInUseUsageDescription"};
                case permission_kind::location_always:
                    return {@"NSLocationAlwaysAndWhenInUseUsageDescription", @"NSLocationWhenInUseUsageDescription"};
                default:
                    return {};
            }
        }

        // Permissions.LocationWhenInUse.GetLocationStatus(whenInUse). The C# CLLocationManager
        // .Status class read is deprecated since iOS 14 (C# pragma-suppresses); the port reads
        // the modern per-manager authorizationStatus instead - same values.
        permission_status location_status(bool when_in_use)
        {
            if (![CLLocationManager locationServicesEnabled])
            {
                return permission_status::disabled;
            }
            CLLocationManager* const manager = [[CLLocationManager alloc] init];
            switch (manager.authorizationStatus)
            {
                case kCLAuthorizationStatusAuthorizedAlways:
                    return permission_status::granted;
                case kCLAuthorizationStatusAuthorizedWhenInUse:
                    return when_in_use ? permission_status::granted : permission_status::denied;
                case kCLAuthorizationStatusDenied:
                    return permission_status::denied;
                case kCLAuthorizationStatusRestricted:
                    return permission_status::restricted;
                default:
                    return permission_status::unknown;
            }
        }

        [[nodiscard]] bool is_location(permission_kind kind)
        {
            return kind == permission_kind::location_when_in_use || kind == permission_kind::location_always;
        }

        class ios_permission_backend final : public i_permission_backend
        {
        public:
            void check_status(permission_kind kind, permission_status_callback on_complete) override
            {
                if (is_location(kind))
                {
                    ensure_declared(kind); // the C# location CheckStatusAsync starts with EnsureDeclared
                    on_complete(location_status(kind == permission_kind::location_when_in_use));
                    return;
                }
                on_complete(permission_status::granted); // the BasePlatformPermission auto-grant
            }

            void request(permission_kind kind, permission_status_callback on_complete) override
            {
                // The location prompt deviation (see the header comment): report the queried status.
                check_status(kind, std::move(on_complete));
            }

            void ensure_declared(permission_kind kind) override
            {
                for (NSString* const key : required_info_plist_keys(kind))
                {
                    if (!is_key_declared_in_info_plist(key))
                    {
                        const char* const utf8 = [key UTF8String];
                        throw permission_error("You must set `" + std::string(utf8 != nullptr ? utf8 : "") +
                                               "` in your Info.plist file to use this permission.");
                    }
                }
            }

            [[nodiscard]] bool should_show_rationale(permission_kind /*kind*/) override
            {
                return false;
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_permission_backend> make_permission_backend()
        {
            return std::make_shared<ios_permission_backend>();
        }
    } // namespace detail
} // namespace maui::application_model
