#pragma once
// maui::media::text_to_speech    <=  Microsoft.Maui.Media.TextToSpeech (static facade)
// maui::media::i_text_to_speech  <=  Microsoft.Maui.Media.ITextToSpeech
// maui::media::locale            <=  Microsoft.Maui.Media.Locale
// maui::media::speech_options    <=  Microsoft.Maui.Media.SpeechOptions
//
// Speaks text via the device's TTS engine and queries the supported locales. The C# Task surface
// becomes the library's callback convention (the geocoding/geolocation precedent - no task type):
// get_locales_async hands a Locale vector to its callback; speak_async runs an optional completion
// callback when the utterance finishes (or is cancelled). The shared TextToSpeechImplementation
// validation + one-at-a-time serialization (the SemaphoreSlim(1,1)) live in
// detail::text_to_speech_base; PlatformGetLocalesAsync / PlatformSpeakAsync are the platform
// partials (pure virtual). C#'s argument exceptions map per the lib rule: empty text ->
// std::invalid_argument (ArgumentNullException), an out-of-range volume/pitch/rate ->
// std::out_of_range (ArgumentOutOfRangeException). The internal-but-tested SplitSpeak is
// detail::split_speak.
//
// Backends (suffix oracle): apple/macOS REAL (TextToSpeech.macos.cs - NSSpeechSynthesizer), ios
// REAL (TextToSpeech.ios.tvos.watchos.cs - AVSpeechSynthesizer). Headless mirrors netstandard
// (TextToSpeech.netstandard.cs throws) until the fake is configured (then it records utterances).

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/core/cancellation_token.hpp"
#include "maui/core/move_only_function.hpp"

namespace maui::media
{
    // Locale: a TTS language/voice (the C# fields; Country is unused/empty on Apple).
    struct locale
    {
        std::string language;
        std::string country;
        std::string name;
        std::string id;
    };

    // SpeechOptions: optional knobs for an utterance. The nullable C# floats become std::optional.
    // Ranges (enforced by the shared validation): pitch [0, 2], volume [0, 1], rate [0.1, 2].
    struct speech_options
    {
        std::optional<locale> locale;
        std::optional<float> pitch;
        std::optional<float> volume;
        std::optional<float> rate;
    };

    using locales_callback = maui::core::move_only_function<void(const std::vector<locale>&)>;
    // SpeakAsync completion (Task completes when the utterance finishes or is cancelled).
    using speak_callback = maui::core::move_only_function<void()>;

    class i_text_to_speech
    {
    public:
        virtual ~i_text_to_speech() = default;

        // GetLocalesAsync(): the locales the engine supports.
        virtual void get_locales_async(locales_callback on_complete) = 0;
        // SpeakAsync(text, options, cancelToken): validates + serializes + speaks; on_complete runs
        // when the utterance finishes (or is cancelled via the token).
        virtual void speak_async(std::string_view text, const std::optional<speech_options>& options,
                                 maui::core::cancellation_token token, speak_callback on_complete) = 0;

    protected:
        i_text_to_speech() = default;
        i_text_to_speech(const i_text_to_speech&) = default;
        i_text_to_speech(i_text_to_speech&&) = default;
        i_text_to_speech& operator=(const i_text_to_speech&) = default;
        i_text_to_speech& operator=(i_text_to_speech&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (TextToSpeechImplementation), one per backend under
        // src/platform/<backend>/essentials_text_to_speech.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_text_to_speech> make_text_to_speech();

        // TextToSpeech.SplitSpeak(text, max): the Android-partial chunker, internal but unit-tested.
        // Splits text into <= max-length parts, breaking on whitespace/punctuation; ported 1:1.
        [[nodiscard]] std::vector<std::string> split_speak(std::string_view text, std::size_t max);

        // The TextToSpeechImplementation pitch/volume/rate bounds (the internal const set).
        inline constexpr float pitch_max = 2.0F;
        inline constexpr float pitch_min = 0.0F;
        inline constexpr float volume_max = 1.0F;
        inline constexpr float volume_min = 0.0F;
        inline constexpr float rate_max = 2.0F;
        inline constexpr float rate_min = 0.1F;
    } // namespace detail

    // The static facade over text_to_speech::default_() (C# TextToSpeech.Default).
    class text_to_speech final
    {
    public:
        text_to_speech() = delete;

        static void get_locales_async(locales_callback on_complete)
        {
            default_().get_locales_async(std::move(on_complete));
        }
        static void speak_async(std::string_view text, maui::core::cancellation_token token = {},
                                speak_callback on_complete = {})
        {
            default_().speak_async(text, std::nullopt, token, std::move(on_complete));
        }
        static void speak_async(std::string_view text, const std::optional<speech_options>& options,
                                maui::core::cancellation_token token = {}, speak_callback on_complete = {})
        {
            default_().speak_async(text, options, token, std::move(on_complete));
        }

        // TextToSpeech.Default (lazy platform default) + SetDefault (the C# internal test seam made
        // public; nullptr resets to the lazy platform default).
        [[nodiscard]] static i_text_to_speech& default_();
        static void set_default(std::shared_ptr<i_text_to_speech> implementation);
    };
} // namespace maui::media
