// semantic_screen_reader - iOS (UIKit) platform partial. Ported 1:1 from SemanticScreenReader.ios.cs:
// Announce posts a UIAccessibility Announcement notification, but ONLY when VoiceOver is running; when
// VoiceOver is off the announce is a silent no-op (NOT a throw). On the simulator VoiceOver is off, so
// announce is a no-op there - the on-simulator suite asserts it does not throw. Compiled as
// Objective-C++ with ARC for the ios backend.

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include <memory>
#include <string_view>

#include "maui/essentials/semantic_screen_reader.hpp"

namespace maui::accessibility
{
    namespace
    {
        class ios_semantic_screen_reader final : public i_semantic_screen_reader
        {
        public:
            void announce(std::string_view text) override
            {
                if (!UIAccessibilityIsVoiceOverRunning())
                {
                    return;
                }
                NSString* const message = [[NSString alloc] initWithBytes:text.data()
                                                                   length:text.size()
                                                                 encoding:NSUTF8StringEncoding];
                UIAccessibilityPostNotification(UIAccessibilityAnnouncementNotification, message);
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_semantic_screen_reader> make_semantic_screen_reader()
        {
            return std::make_shared<ios_semantic_screen_reader>();
        }
    } // namespace detail
} // namespace maui::accessibility
