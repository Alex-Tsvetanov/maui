// text_to_speech - Apple (AppKit / macOS) platform partial. Ported from TextToSpeech.macos.cs:
// GetLocalesAsync enumerates NSSpeechSynthesizer.AvailableVoices (VoiceLanguage / VoiceName /
// VoiceIdentifier); SpeakAsync drives a lazily-created NSSpeechSynthesizer (Volume / Voice / Rate
// from the options) and completes on DidFinishSpeaking. The shared validation +
// serialization live in detail::text_to_speech_base.
//
// Async/cancellation deviation from C#: the port's cancellation_token is poll-only (no Register
// callback), so a token already cancelled at call time stops immediately and completes; the C#
// mid-speak Register(TryCancel) has no analog without a registration hook. The completion callback
// runs on the synthesizer-delegate thread (the main run loop for NSSpeechSynthesizer).

#import <AppKit/AppKit.h>

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

// The NSSpeechSynthesizer delegate trampoline: forwards DidFinishSpeaking to a stored C++ callback.
@interface MauiSpeechSynthesizerDelegate : NSObject <NSSpeechSynthesizerDelegate>
@property(nonatomic, copy) void (^onFinished)(BOOL);
@end

@implementation MauiSpeechSynthesizerDelegate
- (void)speechSynthesizer:(NSSpeechSynthesizer*)sender didFinishSpeaking:(BOOL)finishedSpeaking
{
    (void)sender;
    if (self.onFinished != nil)
    {
        self.onFinished(finishedSpeaking);
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

        float normalize_volume(float volume)
        {
            if (volume > 1.0F)
            {
                return 1.0F;
            }
            if (volume < 0.0F)
            {
                return 0.0F;
            }
            return volume;
        }

        class apple_text_to_speech final : public detail::text_to_speech_base
        {
        protected:
            void platform_get_locales_async(locales_callback on_complete) override
            {
                std::vector<locale> locales;
                for (NSString* const voice in [NSSpeechSynthesizer availableVoices])
                {
                    NSDictionary* const attributes = [NSSpeechSynthesizer attributesForVoice:voice];
                    locale result;
                    result.language = to_std_string(attributes[NSVoiceLanguage]);
                    result.name = to_std_string(attributes[NSVoiceName]);
                    result.id = to_std_string(attributes[NSVoiceIdentifier]);
                    locales.push_back(std::move(result));
                }
                on_complete(locales);
            }

            void platform_speak_async(std::string_view text, const std::optional<speech_options>& options,
                                      maui::core::cancellation_token token, speak_callback on_complete) override
            {
                if (synthesizer_ == nil)
                {
                    synthesizer_ = [[NSSpeechSynthesizer alloc] initWithVoice:nil];
                    delegate_ = [[MauiSpeechSynthesizerDelegate alloc] init];
                    synthesizer_.delegate = delegate_;
                }

                if (options.has_value())
                {
                    if (options->volume.has_value())
                    {
                        synthesizer_.volume = normalize_volume(*options->volume);
                    }
                    if (options->locale.has_value() && !options->locale->id.empty())
                    {
                        [synthesizer_ setVoice:[NSString stringWithUTF8String:options->locale->id.c_str()]];
                    }
                    if (options->rate.has_value())
                    {
                        synthesizer_.rate = *options->rate;
                    }
                }

                // A token already cancelled at call time: stop + complete (the poll-only analog of
                // the C# Register(TryCancel)).
                if (token.is_cancelled())
                {
                    [synthesizer_ stopSpeaking];
                    if (on_complete)
                    {
                        on_complete();
                    }
                    return;
                }

                const auto shared_complete = std::make_shared<speak_callback>(std::move(on_complete));
                delegate_.onFinished = ^(BOOL) {
                  if (*shared_complete)
                  {
                      (*shared_complete)();
                  }
                };

                NSString* const ns_text = [[NSString alloc] initWithBytes:text.data()
                                                                   length:text.size()
                                                                 encoding:NSUTF8StringEncoding];
                [synthesizer_ startSpeakingString:ns_text];
            }

        private:
            NSSpeechSynthesizer* synthesizer_ = nil;
            MauiSpeechSynthesizerDelegate* delegate_ = nil;
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_text_to_speech> make_text_to_speech()
        {
            return std::make_shared<apple_text_to_speech>();
        }
    } // namespace detail
} // namespace maui::media
