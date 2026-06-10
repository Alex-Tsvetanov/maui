#pragma once
// maui::devices::sensors::geocoding    <=  Microsoft.Maui.Devices.Sensors.Geocoding (static facade)
// maui::devices::sensors::i_geocoding  <=  Microsoft.Maui.Devices.Sensors.IGeocoding
//
// Geocodes an address to coordinates and reverse-geocodes coordinates to placemarks. The C#
// Task<IEnumerable<T>> surface becomes callback-based async (the port has no task type): the
// callback receives the result vector (empty when nothing matched, mirroring the C# partials'
// `?? Array.Empty<T>()`). Completion is delivered on the main queue by the CoreLocation partials
// and inline by the headless fake. IPlatformGeocoding.MapServiceToken is Windows/Tizen-only and
// not ported.
//
// Backends (suffix oracle): apple/macOS + ios REAL (Geocoding.ios.tvos.watchos.macos.cs -
// CLGeocoder; geocoding needs NETWORK access, so the headless fake is the test path). Headless
// mirrors netstandard (throws) until faked.

#include <memory>
#include <string_view>
#include <vector>

#include "maui/core/move_only_function.hpp"
#include "maui/essentials/location.hpp"
#include "maui/essentials/placemark.hpp"

namespace maui::devices::sensors
{
    using placemarks_callback = maui::core::move_only_function<void(const std::vector<placemark>&)>;
    using locations_callback = maui::core::move_only_function<void(const std::vector<location>&)>;

    class i_geocoding
    {
    public:
        virtual ~i_geocoding() = default;

        // GetPlacemarksAsync(latitude, longitude): placemarks near the coordinates.
        virtual void get_placemarks_async(double latitude, double longitude, placemarks_callback on_complete) = 0;
        // GetLocationsAsync(address): coordinates matching the address.
        virtual void get_locations_async(std::string_view address, locations_callback on_complete) = 0;

    protected:
        i_geocoding() = default;
        i_geocoding(const i_geocoding&) = default;
        i_geocoding(i_geocoding&&) = default;
        i_geocoding& operator=(const i_geocoding&) = default;
        i_geocoding& operator=(i_geocoding&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (GeocodingImplementation), one per backend under
        // src/platform/<backend>/essentials_geocoding.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_geocoding> make_geocoding();
    } // namespace detail

    // The static facade over geocoding::default_() (C# Geocoding.Default). The location overload
    // mirrors Geocoding.GetPlacemarksAsync(Location).
    class geocoding final
    {
    public:
        geocoding() = delete;

        static void get_placemarks_async(double latitude, double longitude, placemarks_callback on_complete)
        {
            default_().get_placemarks_async(latitude, longitude, std::move(on_complete));
        }
        static void get_placemarks_async(const location& location_value, placemarks_callback on_complete)
        {
            default_().get_placemarks_async(location_value.latitude, location_value.longitude, std::move(on_complete));
        }
        static void get_locations_async(std::string_view address, locations_callback on_complete)
        {
            default_().get_locations_async(address, std::move(on_complete));
        }

        // Geocoding.Default (lazy platform default) + SetCurrent (the test seam; nullptr resets;
        // C# names this one SetCurrent even though the accessor is Default).
        [[nodiscard]] static i_geocoding& default_();
        static void set_current(std::shared_ptr<i_geocoding> implementation);
    };
} // namespace maui::devices::sensors
