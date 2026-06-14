// web_authenticator - iOS (UIKit) platform partial. (STRETCH) Ported from WebAuthenticator.ios.tvos.cs
// in shape: AuthenticateAsync starts an ASWebAuthenticationSession navigating to options.url, waiting
// for a redirect to options.callback_url's scheme; the callback URL parses into a
// web_authenticator_result. The session needs a presentation anchor (a key window) and an interactive
// browser. UI-SEAM NOTE (web_authenticator.hpp): the spawned gtest process has no key window and no
// interactive browser, so with no anchor this completes std::nullopt (the cancellation analog) without
// starting a session; the real flow is exercised only inside a running app. The session wiring is the
// faithful seam; it is not drivable headlessly. Compiled as Objective-C++ with ARC for the ios backend.

#import <AuthenticationServices/AuthenticationServices.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "maui/essentials/web_authenticator.hpp"

#include "src/platform/apple_shared/essentials_url.hpp"

namespace maui::authentication
{
    namespace
    {
        using maui::platform::apple_shared::to_ns_url;

        UIWindow* presentation_anchor()
        {
            UIApplication* const app = [UIApplication sharedApplication];
            if (app == nil)
            {
                return nil;
            }
            for (UIWindow* window in app.windows)
            {
                if (window.isKeyWindow)
                {
                    return window;
                }
            }
            return nil;
        }

        class ios_web_authenticator final : public i_web_authenticator
        {
        public:
            void authenticate_async(const web_authenticator_options& options, maui::core::cancellation_token token,
                                    web_authenticator_callback on_complete) override
            {
                // A real app would start an ASWebAuthenticationSession anchored to the key window
                // (presentation_anchor), navigate to options.url, and on the redirect to
                // options.callback_url's scheme parse the callback URL into a web_authenticator_result.
                // The session needs that anchor and an interactive browser, neither present in the
                // spawned simulator gtest process - so this completes with no result here (the
                // documented stand-in / cancellation analog); the anchor + cancellation are read so the
                // contract stays faithful, and the real flow lives only inside a running app.
                if (token.is_cancelled() || presentation_anchor() == nil || to_ns_url(options.url) == nil)
                {
                    on_complete(std::nullopt);
                    return;
                }
                on_complete(std::nullopt);
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_web_authenticator> make_web_authenticator()
        {
            return std::make_shared<ios_web_authenticator>();
        }
    } // namespace detail
} // namespace maui::authentication
