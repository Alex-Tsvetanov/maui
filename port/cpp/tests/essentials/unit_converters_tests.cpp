// unit_converters - PURE managed, runs identically on every backend (no platform partial). Ported
// from src/Essentials/test/UnitTests/UnitConverters_Tests.cs: every [Theory]/[InlineData] row is
// reproduced as a parameterized GTest case. C#'s `Assert.Equal(expected, actual, N)` rounds BOTH
// operands to N decimal places before comparing, so the port mirrors that with a round-to-N helper
// rather than a relative tolerance.

#include <array>
#include <cmath>

#include <gtest/gtest.h>

#include "maui/essentials/unit_converters.hpp"

namespace
{
    using maui::media::unit_converters;

    // xUnit's Assert.Equal(expected, actual, precision): round each to `precision` decimals, compare.
    void expect_equal_rounded(double expected, double actual, int precision)
    {
        const double scale = std::pow(10.0, precision);
        EXPECT_DOUBLE_EQ(std::round(expected * scale) / scale, std::round(actual * scale) / scale);
    }

    // --- DegreesPerSecondToRadiansPerSecond / inverse ---
    struct one_in_one_out
    {
        double in;
        double out;
    };

    class dps_to_rps : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(dps_to_rps, value)
    {
        expect_equal_rounded(GetParam().out, unit_converters::degrees_per_second_to_radians_per_second(GetParam().in),
                             4);
    }
    INSTANTIATE_TEST_SUITE_P(table, dps_to_rps,
                             ::testing::Values(one_in_one_out{-1, -0.0175}, one_in_one_out{0.1, 0.0017},
                                               one_in_one_out{0.5, 0.0087}, one_in_one_out{1, 0.0175},
                                               one_in_one_out{2, 0.0349}, one_in_one_out{3, 0.0524},
                                               one_in_one_out{10, 0.1745}, one_in_one_out{57.2958, 1},
                                               one_in_one_out{114.5916, 2}, one_in_one_out{171.8873, 3},
                                               one_in_one_out{572.9578, 10}, one_in_one_out{1000, 17.4533},
                                               one_in_one_out{57295.7795, 1000}, one_in_one_out{0, 0}));

    class rps_to_dps : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(rps_to_dps, value)
    {
        expect_equal_rounded(GetParam().out, unit_converters::radians_per_second_to_degrees_per_second(GetParam().in),
                             4);
    }
    INSTANTIATE_TEST_SUITE_P(table, rps_to_dps,
                             ::testing::Values(one_in_one_out{-1, -57.2958}, one_in_one_out{0.0017, 0.0974},
                                               one_in_one_out{0.0087, 0.4985}, one_in_one_out{0.0175, 1.0027},
                                               one_in_one_out{0.0349, 1.9996}, one_in_one_out{0.0524, 3.0023},
                                               one_in_one_out{0.1745, 9.9981}, one_in_one_out{1, 57.2958},
                                               one_in_one_out{2, 114.5916}, one_in_one_out{3, 171.8873},
                                               one_in_one_out{10, 572.9578}, one_in_one_out{17.4533, 1000.0004},
                                               one_in_one_out{1000, 57295.7795}, one_in_one_out{0, 0}));

    // --- Pressure ---
    class kpa_to_hpa : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(kpa_to_hpa, value)
    {
        expect_equal_rounded(GetParam().out, unit_converters::kilopascals_to_hectopascals(GetParam().in), 4);
    }
    INSTANTIATE_TEST_SUITE_P(table, kpa_to_hpa,
                             ::testing::Values(one_in_one_out{-1, -10}, one_in_one_out{0.1, 1}, one_in_one_out{1, 10},
                                               one_in_one_out{10, 100}, one_in_one_out{0, 0}));

    class hpa_to_kpa : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(hpa_to_kpa, value)
    {
        expect_equal_rounded(GetParam().out, unit_converters::hectopascals_to_kilopascals(GetParam().in), 4);
    }
    INSTANTIATE_TEST_SUITE_P(table, hpa_to_kpa,
                             ::testing::Values(one_in_one_out{-10, -1}, one_in_one_out{1, 0.1}, one_in_one_out{10, 1},
                                               one_in_one_out{100, 10}, one_in_one_out{0, 0}));

    class kpa_to_pa : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(kpa_to_pa, value)
    {
        expect_equal_rounded(GetParam().out, unit_converters::kilopascals_to_pascals(GetParam().in), 4);
    }
    INSTANTIATE_TEST_SUITE_P(table, kpa_to_pa,
                             ::testing::Values(one_in_one_out{0, 0}, one_in_one_out{1, 1000}, one_in_one_out{2.5, 2500},
                                               one_in_one_out{-1, -1000}));

    class hpa_to_pa : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(hpa_to_pa, value)
    {
        expect_equal_rounded(GetParam().out, unit_converters::hectopascals_to_pascals(GetParam().in), 4);
    }
    INSTANTIATE_TEST_SUITE_P(table, hpa_to_pa,
                             ::testing::Values(one_in_one_out{0, 0}, one_in_one_out{1, 100}, one_in_one_out{10, 1000},
                                               one_in_one_out{2.5, 250}, one_in_one_out{-1, -100}));

    class atm_to_pa : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(atm_to_pa, value)
    {
        // C# Assert.Equal(actual, expected) here — exact equality.
        EXPECT_DOUBLE_EQ(unit_converters::atmospheres_to_pascals(GetParam().in), GetParam().out);
    }
    INSTANTIATE_TEST_SUITE_P(table, atm_to_pa,
                             ::testing::Values(one_in_one_out{1.0, 101325}, one_in_one_out{1.5, 151987.5},
                                               one_in_one_out{2.0, 202650}, one_in_one_out{2.5, 253312.5}));

    class pa_to_atm : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(pa_to_atm, value)
    {
        EXPECT_DOUBLE_EQ(unit_converters::pascals_to_atmospheres(GetParam().in), GetParam().out);
    }
    INSTANTIATE_TEST_SUITE_P(table, pa_to_atm,
                             ::testing::Values(one_in_one_out{101325, 1.0}, one_in_one_out{151987.5, 1.5},
                                               one_in_one_out{202650, 2.0}, one_in_one_out{253312.5, 2.5}));

    // --- Angle ---
    class deg_to_rad : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(deg_to_rad, value)
    {
        expect_equal_rounded(GetParam().out, unit_converters::degrees_to_radians(GetParam().in), 4);
    }
    INSTANTIATE_TEST_SUITE_P(table, deg_to_rad,
                             ::testing::Values(one_in_one_out{-1, -0.0175}, one_in_one_out{0.1, 0.0017},
                                               one_in_one_out{1, 0.0175}, one_in_one_out{10, 0.1745},
                                               one_in_one_out{180, 3.1416}, one_in_one_out{360, 6.2832},
                                               one_in_one_out{10313.2403, 180}, one_in_one_out{0, 0}));

    class rad_to_deg : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(rad_to_deg, value)
    {
        expect_equal_rounded(GetParam().out, unit_converters::radians_to_degrees(GetParam().in), 4);
    }
    INSTANTIATE_TEST_SUITE_P(table, rad_to_deg,
                             ::testing::Values(one_in_one_out{-1, -57.2958}, one_in_one_out{0.1, 5.7296},
                                               one_in_one_out{1, 57.2958}, one_in_one_out{3.1416, 180.0004},
                                               one_in_one_out{6.2832, 360.0008}, one_in_one_out{10, 572.9578},
                                               one_in_one_out{180, 10313.2403}, one_in_one_out{360, 20626.4806},
                                               one_in_one_out{0, 0}));

    // --- Distance ---
    class miles_to_km : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(miles_to_km, value)
    {
        expect_equal_rounded(GetParam().out, unit_converters::miles_to_kilometers(GetParam().in), 4);
    }
    INSTANTIATE_TEST_SUITE_P(table, miles_to_km,
                             ::testing::Values(one_in_one_out{-1, -1.6093}, one_in_one_out{0.1, 0.1609},
                                               one_in_one_out{1, 1.6093}, one_in_one_out{2, 3.2187},
                                               one_in_one_out{3, 4.828}, one_in_one_out{10, 16.0934},
                                               one_in_one_out{0, 0}));

    class km_to_miles : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(km_to_miles, value)
    {
        expect_equal_rounded(GetParam().out, unit_converters::kilometers_to_miles(GetParam().in), 4);
    }
    INSTANTIATE_TEST_SUITE_P(table, km_to_miles,
                             ::testing::Values(one_in_one_out{-1, -0.6214}, one_in_one_out{0.1, 0.0621},
                                               one_in_one_out{1, 0.6214}, one_in_one_out{2, 1.2427},
                                               one_in_one_out{3, 1.8641}, one_in_one_out{10, 6.2137},
                                               one_in_one_out{0, 0}));

    class miles_to_meters : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(miles_to_meters, value)
    {
        expect_equal_rounded(GetParam().out, unit_converters::miles_to_meters(GetParam().in), 4);
    }
    INSTANTIATE_TEST_SUITE_P(table, miles_to_meters,
                             ::testing::Values(one_in_one_out{0, 0}, one_in_one_out{1, 1609.344},
                                               one_in_one_out{0.5, 804.672}, one_in_one_out{2, 3218.688},
                                               one_in_one_out{-1, -1609.344}));

    class meters_to_intl_feet : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(meters_to_intl_feet, value)
    {
        EXPECT_DOUBLE_EQ(GetParam().out, unit_converters::meters_to_international_feet(GetParam().in));
    }
    INSTANTIATE_TEST_SUITE_P(table, meters_to_intl_feet, ::testing::Values(one_in_one_out{3048, 10000}));

    class intl_feet_to_meters : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(intl_feet_to_meters, value)
    {
        EXPECT_DOUBLE_EQ(GetParam().out, unit_converters::international_feet_to_meters(GetParam().in));
    }
    INSTANTIATE_TEST_SUITE_P(table, intl_feet_to_meters, ::testing::Values(one_in_one_out{20000, 6096}));

    class meters_to_us_feet : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(meters_to_us_feet, value)
    {
        EXPECT_DOUBLE_EQ(GetParam().out, unit_converters::meters_to_us_survey_feet(GetParam().in));
    }
    INSTANTIATE_TEST_SUITE_P(table, meters_to_us_feet, ::testing::Values(one_in_one_out{1200, 3937}));

    class us_feet_to_meters : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(us_feet_to_meters, value)
    {
        EXPECT_DOUBLE_EQ(GetParam().out, unit_converters::us_survey_feet_to_meters(GetParam().in));
    }
    INSTANTIATE_TEST_SUITE_P(table, us_feet_to_meters, ::testing::Values(one_in_one_out{7874, 2400}));

    // --- Weight ---
    class pounds_to_kg : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(pounds_to_kg, value)
    {
        expect_equal_rounded(GetParam().out, unit_converters::pounds_to_kilograms(GetParam().in), 4);
    }
    INSTANTIATE_TEST_SUITE_P(table, pounds_to_kg,
                             ::testing::Values(one_in_one_out{115, 52.1631}, one_in_one_out{65.9, 29.8917},
                                               one_in_one_out{180, 81.6466}, one_in_one_out{8, 3.6287},
                                               one_in_one_out{331.1, 150.1844}, one_in_one_out{0, 0}));

    class pounds_to_stones : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(pounds_to_stones, value)
    {
        expect_equal_rounded(GetParam().out, unit_converters::pounds_to_stones(GetParam().in), 4);
    }
    INSTANTIATE_TEST_SUITE_P(table, pounds_to_stones,
                             ::testing::Values(one_in_one_out{115, 8.2143}, one_in_one_out{65.9, 4.7071},
                                               one_in_one_out{180, 12.8571}, one_in_one_out{8, 0.5714},
                                               one_in_one_out{184.8, 13.2}, one_in_one_out{0, 0}));

    class stones_to_pounds : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(stones_to_pounds, value)
    {
        expect_equal_rounded(GetParam().out, unit_converters::stones_to_pounds(GetParam().in), 4);
    }
    INSTANTIATE_TEST_SUITE_P(table, stones_to_pounds,
                             ::testing::Values(one_in_one_out{14, 196}, one_in_one_out{10.8, 151.2},
                                               one_in_one_out{22.8, 319.2}, one_in_one_out{5, 70},
                                               one_in_one_out{16.85, 235.9}, one_in_one_out{0, 0}));

    class kg_to_pounds : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(kg_to_pounds, value)
    {
        expect_equal_rounded(GetParam().out, unit_converters::kilograms_to_pounds(GetParam().in), 4);
    }
    INSTANTIATE_TEST_SUITE_P(table, kg_to_pounds,
                             ::testing::Values(one_in_one_out{79.2, 174.6061}, one_in_one_out{94.6, 208.5573},
                                               one_in_one_out{67.0, 147.7097}, one_in_one_out{57, 125.6635},
                                               one_in_one_out{82.85, 182.6530}, one_in_one_out{0, 0}));

    // --- Temperature ---
    class fahrenheit_to_celsius : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(fahrenheit_to_celsius, value)
    {
        expect_equal_rounded(GetParam().out, unit_converters::fahrenheit_to_celsius(GetParam().in), 4);
    }
    INSTANTIATE_TEST_SUITE_P(table, fahrenheit_to_celsius,
                             ::testing::Values(one_in_one_out{32, 0}, one_in_one_out{212, 100},
                                               one_in_one_out{98.6, 37}, one_in_one_out{-40, -40},
                                               one_in_one_out{0, -17.7778}));

    class celsius_to_fahrenheit : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(celsius_to_fahrenheit, value)
    {
        expect_equal_rounded(GetParam().out, unit_converters::celsius_to_fahrenheit(GetParam().in), 4);
    }
    INSTANTIATE_TEST_SUITE_P(table, celsius_to_fahrenheit,
                             ::testing::Values(one_in_one_out{0, 32}, one_in_one_out{100, 212},
                                               one_in_one_out{37, 98.6}, one_in_one_out{-40, -40},
                                               one_in_one_out{-17.7778, 0}));

    class celsius_to_kelvin : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(celsius_to_kelvin, value)
    {
        expect_equal_rounded(GetParam().out, unit_converters::celsius_to_kelvin(GetParam().in), 4);
    }
    INSTANTIATE_TEST_SUITE_P(table, celsius_to_kelvin,
                             ::testing::Values(one_in_one_out{0, 273.15}, one_in_one_out{100, 373.15},
                                               one_in_one_out{-273.15, 0}, one_in_one_out{25, 298.15}));

    class kelvin_to_celsius : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(kelvin_to_celsius, value)
    {
        expect_equal_rounded(GetParam().out, unit_converters::kelvin_to_celsius(GetParam().in), 4);
    }
    INSTANTIATE_TEST_SUITE_P(table, kelvin_to_celsius,
                             ::testing::Values(one_in_one_out{273.15, 0}, one_in_one_out{373.15, 100},
                                               one_in_one_out{0, -273.15}, one_in_one_out{298.15, 25}));

    // --- Angular velocity (the hertz pivots) ---
    class dps_to_hz : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(dps_to_hz, value)
    {
        expect_equal_rounded(GetParam().out, unit_converters::degrees_per_second_to_hertz(GetParam().in), 4);
    }
    INSTANTIATE_TEST_SUITE_P(table, dps_to_hz,
                             ::testing::Values(one_in_one_out{0, 0}, one_in_one_out{360, 1}, one_in_one_out{180, 0.5},
                                               one_in_one_out{720, 2}, one_in_one_out{1, 0.0028},
                                               one_in_one_out{-360, -1}));

    class rps_to_hz : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(rps_to_hz, value)
    {
        expect_equal_rounded(GetParam().out, unit_converters::radians_per_second_to_hertz(GetParam().in), 4);
    }
    INSTANTIATE_TEST_SUITE_P(table, rps_to_hz,
                             ::testing::Values(one_in_one_out{0, 0}, one_in_one_out{6.2832, 1.0000},
                                               one_in_one_out{3.1416, 0.5000}, one_in_one_out{12.5664, 2.0000},
                                               one_in_one_out{-6.2832, -1.0000}));

    class hz_to_dps : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(hz_to_dps, value)
    {
        expect_equal_rounded(GetParam().out, unit_converters::hertz_to_degrees_per_second(GetParam().in), 4);
    }
    INSTANTIATE_TEST_SUITE_P(table, hz_to_dps,
                             ::testing::Values(one_in_one_out{0, 0}, one_in_one_out{1, 360}, one_in_one_out{0.5, 180},
                                               one_in_one_out{2, 720}, one_in_one_out{-1, -360}));

    class hz_to_rps : public ::testing::TestWithParam<one_in_one_out>
    {
    };
    TEST_P(hz_to_rps, value)
    {
        expect_equal_rounded(GetParam().out, unit_converters::hertz_to_radians_per_second(GetParam().in), 4);
    }
    INSTANTIATE_TEST_SUITE_P(table, hz_to_rps,
                             ::testing::Values(one_in_one_out{0, 0}, one_in_one_out{1, 6.2832},
                                               one_in_one_out{0.5, 3.1416}, one_in_one_out{2, 12.5664},
                                               one_in_one_out{-1, -6.2832}));

    // --- Coordinate distance (haversine) ---
    struct coords_row
    {
        double lat1;
        double lon1;
        double lat2;
        double lon2;
        double distance_km;
    };

    constexpr auto kCoordRows = std::to_array<coords_row>({
        {.lat1 = 55.85781, .lon1 = -4.24253, .lat2 = 51.509865, .lon2 = -0.118092, .distance_km = 554.3128},
        {.lat1 = 36.12, .lon1 = -86.67, .lat2 = 33.94, .lon2 = -118.40, .distance_km = 2886.4444},
        {.lat1 = 51.509865, .lon1 = -0.118092, .lat2 = -33.92528, .lon2 = 18.42389, .distance_km = 9671.1251},
        {.lat1 = 51.509865, .lon1 = -0.118092, .lat2 = 40.42028, .lon2 = -3.70577, .distance_km = 1263.4938},
        {.lat1 = 42.93708, .lon1 = -75.6107, .lat2 = -33.92528, .lon2 = 18.42389, .distance_km = 12789.5628},
        {.lat1 = 45.80721, .lon1 = 15.96757, .lat2 = 19.432608, .lon2 = -99.133209, .distance_km = 10264.4796},
        {.lat1 = 43.623409, .lon1 = -79.368683, .lat2 = 42.35866, .lon2 = -71.05674, .distance_km = 690.2032},
        {.lat1 = 37.720134, .lon1 = -122.182552, .lat2 = 37.720266, .lon2 = -122.181969, .distance_km = .0533},
    });

    class coordinates_to_km : public ::testing::TestWithParam<coords_row>
    {
    };
    TEST_P(coordinates_to_km, value)
    {
        const auto& p = GetParam();
        expect_equal_rounded(p.distance_km, unit_converters::coordinates_to_kilometers(p.lat1, p.lon1, p.lat2, p.lon2),
                             4);
    }
    INSTANTIATE_TEST_SUITE_P(table, coordinates_to_km, ::testing::ValuesIn(kCoordRows));

    class coordinates_to_mi : public ::testing::TestWithParam<coords_row>
    {
    };
    TEST_P(coordinates_to_mi, value)
    {
        const auto& p = GetParam();
        const double distance_miles = unit_converters::kilometers_to_miles(p.distance_km);
        expect_equal_rounded(distance_miles, unit_converters::coordinates_to_miles(p.lat1, p.lon1, p.lat2, p.lon2), 3);
    }
    INSTANTIATE_TEST_SUITE_P(table, coordinates_to_mi, ::testing::ValuesIn(kCoordRows));
} // namespace
