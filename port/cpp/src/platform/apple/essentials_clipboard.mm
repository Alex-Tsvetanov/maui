// clipboard - Apple (AppKit / macOS) platform partial. Ported 1:1 from Clipboard.macos.cs:
// NSPasteboard.GeneralPasteboard holds a single string type; SetText declares + clears + writes the
// string, HasText / GetText read the string back. macOS has NO clipboard-change notification, so the
// listener hooks throw feature_not_supported, exactly like the C# StartClipboardListeners /
// StopClipboardListeners (which throw NotSupportedOrImplementedException). The Task results complete
// inline. Compiled as Objective-C++ with ARC for the apple backend.

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "maui/essentials/clipboard.hpp"
#include "maui/essentials/feature_not_supported.hpp"

#include "src/platform/apple_shared/essentials_url.hpp"

namespace maui::application_model
{
    namespace
    {
        using maui::platform::apple_shared::to_ns_string;
        using maui::platform::apple_shared::to_std_string;

        // GetPasteboardText: the general pasteboard's string (nil when there is none).
        std::optional<std::string> pasteboard_text()
        {
            NSString* const text = [[NSPasteboard generalPasteboard] stringForType:NSPasteboardTypeString];
            if (text == nil)
            {
                return std::nullopt;
            }
            return to_std_string(text);
        }

        class apple_clipboard final : public detail::clipboard_base
        {
        public:
            [[nodiscard]] bool has_text() const override
            {
                // !string.IsNullOrEmpty(GetPasteboardText()).
                const std::optional<std::string> text = pasteboard_text();
                return text.has_value() && !text->empty();
            }

            void set_text_async(std::string_view text, clipboard_completion_callback on_complete) override
            {
                NSPasteboard* const pasteboard = [NSPasteboard generalPasteboard];
                [pasteboard declareTypes:@[ NSPasteboardTypeString ] owner:nil];
                [pasteboard clearContents];
                [pasteboard setString:to_ns_string(text) forType:NSPasteboardTypeString];
                if (on_complete)
                {
                    on_complete();
                }
            }

            void get_text_async(clipboard_text_callback on_complete) override
            {
                on_complete(pasteboard_text());
            }

        protected:
            // macOS has no pasteboard-change notification (the C# partials throw).
            void start_clipboard_listeners() override
            {
                throw feature_not_supported();
            }
            void stop_clipboard_listeners() override
            {
                throw feature_not_supported();
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_clipboard> make_clipboard()
        {
            return std::make_shared<apple_clipboard>();
        }
    } // namespace detail
} // namespace maui::application_model
