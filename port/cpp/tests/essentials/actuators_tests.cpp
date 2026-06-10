// flashlight / vibration / haptic_feedback on the headless backend: netstandard mirrors when
// unconfigured (flashlight is-supported is FALSE per the C# partial, everything else throws), and
// the full surface via the fakes - including VibrationImplementation's shared clamp ([0, 5000] ms)
// and gate behavior.

#include <chrono>
#include <memory>
#include <optional>

#include <gtest/gtest.h>

#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/flashlight.hpp"
#include "maui/essentials/haptic_feedback.hpp"
#include "maui/essentials/vibration.hpp"

#include "src/platform/headless/essentials_fakes.hpp"

namespace
{
    using namespace maui::devices;
    using maui::application_model::feature_not_supported;

    class actuators_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            reset();
        }
        void TearDown() override
        {
            reset();
        }
        static void reset()
        {
            flashlight::set_default(nullptr);
            vibration::set_default(nullptr);
            haptic_feedback::set_default(nullptr);
        }
    };

    TEST_F(actuators_test, flashlight_netstandard_mirror)
    {
        EXPECT_FALSE(flashlight::is_supported()); // C#: Task.FromResult(false), not a throw
        EXPECT_THROW(flashlight::turn_on(), feature_not_supported);
        EXPECT_THROW(flashlight::turn_off(), feature_not_supported);
    }

    TEST_F(actuators_test, flashlight_fake_toggles)
    {
        auto fake = std::make_shared<headless_flashlight>();
        fake->set_is_supported(true);
        flashlight::set_default(fake);

        EXPECT_TRUE(flashlight::is_supported());
        flashlight::turn_on();
        EXPECT_TRUE(fake->is_on());
        flashlight::turn_off();
        EXPECT_FALSE(fake->is_on());
    }

    TEST_F(actuators_test, vibration_netstandard_mirror)
    {
        EXPECT_THROW((void)vibration::is_supported(), feature_not_supported);
        EXPECT_THROW(vibration::vibrate(), feature_not_supported);
        EXPECT_THROW(vibration::vibrate(std::chrono::milliseconds{100}), feature_not_supported);
        EXPECT_THROW(vibration::cancel(), feature_not_supported);
    }

    TEST_F(actuators_test, vibration_unsupported_gate_throws)
    {
        auto fake = std::make_shared<headless_vibration>();
        fake->set_is_supported(false);
        vibration::set_default(fake);

        EXPECT_FALSE(vibration::is_supported());
        EXPECT_THROW(vibration::vibrate(), feature_not_supported);
        EXPECT_THROW(vibration::cancel(), feature_not_supported);
    }

    TEST_F(actuators_test, vibration_clamps_duration)
    {
        auto fake = std::make_shared<headless_vibration>();
        fake->set_is_supported(true);
        vibration::set_default(fake);

        vibration::vibrate();
        ASSERT_TRUE(fake->last_duration().has_value());
        EXPECT_EQ(fake->last_duration(), std::optional(std::chrono::milliseconds{500}));

        vibration::vibrate(std::chrono::milliseconds{-10}); // negative -> zero
        EXPECT_EQ(fake->last_duration(), std::optional(std::chrono::milliseconds{0}));

        vibration::vibrate(std::chrono::milliseconds{9000}); // > 5 s -> 5 s
        EXPECT_EQ(fake->last_duration(), std::optional(std::chrono::milliseconds{5000}));

        vibration::vibrate(std::chrono::milliseconds{1234});
        EXPECT_EQ(fake->last_duration(), std::optional(std::chrono::milliseconds{1234}));
        EXPECT_TRUE(fake->is_vibrating());

        vibration::cancel();
        EXPECT_FALSE(fake->is_vibrating());
    }

    TEST_F(actuators_test, haptic_feedback_netstandard_mirror)
    {
        EXPECT_THROW((void)haptic_feedback::is_supported(), feature_not_supported);
        EXPECT_THROW(haptic_feedback::perform(), feature_not_supported);
    }

    TEST_F(actuators_test, haptic_feedback_fake_records_type)
    {
        auto fake = std::make_shared<headless_haptic_feedback>();
        fake->set_is_supported(true);
        haptic_feedback::set_default(fake);

        EXPECT_TRUE(haptic_feedback::is_supported());
        haptic_feedback::perform(); // C# default argument: Click
        ASSERT_TRUE(fake->last_performed().has_value());
        EXPECT_EQ(fake->last_performed(), std::optional(haptic_feedback_type::click));

        haptic_feedback::perform(haptic_feedback_type::long_press);
        EXPECT_EQ(fake->last_performed(), std::optional(haptic_feedback_type::long_press));
    }
} // namespace
