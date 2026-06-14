// email - Apple (AppKit / macOS) platform partial. Ported 1:1 from Email.macos.cs: IsComposeSupported
// = NSWorkspace can resolve an app for "mailto:"; PlatformComposeAsync opens the RFC2368 GetMailToUri
// through NSWorkspace (macOS has no in-app compose UI - it hands off to the default mail client). The
// shared IsComposeSupported gate (compose throws feature_not_supported when unsupported) lives in
// i_email's facade path; here compose_async is the PlatformComposeAsync half and runs only after the
// gate passes, so it re-checks support to keep the contract identical to the facade. Compiled as
// Objective-C++ with ARC for the apple backend.

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#include <memory>
#include <string>
#include <utility>

#include "maui/essentials/email.hpp"
#include "maui/essentials/feature_not_supported.hpp"

#include "src/platform/apple_shared/essentials_url.hpp"

namespace maui::application_model::communication
{
    namespace
    {
        using maui::platform::apple_shared::to_ns_url;

        class apple_email final : public i_email
        {
        public:
            [[nodiscard]] bool is_compose_supported() const override
            {
                NSURL* const probe = [NSURL URLWithString:@"mailto:"];
                return probe != nil && [[NSWorkspace sharedWorkspace] URLForApplicationToOpenURL:probe] != nil;
            }

            void compose_async(const email_message& message, email_completion_callback on_complete) override
            {
                // The C# Email.ComposeAsync gate: throw FeatureNotSupportedException before composing.
                if (!is_compose_supported())
                {
                    throw maui::application_model::feature_not_supported();
                }
                NSURL* const url = to_ns_url(detail::get_mail_to_uri(message));
                if (url != nil)
                {
                    [[NSWorkspace sharedWorkspace] openURL:url];
                }
                if (on_complete)
                {
                    on_complete();
                }
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_email> make_email()
        {
            return std::make_shared<apple_email>();
        }
    } // namespace detail
} // namespace maui::application_model::communication
