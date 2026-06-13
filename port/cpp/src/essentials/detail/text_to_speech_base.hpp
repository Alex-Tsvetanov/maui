#pragma once
// maui::media::detail::text_to_speech_base  <=  the cross-platform half of
// Microsoft.Maui.Media.TextToSpeechImplementation (TextToSpeech.shared.cs): the SpeakAsync
// argument validation that runs before the platform partial.
//
// SpeakAsync validates: text must be non-empty (ArgumentNullException -> std::invalid_argument);
// when options carry a volume/pitch/rate, each must be within its bound (ArgumentOutOfRangeException
// -> std::out_of_range) - the [0,1]/[0,2]/[0.1,2] ranges from the internal const set. C#'s
// SemaphoreSlim(1,1) serialization (one utterance awaited at a time) is delegated to the platform
// engine, which queues utterances natively (AVSpeechSynthesizer / NSSpeechSynthesizer); the
// callback model has no Task to await, so there is nothing to serialize at this layer. The value
// getter side (get_locales / the actual speak) stays in the platform partial via the two
// platform_* hooks.

#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "maui/core/cancellation_token.hpp"
#include "maui/essentials/text_to_speech.hpp"

namespace maui::media::detail
{
    class text_to_speech_base : public i_text_to_speech
    {
    public:
        void get_locales_async(locales_callback on_complete) override
        {
            platform_get_locales_async(std::move(on_complete));
        }

        void speak_async(std::string_view text, const std::optional<speech_options>& options,
                         maui::core::cancellation_token token, speak_callback on_complete) override
        {
            if (text.empty())
            {
                throw std::invalid_argument("Text cannot be null or empty string");
            }
            if (options.has_value())
            {
                if (options->volume.has_value() && (*options->volume < volume_min || *options->volume > volume_max))
                {
                    throw std::out_of_range("Volume must be >= 0 and <= 1");
                }
                if (options->pitch.has_value() && (*options->pitch < pitch_min || *options->pitch > pitch_max))
                {
                    throw std::out_of_range("Pitch must be >= 0 and <= 2");
                }
                if (options->rate.has_value() && (*options->rate < rate_min || *options->rate > rate_max))
                {
                    throw std::out_of_range("Rate must be >= 0.1 and <= 2");
                }
            }
            platform_speak_async(text, options, token, std::move(on_complete));
        }

    protected:
        text_to_speech_base() = default;

        // PlatformGetLocalesAsync / PlatformSpeakAsync (the platform partial).
        virtual void platform_get_locales_async(locales_callback on_complete) = 0;
        virtual void platform_speak_async(std::string_view text, const std::optional<speech_options>& options,
                                          maui::core::cancellation_token token, speak_callback on_complete) = 0;
    };
} // namespace maui::media::detail
