// launcher - Apple (AppKit / macOS) platform partial. Ported 1:1 from Launcher.macos.cs:
// can-open asks NSWorkspace for an application registered for the URL
// (URLForApplicationToOpenURL != nil), open is NSWorkspace openURL:, and try-open opens only
// when an application exists (no attempt otherwise). All three are synchronous on macOS (the C#
// Task.FromResult wrappers), so the callbacks complete inline. The URI goes through
// apple_shared::get_native_url - the WebUtils.GetNativeUrl OriginalString->AbsoluteUri fallback,
// so a URI NSURL rejects raw but accepts normalized (e.g. a literal space) still opens; only a URI
// neither form can parse (nil) reports false. Compiled as Objective-C++ with ARC for the apple backend.

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#include <memory>
#include <string_view>
#include <utility>

#include "maui/essentials/launcher.hpp"

#include "src/platform/apple_shared/essentials_url.hpp"

namespace maui::application_model
{
    namespace
    {
        using maui::platform::apple_shared::get_native_url;

        class apple_launcher final : public i_launcher
        {
        public:
            void can_open_async(std::string_view uri, launch_callback on_complete) override
            {
                NSURL* const url = get_native_url(uri);
                on_complete(url != nil && [[NSWorkspace sharedWorkspace] URLForApplicationToOpenURL:url] != nil);
            }

            void open_async(std::string_view uri, launch_callback on_complete) override
            {
                NSURL* const url = get_native_url(uri);
                on_complete(url != nil && [[NSWorkspace sharedWorkspace] openURL:url] == YES);
            }

            void try_open_async(std::string_view uri, launch_callback on_complete) override
            {
                NSURL* const url = get_native_url(uri);
                const bool can_open =
                    url != nil && [[NSWorkspace sharedWorkspace] URLForApplicationToOpenURL:url] != nil;
                if (!can_open)
                {
                    on_complete(false);
                    return;
                }
                on_complete([[NSWorkspace sharedWorkspace] openURL:url] == YES);
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_launcher> make_launcher()
        {
            return std::make_shared<apple_launcher>();
        }
    } // namespace detail
} // namespace maui::application_model
