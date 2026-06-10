#pragma once
// maui::devices::sensors::placemark  <=  Microsoft.Maui.Devices.Sensors.Placemark (Types/)
//
// A user-friendly description of a geographic position. C#'s mutable property-bag class becomes a
// value struct with public fields (the copy constructor C# spells by hand is the defaulted one).

#include <string>

#include "maui/essentials/location.hpp"

namespace maui::devices::sensors
{
    struct placemark
    {
        sensors::location location; // Placemark.Location
        std::string country_code;   // ISO country code
        std::string country_name;
        std::string feature_name;
        std::string postal_code;
        std::string sub_locality;
        std::string thoroughfare;     // street name
        std::string sub_thoroughfare; // street-level detail (house number)
        std::string locality;         // city
        std::string admin_area;       // state/province
        std::string sub_admin_area;   // county/district
    };
} // namespace maui::devices::sensors
