// clipboard - iOS (UIKit) platform partial. Ported 1:1 from Clipboard.ios.cs: UIPasteboard.General
// holds the string (SetText assigns .string, GetText reads it, HasText = .hasStrings); the listener
// observes UIPasteboard.changedNotification and raises on_clipboard_content_changed. The shared
// subscriber-count gate (clipboard_base) calls start/stop_clipboard_listeners on the first/last
// subscriber. Compiled as Objective-C++ with ARC for the ios backend.

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "maui/essentials/clipboard.hpp"

namespace maui::application_model
{
    namespace
    {
        class ios_clipboard final : public detail::clipboard_base
        {
        public:
            ~ios_clipboard() override
            {
                remove_observer();
            }

            ios_clipboard() = default;
            ios_clipboard(const ios_clipboard&) = delete;
            ios_clipboard(ios_clipboard&&) = delete;
            ios_clipboard& operator=(const ios_clipboard&) = delete;
            ios_clipboard& operator=(ios_clipboard&&) = delete;

            [[nodiscard]] bool has_text() const override
            {
                return [UIPasteboard generalPasteboard].hasStrings == YES;
            }

            void set_text_async(std::string_view text, clipboard_completion_callback on_complete) override
            {
                [UIPasteboard generalPasteboard].string = [[NSString alloc] initWithBytes:text.data()
                                                                                   length:text.size()
                                                                                 encoding:NSUTF8StringEncoding];
                if (on_complete)
                {
                    on_complete();
                }
            }

            void get_text_async(clipboard_text_callback on_complete) override
            {
                NSString* const text = [UIPasteboard generalPasteboard].string;
                on_complete(text != nil ? std::optional<std::string>(text.UTF8String) : std::nullopt);
            }

        protected:
            void start_clipboard_listeners() override
            {
                observer_ =
                    [[NSNotificationCenter defaultCenter] addObserverForName:UIPasteboardChangedNotification
                                                                      object:nil
                                                                       queue:nil
                                                                  usingBlock:^(NSNotification* /*notification*/) {
                                                                    on_clipboard_content_changed();
                                                                  }];
            }
            void stop_clipboard_listeners() override
            {
                remove_observer();
            }

        private:
            void remove_observer()
            {
                if (observer_ != nil)
                {
                    [[NSNotificationCenter defaultCenter] removeObserver:observer_];
                    observer_ = nil;
                }
            }

            id observer_ = nil;
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_clipboard> make_clipboard()
        {
            return std::make_shared<ios_clipboard>();
        }
    } // namespace detail
} // namespace maui::application_model
