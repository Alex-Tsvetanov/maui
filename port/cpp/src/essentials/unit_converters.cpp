// unit_converters - the runtime half. Only the great-circle coordinate distance needs <cmath>
// (sin/asin/sqrt), so it lives here; every other converter is a constexpr expression in the
// header. Ported 1:1 from UnitConverters.shared.cs CoordinatesToKilometers / CoordinatesToMiles.

#include "maui/essentials/unit_converters.hpp"

#include <cmath>

namespace maui::media
{
    double unit_converters::coordinates_to_kilometers(double lat1, double lon1, double lat2, double lon2)
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

        const double a = d_lat2 + d_lon2 * std::cos(lat1) * std::cos(lat2);
        const double c = 2 * std::asin(std::sqrt(a));

        return mean_earth_radius_in_kilometers * c;
    }

    double unit_converters::coordinates_to_miles(double lat1, double lon1, double lat2, double lon2)
    {
        return kilometers_to_miles(coordinates_to_kilometers(lat1, lon1, lat2, lon2));
    }
} // namespace maui::media
