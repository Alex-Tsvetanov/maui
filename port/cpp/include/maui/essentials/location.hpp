#pragma once
// maui::devices::sensors::location                   <=  Microsoft.Maui.Devices.Sensors.Location (Types/)
// maui::devices::sensors::distance_units             <=  Microsoft.Maui.Devices.Sensors.DistanceUnits
// maui::devices::sensors::altitude_reference_system  <=  Microsoft.Maui.Devices.Sensors.AltitudeReferenceSystem
//
// A physical location reported by the device. C#'s mutable property-bag class becomes a value
// struct with public fields (the port's value-type convention); nullable members map to
// std::optional, DateTimeOffset to std::chrono::system_clock::time_point. Equality mirrors C#
// EXACTLY: latitude + longitude only. calculate_distance ports Location.CalculateDistance, whose
// Haversine math (UnitConverters.CoordinatesToKilometers/Miles, mean earth radius 6371 km) is
// inlined in location.cpp - the UnitConverters facade itself is outside this unit's scope.

#include <chrono>
#include <optional>
#include <stdexcept>

namespace maui::devices::sensors
{
    enum class distance_units
    {
        kilometers = 0,
        miles = 1,
    };

    enum class altitude_reference_system
    {
        unspecified = 0,
        terrain = 1,
        ellipsoid = 2,
        geoid = 3,
        surface = 4,
    };

    struct location
    {
        double latitude = 0;
        double longitude = 0;
        // C#'s parameterless Location leaves Timestamp default; Location(lat, lon) stamps UtcNow.
        // C++ keeps the field defaulted (epoch) - construct with now() where the C# ctor would.
        std::chrono::system_clock::time_point timestamp{};
        std::optional<double> altitude;          // meters
        std::optional<double> accuracy;          // horizontal, meters
        std::optional<double> vertical_accuracy; // meters
        bool reduced_accuracy = false;           // iOS 14+ reduced-accuracy authorization
        std::optional<double> speed;             // m/s
        std::optional<double> course;            // degrees relative to true north
        bool is_from_mock_provider = false;
        enum altitude_reference_system altitude_reference_system = altitude_reference_system::unspecified;

        constexpr location() = default;
        // Location(double latitude, double longitude, [timestamp]) - the two-arg C# ctor stamps
        // UtcNow; pass a timestamp explicitly for determinism where needed.
        location(double latitude_, double longitude_,
                 std::chrono::system_clock::time_point timestamp_ = std::chrono::system_clock::now())
            : latitude(latitude_), longitude(longitude_), timestamp(timestamp_)
        {
        }

        // Location.Equals: latitude + longitude only (every other field is ignored).
        friend bool operator==(const location& a, const location& b)
        {
            return a.latitude == b.latitude && a.longitude == b.longitude;
        }

        // Location.CalculateDistance (Haversine; throws std::invalid_argument for an unknown unit,
        // mirroring ArgumentOutOfRangeException).
        [[nodiscard]] static double calculate_distance(double latitude_start, double longitude_start,
                                                       double latitude_end, double longitude_end, distance_units units);
        [[nodiscard]] static double calculate_distance(const location& start, const location& end, distance_units units)
        {
            return calculate_distance(start.latitude, start.longitude, end.latitude, end.longitude, units);
        }
        [[nodiscard]] static double calculate_distance(double latitude_start, double longitude_start,
                                                       const location& end, distance_units units)
        {
            return calculate_distance(latitude_start, longitude_start, end.latitude, end.longitude, units);
        }
        [[nodiscard]] static double calculate_distance(const location& start, double latitude_end, double longitude_end,
                                                       distance_units units)
        {
            return calculate_distance(start.latitude, start.longitude, latitude_end, longitude_end, units);
        }
    };
} // namespace maui::devices::sensors
