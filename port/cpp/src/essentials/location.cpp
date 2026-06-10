// Location.CalculateDistance  <=  Microsoft.Maui.Devices.Sensors.Location +
// Microsoft.Maui.Media.UnitConverters.CoordinatesToKilometers/CoordinatesToMiles
// (UnitConverters.shared.cs). The Haversine math and its constants are inlined here - the
// UnitConverters facade itself is outside this unit's scope.

#include "maui/essentials/location.hpp"

#include <cmath>
#include <numbers>
#include <stdexcept>

namespace maui::devices::sensors
{
    namespace
    {
        constexpr double degrees_to_radians_factor = std::numbers::pi / 180.0;
        constexpr double miles_to_kilometers = 1.609344;
        constexpr double kilometers_to_miles = 1.0 / miles_to_kilometers;
        constexpr double mean_earth_radius_in_kilometers = 6371.0;

        double degrees_to_radians(double degrees)
        {
            return degrees * degrees_to_radians_factor;
        }

        // UnitConverters.CoordinatesToKilometers: the Haversine great-circle distance.
        double coordinates_to_kilometers(double lat1, double lon1, double lat2, double lon2)
        {
            if (lat1 == lat2 && lon1 == lon2)
            {
                return 0;
            }

            const double d_lat = degrees_to_radians(lat2 - lat1);
            const double d_lon = degrees_to_radians(lon2 - lon1);

            lat1 = degrees_to_radians(lat1);
            lat2 = degrees_to_radians(lat2);

            const double d_lat2 = std::sin(d_lat / 2) * std::sin(d_lat / 2);
            const double d_lon2 = std::sin(d_lon / 2) * std::sin(d_lon / 2);

            const double a = d_lat2 + (d_lon2 * std::cos(lat1) * std::cos(lat2));
            const double c = 2 * std::asin(std::sqrt(a));

            return mean_earth_radius_in_kilometers * c;
        }
    } // namespace

    double location::calculate_distance(double latitude_start, double longitude_start, double latitude_end,
                                        double longitude_end, distance_units units)
    {
        switch (units)
        {
            case distance_units::kilometers:
                return coordinates_to_kilometers(latitude_start, longitude_start, latitude_end, longitude_end);
            case distance_units::miles:
                // UnitConverters.CoordinatesToMiles = KilometersToMiles(CoordinatesToKilometers(...)).
                return kilometers_to_miles *
                       coordinates_to_kilometers(latitude_start, longitude_start, latitude_end, longitude_end);
        }
        // ArgumentOutOfRangeException(nameof(units)).
        throw std::invalid_argument("location::calculate_distance: unknown distance unit");
    }
} // namespace maui::devices::sensors
