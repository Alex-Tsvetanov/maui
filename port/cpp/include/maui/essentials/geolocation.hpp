#pragma once
// maui::devices::sensors::geolocation                 <=  Microsoft.Maui.Devices.Sensors.Geolocation (static facade)
// maui::devices::sensors::i_geolocation               <=  Microsoft.Maui.Devices.Sensors.IGeolocation
// maui::devices::sensors::geolocation_accuracy        <=  Microsoft.Maui.Devices.Sensors.GeolocationAccuracy
// maui::devices::sensors::geolocation_request         <=  Microsoft.Maui.Devices.Sensors.GeolocationRequest
// maui::devices::sensors::geolocation_listening_request <= Microsoft.Maui.Devices.Sensors.GeolocationListeningRequest
// maui::devices::sensors::geolocation_error            <=  Microsoft.Maui.Devices.Sensors.GeolocationError
//
// The device-location API. Adaptations from the C# surface, applied uniformly:
//   - Task<Location?> becomes callback-based async (location_callback receives optional<location>;
//     the port has no task type) - completion is delivered on the platform's main queue on
//     apple/ios, and inline by the headless fake.
//   - The Permissions subsystem is outside this unit's scope: the real partials skip
//     Permissions.EnsureGrantedAsync and rely on the host app holding location authorization.
//   - FeatureNotEnabledException ("Location services are not enabled on device.") folds into
//     feature_not_supported with that message (the lib's single-error-type rule).
//   - Task<bool> StartListeningForegroundAsync completes synchronously in the CoreLocation partial
//     once permissions are skipped, so it ports as bool start_listening_foreground(...). It throws
//     std::logic_error("Already listening to location changes.") on a double start.
//   - The events keep raw event<> accessors (no listener lifecycle, unlike battery/display):
//     LocationChanged collapses to its location payload; ListeningFailed to geolocation_error.
//     Per the C# contract, a listening failure stops listening before the event is raised.
//
// Backends (suffix oracle): apple/macOS + ios REAL (Geolocation.ios.macos.cs - CLLocationManager;
// single-shot GetLocationAsync honors the request timeout and the poll-style cancellation_token).
// Headless mirrors netstandard (is_enabled/is_listening_foreground false, everything else throws)
// until faked - the fake is the test path.

#include <chrono>
#include <memory>
#include <optional>

#include "maui/core/cancellation_token.hpp"
#include "maui/core/event.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/essentials/location.hpp"

namespace maui::devices::sensors
{
    enum class geolocation_accuracy
    {
        default_ = 0, // Medium ("default" is a C++ keyword)
        lowest = 1,
        low = 2,
        medium = 3,
        high = 4,
        best = 5,
    };

    // The criteria for a single location request.
    struct geolocation_request
    {
        // GeolocationRequest.Timeout (zero = no timeout).
        std::chrono::milliseconds timeout{0};
        geolocation_accuracy desired_accuracy = geolocation_accuracy::default_;
        // iOS 14+ only: request temporary full accuracy.
        bool request_full_accuracy = false;
    };

    // The criteria for foreground listening.
    struct geolocation_listening_request
    {
        geolocation_accuracy desired_accuracy = geolocation_accuracy::default_;
        // Minimum time between updates (must be positive; most sensors cap at ~1 s).
        std::chrono::milliseconds minimum_time{1000};
    };

    enum class geolocation_error
    {
        position_unavailable, // no position data could be retrieved
        unauthorized,         // the app lost (or never had) location authorization
    };

    // Receives the (possibly absent) result of an async location query.
    using location_callback = maui::core::move_only_function<void(const std::optional<location>&)>;

    class i_geolocation
    {
    public:
        virtual ~i_geolocation() = default;

        // GetLastKnownLocationAsync: a recent cached location, or nullopt when none is known.
        virtual void get_last_known_location_async(location_callback on_complete) = 0;
        // GetLocationAsync(request, cancelToken): the current location, or nullopt when it could
        // not be determined (including timeout/cancellation).
        virtual void get_location_async(const geolocation_request& request, maui::core::cancellation_token token,
                                        location_callback on_complete) = 0;

        [[nodiscard]] virtual bool is_listening_foreground() const = 0;
        // True when the device's location services are enabled.
        [[nodiscard]] virtual bool is_enabled() const = 0;

        // LocationChanged (payload = the new location) / ListeningFailed (payload = the error;
        // listening has already stopped when it is raised).
        virtual maui::core::event<location>& location_changed() = 0;
        virtual maui::core::event<geolocation_error>& listening_failed() = 0;

        // StartListeningForegroundAsync / StopListeningForeground.
        virtual bool start_listening_foreground(const geolocation_listening_request& request) = 0;
        virtual void stop_listening_foreground() = 0;

    protected:
        i_geolocation() = default;
        i_geolocation(const i_geolocation&) = default;
        i_geolocation(i_geolocation&&) = default;
        i_geolocation& operator=(const i_geolocation&) = default;
        i_geolocation& operator=(i_geolocation&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (GeolocationImplementation), one per backend under
        // src/platform/<backend>/essentials_geolocation.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_geolocation> make_geolocation();
    } // namespace detail

    // The static facade over geolocation::default_() (C# Geolocation.Default). The request-less
    // overload mirrors Geolocation.GetLocationAsync() / GeolocationExtensions (default request).
    class geolocation final
    {
    public:
        geolocation() = delete;

        static void get_last_known_location_async(location_callback on_complete)
        {
            default_().get_last_known_location_async(std::move(on_complete));
        }
        static void get_location_async(location_callback on_complete)
        {
            default_().get_location_async(geolocation_request{}, {}, std::move(on_complete));
        }
        static void get_location_async(const geolocation_request& request, location_callback on_complete)
        {
            default_().get_location_async(request, {}, std::move(on_complete));
        }
        static void get_location_async(const geolocation_request& request, maui::core::cancellation_token token,
                                       location_callback on_complete)
        {
            default_().get_location_async(request, token, std::move(on_complete));
        }
        [[nodiscard]] static bool is_listening_foreground()
        {
            return default_().is_listening_foreground();
        }
        [[nodiscard]] static bool is_enabled()
        {
            return default_().is_enabled();
        }
        static maui::core::event<location>& location_changed()
        {
            return default_().location_changed();
        }
        static maui::core::event<geolocation_error>& listening_failed()
        {
            return default_().listening_failed();
        }
        static bool start_listening_foreground(const geolocation_listening_request& request)
        {
            return default_().start_listening_foreground(request);
        }
        static void stop_listening_foreground()
        {
            default_().stop_listening_foreground();
        }

        // Geolocation.Default (lazy platform default) + SetDefault (the test seam; nullptr resets).
        [[nodiscard]] static i_geolocation& default_();
        static void set_default(std::shared_ptr<i_geolocation> implementation);
    };
} // namespace maui::devices::sensors
