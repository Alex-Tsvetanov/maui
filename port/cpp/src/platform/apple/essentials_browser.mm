// browser - Apple (AppKit / macOS) platform partial. Ported 1:1 from Browser.macos.cs: macOS has
// no in-app browser surface, so EVERY launch mode opens the default browser through NSWorkspace
// openURL: and the options (colors / title mode / flags) are ignored, exactly like the C#
// partial. Synchronous - the callback completes inline. Compiled as Objective-C++ with ARC for
// the apple backend.

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#include <memory>
#include <string_view>
#include <utility>

#include "maui/essentials/browser.hpp"

namespace maui::application_model
{
    namespace
    {
        class apple_browser final : public i_browser
        {
        public:
            void open_async(std::string_view uri, const browser_launch_options& /*options*/,
                            launch_callback on_complete) override
            {
                NSString* const text = [[NSString alloc] initWithBytes:uri.data()
                                                                length:uri.size()
                                                              encoding:NSUTF8StringEncoding];
                NSURL* const url = [NSURL URLWithString:text];
                on_complete(url != nil && [[NSWorkspace sharedWorkspace] openURL:url] == YES);
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_browser> make_browser()
        {
            return std::make_shared<apple_browser>();
        }
    } // namespace detail
} // namespace maui::application_model
