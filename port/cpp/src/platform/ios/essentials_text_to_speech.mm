// text_to_speech - iOS (UIKit) platform partial. Ported from TextToSpeech.ios.tvos.watchos.cs:
// GetLocalesAsync enumerates AVSpeechSynthesisVoice.GetSpeechVoices (Language / Name / Identifier);
// SpeakAsync builds an AVSpeechUtterance (Voice by identifier else language else current language;
// PitchMultiplier / Volume / Rate from the options) and speaks it on a lazily-created
// AVSpeechSynthesizer, completing on DidFinishSpeechUtterance. The shared validation lives in
// detail::text_to_speech_base.
//
// Async/cancellation deviation from C#: the port's cancellation_token is poll-only (no Register
// callback), so a token already cancelled at call time stops immediately and completes; the C#
// mid-speak Register(TryCancel) has no analog without a registration hook.

#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/core/cancellation_token.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/essentials/text_to_speech.hpp"

#include "src/essentials/detail/text_to_speech_base.hpp"

// The AVSpeechSynthesizer delegate trampoline: forwards DidFinishSpeechUtterance to a C++ callback.
@interface MauiAVSpeechDelegate : NSObject <AVSpeechSynthesizerDelegate>
@property(nonatomic, copy) void (^onFinished)(AVSpeechUtterance*);
@end

@implementation MauiAVSpeechDelegate
- (void)speechSynthesizer:(AVSpeechSynthesizer*)synthesizer didFinishSpeechUtterance:(AVSpeechUtterance*)utterance
{
    (void)synthesizer;
    if (self.onFinished != nil)
    {
        self.onFinished(utterance);
    }
}
@end

namespace maui::media
{
    namespace
    {
        std::string to_std_string(NSString* value)
        {
            const char* const utf8 = [value UTF8String];
            return utf8 != nullptr ? std::string(utf8) : std::string();
        }

        NSString* to_ns_string(std::string_view value)
        {
            return [[NSString alloc] initWithBytes:value.data() length:value.size() encoding:NSUTF8StringEncoding];
        }

        // GetSpeechUtterance: voice selection + the pitch/volume/rate knobs.
        AVSpeechUtterance* make_utterance(std::string_view text, const std::optional<speech_options>& options)
        {
            AVSpeechUtterance* const utterance = [AVSpeechUtterance speechUtteranceWithString:to_ns_string(text)];
            if (options.has_value())
            {
                if (options->locale.has_value() && !options->locale->id.empty())
                {
                    utterance.voice = [AVSpeechSynthesisVoice voiceWithIdentifier:to_ns_string(options->locale->id)];
                }
                else if (options->locale.has_value() && !options->locale->language.empty())
                {
                    utterance.voice =
                        [AVSpeechSynthesisVoice voiceWithLanguage:to_ns_string(options->locale->language)];
                }
                else
                {
                    utterance.voice =
                        [AVSpeechSynthesisVoice voiceWithLanguage:[AVSpeechSynthesisVoice currentLanguageCode]];
                }
                if (options->pitch.has_value())
                {
                    utterance.pitchMultiplier = *options->pitch;
                }
                if (options->volume.has_value())
                {
                    utterance.volume = *options->volume;
                }
                if (options->rate.has_value())
                {
                    utterance.rate = *options->rate;
                }
            }
            return utterance;
        }

        class ios_text_to_speech final : public detail::text_to_speech_base
        {
        protected:
            void platform_get_locales_async(locales_callback on_complete) override
            {
                std::vector<locale> locales;
                NSArray<AVSpeechSynthesisVoice*>* const voices = [AVSpeechSynthesisVoice speechVoices];
                for (NSUInteger i = 0; i < voices.count; ++i)
                {
                    AVSpeechSynthesisVoice* const voice = voices[i];
                    locale result;
                    result.language = to_std_string(voice.language);
                    result.name = to_std_string(voice.name);
                    result.id = to_std_string(voice.identifier);
                    locales.push_back(std::move(result));
                }
                on_complete(locales);
            }

            void platform_speak_async(std::string_view text, const std::optional<speech_options>& options,
                                      maui::core::cancellation_token token, speak_callback on_complete) override
            {
                if (synthesizer_ == nil)
                {
                    synthesizer_ = [[AVSpeechSynthesizer alloc] init];
                    delegate_ = [[MauiAVSpeechDelegate alloc] init];
                    synthesizer_.delegate = delegate_;
                }

                AVSpeechUtterance* const utterance = make_utterance(text, options);

                if (token.is_cancelled())
                {
                    [synthesizer_ stopSpeakingAtBoundary:AVSpeechBoundaryImmediate];
                    if (on_complete)
                    {
                        on_complete();
                    }
                    return;
                }

                const auto shared_complete = std::make_shared<speak_callback>(std::move(on_complete));
                AVSpeechUtterance* const expected = utterance;
                delegate_.onFinished = ^(AVSpeechUtterance* finished) {
                  if (finished == expected && *shared_complete)
                  {
                      (*shared_complete)();
                  }
                };
                [synthesizer_ speakUtterance:utterance];
            }

        private:
            AVSpeechSynthesizer* synthesizer_ = nil;
            MauiAVSpeechDelegate* delegate_ = nil;
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_text_to_speech> make_text_to_speech()
        {
            return std::make_shared<ios_text_to_speech>();
        }
    } // namespace detail
} // namespace maui::media
