// Ports of the value-type halves of src/Essentials/test/UnitTests: the *_Comparison theories from
// Accelerometer/Gyroscope/Magnetometer/Compass/Barometer/OrientationSensor/DeviceDisplay_Tests.cs,
// the CoordinatesToKilometers/Miles theories from UnitConverters_Tests.cs (through
// Location.CalculateDistance), plus the port's own coverage for device_platform/device_idiom
// (DevicePlatform.shared.cs semantics), version_info::parse (Utils.ParseVersion) and the
// System.Numerics quaternion slice. Backend-agnostic: runs on headless, apple and ios.

#include <array>
#include <cmath>
#include <stdexcept>

#include <gtest/gtest.h>

#include "maui/essentials/device_display.hpp"
#include "maui/essentials/device_info.hpp"
#include "maui/essentials/location.hpp"
#include "maui/essentials/placemark.hpp"
#include "maui/essentials/sensor_types.hpp"
#include "maui/graphics/quaternion.hpp"

namespace
{
    using namespace maui::devices;
    using namespace maui::devices::sensors;

    // Accelerometer_Tests.Accelerometer_Comparison
    TEST(essentials_types, accelerometer_data_comparison)
    {
        const accelerometer_data zero(0.0F, 0.0F, 0.0F);
        EXPECT_TRUE(zero == accelerometer_data(0.0F, 0.0F, 0.0F));
        EXPECT_FALSE(zero == accelerometer_data(1.0F, 0.0F, 0.0F));
        EXPECT_FALSE(zero == accelerometer_data(0.0F, 1.0F, 0.0F));
        EXPECT_FALSE(zero == accelerometer_data(0.0F, 0.0F, 1.0F));
    }

    // Gyroscope_Tests.Gyroscope_Comparison
    TEST(essentials_types, gyroscope_data_comparison)
    {
        const gyroscope_data zero(0.0F, 0.0F, 0.0F);
        EXPECT_TRUE(zero == gyroscope_data(0.0F, 0.0F, 0.0F));
        EXPECT_FALSE(zero == gyroscope_data(1.0F, 0.0F, 0.0F));
        EXPECT_FALSE(zero == gyroscope_data(0.0F, 1.0F, 0.0F));
        EXPECT_FALSE(zero == gyroscope_data(0.0F, 0.0F, 1.0F));
    }

    // Magnetometer_Tests.Magnetometer_Comparison
    TEST(essentials_types, magnetometer_data_comparison)
    {
        const magnetometer_data zero(0.0F, 0.0F, 0.0F);
        EXPECT_TRUE(zero == magnetometer_data(0.0F, 0.0F, 0.0F));
        EXPECT_FALSE(zero == magnetometer_data(1.0F, 0.0F, 0.0F));
        EXPECT_FALSE(zero == magnetometer_data(0.0F, 1.0F, 0.0F));
        EXPECT_FALSE(zero == magnetometer_data(0.0F, 0.0F, 1.0F));
    }

    // Compass_Tests.CompassData_Comparison
    TEST(essentials_types, compass_data_comparison)
    {
        EXPECT_TRUE(compass_data(0.0) == compass_data(0.0));
        EXPECT_FALSE(compass_data(0.0) == compass_data(1.0));
    }

    // Barometer_Tests.BarometerData_Comparison
    TEST(essentials_types, barometer_data_comparison)
    {
        EXPECT_TRUE(barometer_data(0.0) == barometer_data(0.0));
        EXPECT_FALSE(barometer_data(0.0) == barometer_data(1.0));
    }

    // OrientationSensor_Tests.OrientationSensorData_Comparison
    TEST(essentials_types, orientation_sensor_data_comparison)
    {
        const orientation_sensor_data zero(0.0F, 0.0F, 0.0F, 0.0F);
        EXPECT_TRUE(zero == orientation_sensor_data(0.0F, 0.0F, 0.0F, 0.0F));
        EXPECT_FALSE(zero == orientation_sensor_data(1.0F, 0.0F, 0.0F, 0.0F));
        EXPECT_FALSE(zero == orientation_sensor_data(0.0F, 1.0F, 0.0F, 0.0F));
        EXPECT_FALSE(zero == orientation_sensor_data(0.0F, 0.0F, 1.0F, 0.0F));
        EXPECT_FALSE(zero == orientation_sensor_data(0.0F, 0.0F, 0.0F, 1.0F));
    }

    // DeviceDisplay_Tests.DeviceDisplay_Comparison (the representative rows; refresh_rate is NOT
    // part of equality - C# DisplayInfo.Equals ignores it).
    // The C# DisplayInfo(width, height, density, orientation, rotation) constructor shape.
    display_info make_display_info(double width, double height, double density, display_orientation orientation,
                                   display_rotation rotation)
    {
        return {.width = width,
                .height = height,
                .density = density,
                .orientation = orientation,
                .rotation = rotation,
                .refresh_rate = 0};
    }

    TEST(essentials_types, display_info_comparison)
    {
        const auto landscape0 = display_orientation::landscape;
        const auto rotation0 = display_rotation::rotation_0;
        const display_info base = make_display_info(0, 0, 0, landscape0, rotation0);
        EXPECT_TRUE(base == make_display_info(0, 0, 0, landscape0, rotation0));
        const display_info wide = make_display_info(1.1, 0, 2.2, landscape0, display_rotation::rotation_180);
        EXPECT_TRUE(wide == make_display_info(1.1, 0, 2.2, landscape0, display_rotation::rotation_180));
        EXPECT_FALSE(base == make_display_info(1.0, 0, 0, landscape0, rotation0));
        EXPECT_FALSE(base == make_display_info(0, 1.0, 0, landscape0, rotation0));
        EXPECT_FALSE(base == make_display_info(0, 0, 1.0, landscape0, rotation0));
        EXPECT_FALSE(base == make_display_info(0, 0, 0, display_orientation::portrait, rotation0));
        EXPECT_FALSE(base == make_display_info(0, 0, 0, landscape0, display_rotation::rotation_180));

        display_info with_rate = base;
        with_rate.refresh_rate = 60;
        EXPECT_TRUE(base == with_rate);
    }

    // DevicePlatform.shared.cs semantics: named statics, Create, equality, ToString.
    TEST(essentials_types, device_platform_semantics)
    {
        EXPECT_EQ(device_platform::ios().to_string(), "iOS");
        EXPECT_EQ(device_platform::mac_os().to_string(), "macOS");
        EXPECT_EQ(device_platform::mac_catalyst().to_string(), "MacCatalyst");
        EXPECT_TRUE(device_platform::android() == device_platform::create("Android"));
        EXPECT_FALSE(device_platform::ios() == device_platform::mac_os());
        EXPECT_TRUE(device_platform() == device_platform());
        EXPECT_EQ(device_platform().to_string(), "");
        EXPECT_THROW((void)device_platform::create(""), std::invalid_argument);
    }

    // DeviceIdiom.shared.cs semantics.
    TEST(essentials_types, device_idiom_semantics)
    {
        EXPECT_EQ(device_idiom::phone().to_string(), "Phone");
        EXPECT_EQ(device_idiom::desktop().to_string(), "Desktop");
        EXPECT_TRUE(device_idiom::tv() == device_idiom::create("TV"));
        EXPECT_FALSE(device_idiom::watch() == device_idiom::tablet());
        EXPECT_THROW((void)device_idiom::create(""), std::invalid_argument);
    }

    // Utils.ParseVersion: full parse, bare major, garbage -> 0.0.
    TEST(essentials_types, version_parse)
    {
        EXPECT_EQ(version_info::parse("14.2.1"), (version_info{14, 2, 1, -1}));
        EXPECT_EQ(version_info::parse("17.5"), (version_info{17, 5, -1, -1}));
        EXPECT_EQ(version_info::parse("1.2.3.4"), (version_info{1, 2, 3, 4}));
        EXPECT_EQ(version_info::parse("12"), (version_info{12, 0, -1, -1}));
        EXPECT_EQ(version_info::parse("not a version"), (version_info{0, 0, -1, -1}));
        EXPECT_EQ(version_info::parse(""), (version_info{0, 0, -1, -1}));
        EXPECT_EQ(version_info::parse("1.2.3.4.5"), (version_info{0, 0, -1, -1}));
        EXPECT_EQ(version_info::parse("1."), (version_info{0, 0, -1, -1}));
        EXPECT_EQ((version_info{14, 2, 1, -1}).to_string(), "14.2.1");
        EXPECT_EQ((version_info{17, 5, -1, -1}).to_string(), "17.5");
    }

    // Location.Equals: latitude + longitude only.
    TEST(essentials_types, location_equality)
    {
        const location a(47.0, 8.0);
        location b(47.0, 8.0);
        b.altitude = 100.0;
        b.accuracy = 5.0;
        EXPECT_TRUE(a == b);
        EXPECT_FALSE(a == location(47.0, 9.0));
    }

    // UnitConverters_Tests.CoordinatesToKilometers / CoordinatesToMiles (through CalculateDistance;
    // tolerance mirrors xunit's 4 / 3 decimal-place precision).
    TEST(essentials_types, location_calculate_distance)
    {
        struct row
        {
            constexpr row(double lat1_, double lon1_, double lat2_, double lon2_, double km_)
                : lat1(lat1_), lon1(lon1_), lat2(lat2_), lon2(lon2_), km(km_)
            {
            }
            double lat1, lon1, lat2, lon2, km;
        };
        constexpr std::array<row, 8> rows{
            row{55.85781, -4.24253, 51.509865, -0.118092, 554.3128},    // glasgow -> london
            row{36.12, -86.67, 33.94, -118.40, 2886.4444},              // nashville -> los angeles
            row{51.509865, -0.118092, -33.92528, 18.42389, 9671.1251},  // london -> cape town
            row{51.509865, -0.118092, 40.42028, -3.70577, 1263.4938},   // london -> madrid
            row{42.93708, -75.6107, -33.92528, 18.42389, 12789.5628},   // new york -> cape town
            row{45.80721, 15.96757, 19.432608, -99.133209, 10264.4796}, // zagreb -> mexico city
            row{43.623409, -79.368683, 42.35866, -71.05674, 690.2032},  // toronto -> boston
            row{37.720134, -122.182552, 37.720266, -122.181969, 0.0533},
        };
        constexpr double kilometers_to_miles = 1.0 / 1.609344;
        for (const auto& r : rows)
        {
            const location from(r.lat1, r.lon1);
            const location to(r.lat2, r.lon2);
            EXPECT_NEAR(location::calculate_distance(from, to, distance_units::kilometers), r.km, 5e-5);
            EXPECT_NEAR(location::calculate_distance(to, from, distance_units::kilometers), r.km, 5e-5);
            EXPECT_NEAR(location::calculate_distance(r.lat1, r.lon1, to, distance_units::miles),
                        r.km * kilometers_to_miles, 5e-4);
            EXPECT_NEAR(location::calculate_distance(from, r.lat2, r.lon2, distance_units::miles),
                        r.km * kilometers_to_miles, 5e-4);
        }
        EXPECT_EQ(location::calculate_distance(1, 1, 1, 1, distance_units::kilometers), 0.0);
    }

    // The System.Numerics.Quaternion slice: Multiply + the identity.
    TEST(essentials_types, quaternion_multiply)
    {
        using maui::graphics::quaternion;
        constexpr quaternion identity;
        constexpr quaternion q(0.1F, 0.2F, 0.3F, 0.9F);
        EXPECT_TRUE(quaternion::multiply(identity, q) == q);
        EXPECT_TRUE(quaternion::multiply(q, identity) == q);

        // A 90-degree Z rotation composed with itself is a 180-degree Z rotation.
        const float half = std::sqrt(0.5F);
        const quaternion z90(0, 0, half, half);
        const quaternion z180 = quaternion::multiply(z90, z90);
        EXPECT_NEAR(z180.x, 0.0F, 1e-6F);
        EXPECT_NEAR(z180.y, 0.0F, 1e-6F);
        EXPECT_NEAR(z180.z, 1.0F, 1e-6F);
        EXPECT_NEAR(z180.w, 0.0F, 1e-6F);
    }

    // Placemark: the value-struct surface (C#'s copy constructor is the defaulted one here).
    TEST(essentials_types, placemark_copies)
    {
        placemark p;
        p.location = location(1.0, 2.0);
        p.country_code = "CH";
        p.locality = "Zurich";
        const placemark copy = p;
        EXPECT_TRUE(copy.location == p.location);
        EXPECT_EQ(copy.country_code, "CH");
        EXPECT_EQ(copy.locality, "Zurich");
    }
} // namespace
