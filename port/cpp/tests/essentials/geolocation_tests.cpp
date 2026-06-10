// geolocation on the headless backend: the netstandard mirror (is_enabled/is_listening false,
// queries throw), the not-enabled fold (FeatureNotEnabledException -> feature_not_supported with
// the C# message), the async query callbacks (inline on headless), the poll-style cancellation
// token, and the foreground-listening contract (double-listen logic_error; a listening failure
// stops listening BEFORE ListeningFailed is raised).

#include <atomic>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include "maui/core/cancellation_token.hpp"
#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/geolocation.hpp"
#include "maui/essentials/location.hpp"

#include "src/platform/headless/essentials_fakes.hpp"

namespace
{
    using namespace maui::devices::sensors;
    using maui::application_model::feature_not_supported;

    class geolocation_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            geolocation::set_default(nullptr);
        }
        void TearDown() override
        {
            geolocation::set_default(nullptr);
        }
    };

    TEST_F(geolocation_test, netstandard_mirror)
    {
        EXPECT_FALSE(geolocation::is_enabled());
        EXPECT_FALSE(geolocation::is_listening_foreground());
        EXPECT_THROW(geolocation::get_last_known_location_async([](const std::optional<location>&) {}),
                     feature_not_supported);
        EXPECT_THROW(geolocation::get_location_async([](const std::optional<location>&) {}), feature_not_supported);
        EXPECT_THROW((void)geolocation::start_listening_foreground({}), feature_not_supported);
    }

    TEST_F(geolocation_test, not_enabled_folds_into_feature_not_supported)
    {
        auto fake = std::make_shared<headless_geolocation>();
        fake->set_is_enabled(false); // configured, but location services off
        fake->set_current_location(location(1, 2));
        geolocation::set_default(fake);

        try
        {
            geolocation::get_location_async([](const std::optional<location>&) {});
            FAIL() << "expected feature_not_supported";
        }
        catch (const feature_not_supported& error)
        {
            EXPECT_EQ(std::string(error.what()), "Location services are not enabled on device.");
        }
        EXPECT_THROW((void)geolocation::start_listening_foreground({}), feature_not_supported);
    }

    TEST_F(geolocation_test, queries_complete_with_staged_locations)
    {
        auto fake = std::make_shared<headless_geolocation>();
        fake->set_is_enabled(true);
        fake->set_last_known_location(location(47.3769, 8.5417));
        fake->set_current_location(location(48.1351, 11.5820));
        geolocation::set_default(fake);

        std::optional<location> last_known;
        geolocation::get_last_known_location_async([&](const std::optional<location>& value) { last_known = value; });
        ASSERT_TRUE(last_known.has_value());
        EXPECT_TRUE(last_known == std::optional<location>(location(47.3769, 8.5417)));

        std::optional<location> current;
        geolocation_request request;
        request.desired_accuracy = geolocation_accuracy::best;
        geolocation::get_location_async(request, [&](const std::optional<location>& value) { current = value; });
        ASSERT_TRUE(current.has_value());
        EXPECT_TRUE(current == std::optional<location>(location(48.1351, 11.5820)));
        ASSERT_TRUE(fake->last_request().has_value());
        EXPECT_EQ(fake->last_request().value_or(geolocation_request{}).desired_accuracy, geolocation_accuracy::best);

        // A nullopt stage mirrors "no location could be determined".
        fake->set_current_location(std::nullopt);
        bool completed = false;
        geolocation::get_location_async([&](const std::optional<location>& value) {
            completed = true;
            EXPECT_FALSE(value.has_value());
        });
        EXPECT_TRUE(completed);
    }

    TEST_F(geolocation_test, cancelled_token_completes_with_no_location)
    {
        auto fake = std::make_shared<headless_geolocation>();
        fake->set_is_enabled(true);
        fake->set_current_location(location(1, 1));
        geolocation::set_default(fake);

        auto flag = std::make_shared<std::atomic<bool>>(false);
        const maui::core::cancellation_token token(flag);
        token.cancel();

        bool completed = false;
        geolocation::get_location_async({}, token, [&](const std::optional<location>& value) {
            completed = true;
            EXPECT_FALSE(value.has_value());
        });
        EXPECT_TRUE(completed);
    }

    TEST_F(geolocation_test, foreground_listening_contract)
    {
        auto fake = std::make_shared<headless_geolocation>();
        fake->set_is_enabled(true);
        geolocation::set_default(fake);

        geolocation_listening_request request;
        request.desired_accuracy = geolocation_accuracy::high;
        EXPECT_TRUE(geolocation::start_listening_foreground(request));
        EXPECT_TRUE(geolocation::is_listening_foreground());
        ASSERT_TRUE(fake->last_listening_request().has_value());
        EXPECT_EQ(fake->last_listening_request().value_or(geolocation_listening_request{}).desired_accuracy,
                  geolocation_accuracy::high);

        // Already listening -> InvalidOperationException("Already listening to location changes.").
        try
        {
            (void)geolocation::start_listening_foreground(request);
            FAIL() << "expected std::logic_error";
        }
        catch (const std::logic_error& error)
        {
            EXPECT_EQ(std::string(error.what()), "Already listening to location changes.");
        }

        int updates = 0;
        location last{};
        geolocation::location_changed().connect([&](const location& value) {
            ++updates;
            last = value;
        });
        fake->simulate_location_update(location(50.0, 14.4));
        EXPECT_EQ(updates, 1);
        EXPECT_TRUE(last == location(50.0, 14.4));

        geolocation::stop_listening_foreground();
        EXPECT_FALSE(geolocation::is_listening_foreground());
        geolocation::stop_listening_foreground(); // no-op while not listening

        fake->simulate_location_update(location(51.0, 15.0)); // dropped - not listening
        EXPECT_EQ(updates, 1);
    }

    TEST_F(geolocation_test, listening_failure_stops_before_raising)
    {
        auto fake = std::make_shared<headless_geolocation>();
        fake->set_is_enabled(true);
        geolocation::set_default(fake);
        ASSERT_TRUE(geolocation::start_listening_foreground({}));

        int failures = 0;
        geolocation_error last_error{};
        geolocation::listening_failed().connect([&](const geolocation_error& error) {
            ++failures;
            last_error = error;
            // The C# contract: listening has already stopped when the event arrives.
            EXPECT_FALSE(geolocation::is_listening_foreground());
        });

        fake->simulate_listening_failed(geolocation_error::position_unavailable);
        EXPECT_EQ(failures, 1);
        EXPECT_EQ(last_error, geolocation_error::position_unavailable);

        fake->simulate_listening_failed(geolocation_error::unauthorized); // not listening -> dropped
        EXPECT_EQ(failures, 1);
    }
} // namespace
