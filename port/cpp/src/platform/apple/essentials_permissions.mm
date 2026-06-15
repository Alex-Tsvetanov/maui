// permissions - Apple (AppKit / macOS) platform partial, ported from Permissions.macos.cs. The
// BasePlatformPermission base auto-grants (check/request resolve Granted inline, rationale is
// false) and EnsureDeclared only enforces RequiredInfoPlistKeys - of the ported kinds NONE are
// required on macOS (LocationWhenInUse's usage key is merely "recommended": a Console nudge in
// C#, a no-op here that never throws). LocationWhenInUse overrides the auto-grant with the real
// CoreLocation STATUS QUERY (Permissions.macos.cs GetLocationStatus): location services off ->
// disabled, else a CLLocationManager's authorizationStatus maps AuthorizedAlways/WhenInUse ->
// granted, Denied -> denied, Restricted -> restricted, NotDetermined -> unknown. RequestAsync on
// macOS reports the QUERIED status with NO interactive prompt (unlike iOS). LocationAlways is an
// empty macOS partial, so it keeps the base auto-grant (NOT routed through CoreLocation).
// Compiled as Objective-C++ with ARC for the apple backend.

#import <CoreLocation/CoreLocation.h>

#include <memory>
#include <utility>

#include "maui/essentials/permissions.hpp"

namespace maui::application_model
{
    namespace
    {
        // Permissions.LocationWhenInUse.GetLocationStatus (macOS takes NO whenInUse argument,
        // unlike the iOS twin: AuthorizedAlways/WhenInUse -> Granted). The C# CLAuthorizationStatus
        // .AuthorizedWhenInUse enumerator is iOS-only at the native layer
        // (kCLAuthorizationStatusAuthorizedWhenInUse is API_UNAVAILABLE(macos)); on macOS the
        // granted status surfaces as kCLAuthorizationStatusAuthorizedAlways (its deprecated synonym
        // kCLAuthorizationStatusAuthorized equals AuthorizedAlways), so the single Always case
        // covers both. The C# CLLocationManager.Status class read is deprecated since macOS 11 (C#
        // pragma-suppresses); the port reads the modern per-manager authorizationStatus instead -
        // same values.
        permission_status location_status()
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
                case kCLAuthorizationStatusDenied:
                    return permission_status::denied;
                case kCLAuthorizationStatusRestricted:
                    return permission_status::restricted;
                default:
                    return permission_status::unknown;
            }
        }

        // Only LocationWhenInUse overrides the macOS auto-grant; LocationAlways stays auto-granted
        // (its macOS partial is empty - see Permissions.macos.cs).
        [[nodiscard]] bool is_location(permission_kind kind)
        {
            return kind == permission_kind::location_when_in_use;
        }

        class apple_permission_backend final : public i_permission_backend
        {
        public:
            void check_status(permission_kind kind, permission_status_callback on_complete) override
            {
                if (is_location(kind))
                {
                    ensure_declared(kind); // the C# location CheckStatusAsync starts with EnsureDeclared
                    on_complete(location_status());
                    return;
                }
                on_complete(permission_status::granted); // the BasePlatformPermission auto-grant
            }

            void request(permission_kind kind, permission_status_callback on_complete) override
            {
                // No interactive prompt on macOS: report the queried status (see the header comment).
                check_status(kind, std::move(on_complete));
            }

            void ensure_declared(permission_kind /*kind*/) override
            {
                // No ported kind has RequiredInfoPlistKeys on macOS (location's usage key is only
                // recommended - a Console nudge in C# that never throws).
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
            return std::make_shared<apple_permission_backend>();
        }
    } // namespace detail
} // namespace maui::application_model
