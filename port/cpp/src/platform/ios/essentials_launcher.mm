// launcher - iOS (UIKit) platform partial. Ported 1:1 from Launcher.ios.tvos.cs: can-open is
// UIApplication.canOpenURL (synchronous; schemes must be declared in LSApplicationQueriesSchemes
// to query third-party apps), open is openURL:options:completionHandler: (genuinely async - the
// completion arrives on the main queue), and try-open opens only when canOpenURL says yes. The
// spawned simulator test process has no UIApplication instance ([UIApplication sharedApplication]
// is nil), so every query completes false there - asserted by the on-simulator suite; a real app
// exercises the true paths. The URI goes through apple_shared::get_native_url - the
// WebUtils.GetNativeUrl OriginalString->AbsoluteUri fallback, so a URI NSURL rejects raw but accepts
// normalized still parses; only a URI neither form can parse (nil) reports false.
// Compiled as Objective-C++ with ARC for the ios backend.

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

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

        bool can_open_url(NSURL* url)
        {
            UIApplication* const app = [UIApplication sharedApplication];
            return url != nil && app != nil && [app canOpenURL:url] == YES;
        }

        void open_url(NSURL* url, launch_callback on_complete)
        {
            UIApplication* const app = [UIApplication sharedApplication];
            if (url == nil || app == nil)
            {
                on_complete(false);
                return;
            }
            const auto shared_callback = std::make_shared<launch_callback>(std::move(on_complete));
            [app openURL:url
                options:@{}
                completionHandler:^(BOOL success) {
                  (*shared_callback)(success == YES);
                }];
        }

        class ios_launcher final : public i_launcher
        {
        public:
            void can_open_async(std::string_view uri, launch_callback on_complete) override
            {
                on_complete(can_open_url(get_native_url(uri)));
            }

            void open_async(std::string_view uri, launch_callback on_complete) override
            {
                open_url(get_native_url(uri), std::move(on_complete));
            }

            void try_open_async(std::string_view uri, launch_callback on_complete) override
            {
                NSURL* const url = get_native_url(uri);
                if (!can_open_url(url))
                {
                    on_complete(false);
                    return;
                }
                open_url(url, std::move(on_complete));
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_launcher> make_launcher()
        {
            return std::make_shared<ios_launcher>();
        }
    } // namespace detail
} // namespace maui::application_model
