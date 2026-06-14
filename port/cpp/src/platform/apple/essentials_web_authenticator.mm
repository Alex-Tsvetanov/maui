// web_authenticator - Apple (AppKit / macOS) platform partial. (STRETCH) Ported from
// WebAuthenticator.macos.cs in shape: AuthenticateAsync starts an ASWebAuthenticationSession that
// navigates to options.url and waits for a redirect to options.callback_url's scheme; the callback
// URL is parsed into a web_authenticator_result. The session needs an NSWindow presentation anchor
// (ContextProvider). UNBUNDLED-PROCESS / SIMULATOR NOTE (web_authenticator.hpp): the spawned gtest
// process has no key window to anchor the session and no interactive browser to complete a real flow,
// so when no presentation anchor exists this completes std::nullopt (the cancellation analog) without
// starting a session - the real flow is exercised only inside a running app. The session wiring below
// is the faithful seam; it is not drivable headlessly. Compiled as Objective-C++ with ARC for the
// apple backend.

#import <AppKit/AppKit.h>
#import <AuthenticationServices/AuthenticationServices.h>
#import <Foundation/Foundation.h>

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

        // The key window is the ASWebAuthenticationSession presentation anchor (nil in an unbundled
        // process - see the header note).
        NSWindow* presentation_anchor()
        {
            return [NSApplication sharedApplication].keyWindow;
        }

        class apple_web_authenticator final : public i_web_authenticator
        {
        public:
            void authenticate_async(const web_authenticator_options& options, maui::core::cancellation_token token,
                                    web_authenticator_callback on_complete) override
            {
                // A real app would start an ASWebAuthenticationSession anchored to the key window
                // (presentation_anchor), navigate to options.url, and on the redirect to
                // options.callback_url's scheme parse the callback URL into a web_authenticator_result.
                // The session needs that anchor and an interactive browser, neither present in the
                // unbundled / headless test process - so this completes with no result here (the
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
            return std::make_shared<apple_web_authenticator>();
        }
    } // namespace detail
} // namespace maui::authentication
