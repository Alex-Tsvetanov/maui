#pragma once
// maui::media::unit_converters  <=  Microsoft.Maui.Media.UnitConverters
//
// A pure-managed static collection of unit conversions (temperature, distance, angle, pressure,
// weight, angular velocity, plus the great-circle coordinate distance). UnitConverters.shared.cs
// has NO platform partial - it is identical on every backend, so this is fully headless and the
// same on apple/ios. The C# `public static class UnitConverters` with `static double Xxx(double)`
// methods becomes a non-instantiable class of `static` free-functions (snake_case), constexpr
// where the body is a plain arithmetic expression (the coordinate distance uses std::sin/asin/sqrt,
// so it stays a runtime function defined in unit_converters.cpp). Values verified exhaustively
// against src/Essentials/test/UnitTests/UnitConverters_Tests.cs.

#include <numbers>

namespace maui::media
{
    class unit_converters final
    {
    public:
        unit_converters() = delete;

        // --- Temperature ---
        // FahrenheitToCelsius.
        [[nodiscard]] static constexpr double fahrenheit_to_celsius(double fahrenheit)
        {
            return (fahrenheit - 32.0) / 1.8;
        }
        // CelsiusToFahrenheit.
        [[nodiscard]] static constexpr double celsius_to_fahrenheit(double celsius)
        {
            return celsius * 1.8 + 32.0;
        }
        // CelsiusToKelvin.
        [[nodiscard]] static constexpr double celsius_to_kelvin(double celsius)
        {
            return celsius + celsius_to_kelvin_offset;
        }
        // KelvinToCelsius.
        [[nodiscard]] static constexpr double kelvin_to_celsius(double kelvin)
        {
            return kelvin - celsius_to_kelvin_offset;
        }

        // --- Distance ---
        // MilesToMeters.
        [[nodiscard]] static constexpr double miles_to_meters(double miles)
        {
            return miles * miles_to_meters_factor;
        }
        // MilesToKilometers.
        [[nodiscard]] static constexpr double miles_to_kilometers(double miles)
        {
            return miles * miles_to_kilometers_factor;
        }
        // KilometersToMiles.
        [[nodiscard]] static constexpr double kilometers_to_miles(double kilometers)
        {
            return kilometers * kilometers_to_miles_factor;
        }
        // MetersToInternationalFeet (international foot = exactly 0.3048 m, 1959 convention).
        [[nodiscard]] static constexpr double meters_to_international_feet(double meters)
        {
            return meters / international_foot_definition;
        }
        // InternationalFeetToMeters.
        [[nodiscard]] static constexpr double international_feet_to_meters(double international_feet)
        {
            return international_feet * international_foot_definition;
        }
        // MetersToUSSurveyFeet (US survey foot = exactly 1200/3937 m).
        [[nodiscard]] static constexpr double meters_to_us_survey_feet(double meters)
        {
            return meters / us_survey_foot_definition;
        }
        // USSurveyFeetToMeters.
        [[nodiscard]] static constexpr double us_survey_feet_to_meters(double us_feet)
        {
            return us_feet * us_survey_foot_definition;
        }

        // --- Angle ---
        // DegreesToRadians.
        [[nodiscard]] static constexpr double degrees_to_radians(double degrees)
        {
            return degrees * degrees_to_radians_factor;
        }
        // RadiansToDegrees.
        [[nodiscard]] static constexpr double radians_to_degrees(double radians)
        {
            return radians / degrees_to_radians_factor;
        }

        // --- Weight ---
        // PoundsToKilograms.
        [[nodiscard]] static constexpr double pounds_to_kilograms(double pounds)
        {
            return pounds * pounds_to_kg;
        }
        // PoundsToStones.
        [[nodiscard]] static constexpr double pounds_to_stones(double pounds)
        {
            return pounds * pounds_to_stones_factor;
        }
        // StonesToPounds.
        [[nodiscard]] static constexpr double stones_to_pounds(double stones)
        {
            return stones * stones_to_pounds_factor;
        }
        // KilogramsToPounds.
        [[nodiscard]] static constexpr double kilograms_to_pounds(double kilograms)
        {
            return kilograms * kg_to_pounds;
        }

        // --- Angular velocity ---
        // DegreesPerSecondToRadiansPerSecond.
        [[nodiscard]] static constexpr double degrees_per_second_to_radians_per_second(double degrees)
        {
            return hertz_to_radians_per_second(degrees_per_second_to_hertz(degrees));
        }
        // RadiansPerSecondToDegreesPerSecond.
        [[nodiscard]] static constexpr double radians_per_second_to_degrees_per_second(double radians)
        {
            return hertz_to_degrees_per_second(radians_per_second_to_hertz(radians));
        }
        // DegreesPerSecondToHertz.
        [[nodiscard]] static constexpr double degrees_per_second_to_hertz(double degrees)
        {
            return degrees / total_degrees;
        }
        // RadiansPerSecondToHertz.
        [[nodiscard]] static constexpr double radians_per_second_to_hertz(double radians)
        {
            return radians / two_pi;
        }
        // HertzToDegreesPerSecond.
        [[nodiscard]] static constexpr double hertz_to_degrees_per_second(double hertz)
        {
            return hertz * total_degrees;
        }
        // HertzToRadiansPerSecond.
        [[nodiscard]] static constexpr double hertz_to_radians_per_second(double hertz)
        {
            return hertz * two_pi;
        }

        // --- Pressure ---
        // KilopascalsToHectopascals.
        [[nodiscard]] static constexpr double kilopascals_to_hectopascals(double kpa)
        {
            return kpa * 10.0;
        }
        // HectopascalsToKilopascals.
        [[nodiscard]] static constexpr double hectopascals_to_kilopascals(double hpa)
        {
            return hpa / 10.0;
        }
        // KilopascalsToPascals.
        [[nodiscard]] static constexpr double kilopascals_to_pascals(double kpa)
        {
            return kpa * 1000.0;
        }
        // HectopascalsToPascals.
        [[nodiscard]] static constexpr double hectopascals_to_pascals(double hpa)
        {
            return hpa * 100.0;
        }
        // AtmospheresToPascals.
        [[nodiscard]] static constexpr double atmospheres_to_pascals(double atm)
        {
            return atm * atmosphere_pascals;
        }
        // PascalsToAtmospheres.
        [[nodiscard]] static constexpr double pascals_to_atmospheres(double pascals)
        {
            return pascals / atmosphere_pascals;
        }

        // --- Coordinate (great-circle, haversine) ---
        // CoordinatesToMiles.
        [[nodiscard]] static double coordinates_to_miles(double lat1, double lon1, double lat2, double lon2);
        // CoordinatesToKilometers.
        [[nodiscard]] static double coordinates_to_kilometers(double lat1, double lon1, double lat2, double lon2);

    private:
        // The C# private const set (UnitConverters.shared.cs), kept byte-identical.
        static constexpr double two_pi = 2.0 * std::numbers::pi;
        static constexpr double total_degrees = 360.0;
        static constexpr double atmosphere_pascals = 101325.0;
        static constexpr double degrees_to_radians_factor = std::numbers::pi / 180.0;
        static constexpr double miles_to_kilometers_factor = 1.609344;
        static constexpr double miles_to_meters_factor = 1609.344;
        static constexpr double kilometers_to_miles_factor = 1.0 / miles_to_kilometers_factor;
        static constexpr double celsius_to_kelvin_offset = 273.15;
        static constexpr double pounds_to_kg = 0.45359237;
        static constexpr double pounds_to_stones_factor = 0.07142857;
        static constexpr double stones_to_pounds_factor = 14;
        static constexpr double kg_to_pounds = 2.204623;
        static constexpr double mean_earth_radius_in_kilometers = 6371.0;
        static constexpr double international_foot_definition = 0.3048;
        static constexpr double us_survey_foot_definition = 1200.0 / 3937;
    };
} // namespace maui::media
