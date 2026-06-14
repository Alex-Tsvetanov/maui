// text_to_speech on the headless backend. Ports TextToSpeech_Tests.cs (the netstandard speak
// throws; the SplitSpeak chunker) and exercises the shared validation + the configured fake: the
// argument guards (empty text, out-of-range volume/pitch/rate), the recorded utterances, the
// locale query, and the cancellation-completes contract.

#include <atomic>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "maui/core/cancellation_token.hpp"
#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/text_to_speech.hpp"

#include "src/platform/headless/essentials_fakes.hpp"

namespace
{
    using namespace maui::media;
    using maui::application_model::feature_not_supported;

    class text_to_speech_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            text_to_speech::set_default(nullptr);
        }
        void TearDown() override
        {
            text_to_speech::set_default(nullptr);
        }
    };

    // TextToSpeech_Tests.TextToSpeech_Speak_Fail_On_NetStandard: speak throws on netstandard.
    TEST_F(text_to_speech_test, netstandard_mirror_speak_throws)
    {
        EXPECT_THROW(text_to_speech::speak_async("Maui Essentials!"), feature_not_supported);
    }

    TEST_F(text_to_speech_test, netstandard_mirror_get_locales_throws)
    {
        EXPECT_THROW(text_to_speech::get_locales_async([](const std::vector<locale>&) {}), feature_not_supported);
    }

    // TextToSpeech_Tests.TextToSpeech_Slit_Text: the lorem text splits into 2 parts at max 4000.
    TEST_F(text_to_speech_test, split_speak_splits_long_text)
    {
        std::string text;
        // Build > 4000 chars of space-separated words (a long run that needs exactly one split).
        while (text.length() <= 4000)
        {
            text += "lorem ipsum dolor sit amet ";
        }
        // Ensure the total exceeds 4000 but stays under 8000 (one split -> two parts).
        const auto parts = detail::split_speak(text, 4000);
        EXPECT_EQ(parts.size(), 2U);
        EXPECT_LE(parts[0].length(), 4000U);
    }

    TEST_F(text_to_speech_test, split_speak_short_text_is_one_part)
    {
        const auto parts = detail::split_speak("short text", 4000);
        ASSERT_EQ(parts.size(), 1U);
        EXPECT_EQ(parts[0], "short text");
    }

    TEST_F(text_to_speech_test, configured_fake_records_utterance)
    {
        auto fake = std::make_shared<headless_text_to_speech>();
        fake->set_locales({locale{.language = "en", .country = "", .name = "English", .id = "en-US"}});
        text_to_speech::set_default(fake);

        bool completed = false;
        text_to_speech::speak_async("hello world", {}, {}, [&] { completed = true; });
        EXPECT_TRUE(completed);
        ASSERT_EQ(fake->spoken_utterances().size(), 1U);
        EXPECT_EQ(fake->spoken_utterances()[0].text, "hello world");
        EXPECT_FALSE(fake->spoken_utterances()[0].cancelled);
    }

    TEST_F(text_to_speech_test, get_locales_returns_staged)
    {
        auto fake = std::make_shared<headless_text_to_speech>();
        fake->set_locales({locale{.language = "en", .country = "", .name = "English", .id = "en-US"},
                           locale{.language = "fr", .country = "", .name = "French", .id = "fr-FR"}});
        text_to_speech::set_default(fake);

        std::vector<locale> got;
        text_to_speech::get_locales_async([&](const std::vector<locale>& locales) { got = locales; });
        ASSERT_EQ(got.size(), 2U);
        EXPECT_EQ(got[0].id, "en-US");
        EXPECT_EQ(got[1].language, "fr");
    }

    TEST_F(text_to_speech_test, speak_validates_arguments)
    {
        auto fake = std::make_shared<headless_text_to_speech>();
        fake->set_locales({});
        text_to_speech::set_default(fake);

        EXPECT_THROW(text_to_speech::speak_async(""), std::invalid_argument);

        speech_options bad_volume;
        bad_volume.volume = 1.5F;
        EXPECT_THROW(text_to_speech::speak_async("hi", bad_volume), std::out_of_range);

        speech_options bad_pitch;
        bad_pitch.pitch = 2.5F;
        EXPECT_THROW(text_to_speech::speak_async("hi", bad_pitch), std::out_of_range);

        speech_options bad_rate;
        bad_rate.rate = 0.05F;
        EXPECT_THROW(text_to_speech::speak_async("hi", bad_rate), std::out_of_range);

        // In-range values are accepted.
        speech_options ok;
        ok.volume = 0.5F;
        ok.pitch = 1.0F;
        ok.rate = 1.0F;
        EXPECT_NO_THROW(text_to_speech::speak_async("hi", ok));
    }

    TEST_F(text_to_speech_test, cancelled_token_still_completes_and_records)
    {
        auto fake = std::make_shared<headless_text_to_speech>();
        fake->set_locales({});
        text_to_speech::set_default(fake);

        auto flag = std::make_shared<std::atomic<bool>>(true);
        const maui::core::cancellation_token token(flag);

        bool completed = false;
        text_to_speech::speak_async("hello", {}, token, [&] { completed = true; });
        EXPECT_TRUE(completed);
        ASSERT_EQ(fake->spoken_utterances().size(), 1U);
        EXPECT_TRUE(fake->spoken_utterances()[0].cancelled);
    }
} // namespace
