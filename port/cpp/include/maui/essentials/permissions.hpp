#pragma once
// maui::application_model::permissions        <=  Microsoft.Maui.ApplicationModel.Permissions (static class + nested
// types) maui::application_model::permission_status  <=  Microsoft.Maui.ApplicationModel.PermissionStatus
// maui::application_model::permission_error   <=  Microsoft.Maui.ApplicationModel.PermissionException
//
// Runtime permission checks/requests. C#'s design is `new TPermission()` per call (no DI seam),
// with per-PLATFORM behavior split across BasePlatformPermission partials. The port keeps the
// generic surface - permissions::check_status_async<permissions::camera>(...) mirrors
// Permissions.CheckStatusAsync<Permissions.Camera>() - but routes every permission type through
// ONE injectable backend seam (i_permission_backend keyed by permission_kind): that is the
// per-platform partial AND the fake-grantable test seam the C# design lacks (a port addition,
// required because `new TPermission()` offers no injection point). The Task<PermissionStatus>
// surface becomes the library's callback convention (inline completion on every ported path).
//
// Backends (suffix oracle + the desktop auto-grant decision):
//   * apple/macOS (Permissions.macos.cs): the DESKTOP AUTO-GRANT stub - every kind checks and
//     requests as granted, should_show_rationale is false, ensure_declared validates required
//     Info.plist keys (none of the ported kinds require one on macOS - location's key is only
//     "recommended" there, a console nudge in C#). The macOS LocationWhenInUse CoreLocation
//     override is NOT ported (the auto-grant stub per this unit's scope; documented).
//   * ios (Permissions.ios.tvos.watchos.cs + Permissions.ios.cs): the base is auto-grant +
//     required-Info.plist-key enforcement; location_when_in_use carries the real STATUS-QUERY
//     architecture (CLLocationManager.authorizationStatus -> permission_status). The interactive
//     request prompt (requestWhenInUseAuthorization) is NOT driveable from the spawned test
//     process - request_async ensures declaration and reports the current queried status
//     (documented; a real app's prompt round-trips through the OS and is then visible here).
//   * headless (Permissions.netstandard.cs): every member throws until the fake backend is
//     installed (the netstandard mirror); the fake stages per-kind statuses + request answers.
//
// C#'s ShouldShowRationale is Android-only behavior (false elsewhere); EnsureDeclared throws
// permission_error when a required manifest entry is missing. The apple-only nested
// EventPermissions type is not ported (no cross-platform contract).

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "maui/core/move_only_function.hpp"

namespace maui::application_model
{
    // Possible statuses of a permission (Permissions.shared.enums.cs).
    enum class permission_status
    {
        unknown = 0,
        denied = 1,
        disabled = 2,
        granted = 3,
        restricted = 4,
        limited = 5, // iOS only
    };

    // PermissionException : UnauthorizedAccessException - raised for missing manifest
    // declarations and off-main-thread requests.
    class permission_error : public std::runtime_error
    {
    public:
        explicit permission_error(const std::string& message) : std::runtime_error(message)
        {
        }
    };

    // The C# nested permission TYPES, as a value enum - the key the backend seam dispatches on.
    enum class permission_kind
    {
        battery,
        bluetooth,
        calendar_read,
        calendar_write,
        camera,
        contacts_read,
        contacts_write,
        flashlight,
        launch_app,
        location_when_in_use,
        location_always,
        maps,
        media,
        microphone,
        nearby_wifi_devices,
        network_state,
        phone,
        photos,
        photos_add_only,
        post_notifications,
        reminders,
        sensors,
        sms,
        speech,
        storage_read,
        storage_write,
        vibrate,
    };

    // Receives a check/request result.
    using permission_status_callback = maui::core::move_only_function<void(permission_status)>;

    // The per-backend partial: one implementation per platform under
    // src/platform/<backend>/essentials_permissions.{cpp,mm}.
    class i_permission_backend
    {
    public:
        virtual ~i_permission_backend() = default;

        virtual void check_status(permission_kind kind, permission_status_callback on_complete) = 0;
        virtual void request(permission_kind kind, permission_status_callback on_complete) = 0;
        virtual void ensure_declared(permission_kind kind) = 0;
        [[nodiscard]] virtual bool should_show_rationale(permission_kind kind) = 0;

    protected:
        i_permission_backend() = default;
        i_permission_backend(const i_permission_backend&) = default;
        i_permission_backend(i_permission_backend&&) = default;
        i_permission_backend& operator=(const i_permission_backend&) = default;
        i_permission_backend& operator=(i_permission_backend&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory.
        [[nodiscard]] std::shared_ptr<i_permission_backend> make_permission_backend();
    } // namespace detail

    // The static facade + the nested permission types (C# Permissions).
    class permissions final
    {
    public:
        permissions() = delete;

        // BasePermission - the abstract contract every permission type satisfies.
        class base_permission
        {
        public:
            virtual ~base_permission() = default;

            virtual void check_status_async(permission_status_callback on_complete) = 0;
            virtual void request_async(permission_status_callback on_complete) = 0;
            virtual void ensure_declared() = 0;
            [[nodiscard]] virtual bool should_show_rationale() = 0;

        protected:
            base_permission() = default;
            base_permission(const base_permission&) = default;
            base_permission(base_permission&&) = default;
            base_permission& operator=(const base_permission&) = default;
            base_permission& operator=(base_permission&&) = default;
        };

        // The shared concrete shape: every C# permission type is this template over its kind,
        // routing through the installed backend (the platform-partial dispatch).
        template <permission_kind Kind> class typed_permission final : public base_permission
        {
        public:
            void check_status_async(permission_status_callback on_complete) override
            {
                backend().check_status(Kind, std::move(on_complete));
            }
            void request_async(permission_status_callback on_complete) override
            {
                backend().request(Kind, std::move(on_complete));
            }
            void ensure_declared() override
            {
                backend().ensure_declared(Kind);
            }
            [[nodiscard]] bool should_show_rationale() override
            {
                return backend().should_show_rationale(Kind);
            }
        };

        // The C# nested permission types (Permissions.Battery -> permissions::battery, ...).
        using battery = typed_permission<permission_kind::battery>;
        using bluetooth = typed_permission<permission_kind::bluetooth>;
        using calendar_read = typed_permission<permission_kind::calendar_read>;
        using calendar_write = typed_permission<permission_kind::calendar_write>;
        using camera = typed_permission<permission_kind::camera>;
        using contacts_read = typed_permission<permission_kind::contacts_read>;
        using contacts_write = typed_permission<permission_kind::contacts_write>;
        using flashlight = typed_permission<permission_kind::flashlight>;
        using launch_app = typed_permission<permission_kind::launch_app>;
        using location_when_in_use = typed_permission<permission_kind::location_when_in_use>;
        using location_always = typed_permission<permission_kind::location_always>;
        using maps = typed_permission<permission_kind::maps>;
        using media = typed_permission<permission_kind::media>;
        using microphone = typed_permission<permission_kind::microphone>;
        using nearby_wifi_devices = typed_permission<permission_kind::nearby_wifi_devices>;
        using network_state = typed_permission<permission_kind::network_state>;
        using phone = typed_permission<permission_kind::phone>;
        using photos = typed_permission<permission_kind::photos>;
        using photos_add_only = typed_permission<permission_kind::photos_add_only>;
        using post_notifications = typed_permission<permission_kind::post_notifications>;
        using reminders = typed_permission<permission_kind::reminders>;
        using sensors = typed_permission<permission_kind::sensors>;
        using sms = typed_permission<permission_kind::sms>;
        using speech = typed_permission<permission_kind::speech>;
        using storage_read = typed_permission<permission_kind::storage_read>;
        using storage_write = typed_permission<permission_kind::storage_write>;
        using vibrate = typed_permission<permission_kind::vibrate>;

        // Permissions.CheckStatusAsync<TPermission>() and friends - `new TPermission()` per call,
        // exactly like C#.
        template <class TPermission> static void check_status_async(permission_status_callback on_complete)
        {
            TPermission{}.check_status_async(std::move(on_complete));
        }
        template <class TPermission> static void request_async(permission_status_callback on_complete)
        {
            TPermission{}.request_async(std::move(on_complete));
        }
        template <class TPermission> [[nodiscard]] static bool should_show_rationale()
        {
            return TPermission{}.should_show_rationale();
        }
        // Internal in C#; the port's test surface.
        template <class TPermission> static void ensure_declared()
        {
            TPermission{}.ensure_declared();
        }

        // The lazy platform backend + the injection seam (a PORT ADDITION - the fake-grantable
        // seam; nullptr resets to the lazy platform default).
        [[nodiscard]] static i_permission_backend& backend();
        static void set_backend(std::shared_ptr<i_permission_backend> implementation);
    };
} // namespace maui::application_model
