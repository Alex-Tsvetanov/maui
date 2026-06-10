// geocoding on the headless backend. Ports Geocoding_Tests.cs (every query throws on netstandard)
// and exercises the staged async results through the facade - geocoding needs network on the real
// backends (CLGeocoder), so this fake IS the feature's test path.

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/geocoding.hpp"
#include "maui/essentials/location.hpp"
#include "maui/essentials/placemark.hpp"

#include "src/platform/headless/essentials_fakes.hpp"

namespace
{
    using namespace maui::devices::sensors;
    using maui::application_model::feature_not_supported;

    class geocoding_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            geocoding::set_current(nullptr);
        }
        void TearDown() override
        {
            geocoding::set_current(nullptr);
        }
    };

    // Geocoding_Tests: Placemarks / Placemarks(Location) / Locations all throw on netstandard.
    TEST_F(geocoding_test, netstandard_mirror)
    {
        EXPECT_THROW(geocoding::get_placemarks_async(1, 1, [](const std::vector<placemark>&) {}),
                     feature_not_supported);
        EXPECT_THROW(geocoding::get_placemarks_async(location(1, 1), [](const std::vector<placemark>&) {}),
                     feature_not_supported);
        EXPECT_THROW(geocoding::get_locations_async("Microsoft Building 25", [](const std::vector<location>&) {}),
                     feature_not_supported);
    }

    TEST_F(geocoding_test, staged_placemarks_round_trip)
    {
        auto fake = std::make_shared<headless_geocoding>();
        placemark zurich;
        zurich.location = location(47.3769, 8.5417);
        zurich.locality = "Zurich";
        zurich.country_code = "CH";
        fake->set_placemarks({zurich});
        geocoding::set_current(fake);

        std::vector<placemark> result;
        geocoding::get_placemarks_async(47.4, 8.5, [&](const std::vector<placemark>& value) { result = value; });
        ASSERT_EQ(result.size(), 1U);
        EXPECT_EQ(result[0].locality, "Zurich");
        EXPECT_EQ(result[0].country_code, "CH");
        ASSERT_TRUE(fake->last_coordinates().has_value());
        EXPECT_TRUE(fake->last_coordinates() == std::optional<location>(location(47.4, 8.5)));

        // The Location overload forwards its coordinates.
        geocoding::get_placemarks_async(location(1.5, 2.5), [](const std::vector<placemark>&) {});
        EXPECT_TRUE(fake->last_coordinates() == std::optional<location>(location(1.5, 2.5)));
    }

    TEST_F(geocoding_test, staged_locations_round_trip)
    {
        auto fake = std::make_shared<headless_geocoding>();
        fake->set_locations({location(47.6204, -122.3491)});
        geocoding::set_current(fake);

        std::vector<location> result;
        geocoding::get_locations_async("Space Needle", [&](const std::vector<location>& value) { result = value; });
        ASSERT_EQ(result.size(), 1U);
        EXPECT_TRUE(result[0] == location(47.6204, -122.3491));
        ASSERT_TRUE(fake->last_address().has_value());
        EXPECT_EQ(fake->last_address(), std::optional<std::string>("Space Needle"));

        // An empty stage mirrors "no matches" (the C# `?? Array.Empty<T>()`).
        fake->set_locations({});
        bool completed = false;
        geocoding::get_locations_async("nowhere", [&](const std::vector<location>& value) {
            completed = true;
            EXPECT_TRUE(value.empty());
        });
        EXPECT_TRUE(completed);
    }
} // namespace
