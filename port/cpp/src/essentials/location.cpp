// Location.CalculateDistance  <=  Microsoft.Maui.Devices.Sensors.Location, which (like the C#) delegates
// the Haversine math to Microsoft.Maui.Media.UnitConverters.CoordinatesToKilometers/CoordinatesToMiles.

#include "maui/essentials/location.hpp"

#include "maui/essentials/unit_converters.hpp"

#include <stdexcept>

namespace maui::devices::sensors
{
    double location::calculate_distance(double latitude_start, double longitude_start, double latitude_end,
                                        double longitude_end, distance_units units)
    {
        switch (units)
        {
            case distance_units::kilometers:
                return maui::media::unit_converters::coordinates_to_kilometers(latitude_start, longitude_start,
                                                                               latitude_end, longitude_end);
            case distance_units::miles:
                return maui::media::unit_converters::coordinates_to_miles(latitude_start, longitude_start, latitude_end,
                                                                          longitude_end);
        }
        // ArgumentOutOfRangeException(nameof(units)).
        throw std::invalid_argument("location::calculate_distance: unknown distance unit");
    }
} // namespace maui::devices::sensors
