// email - iOS (UIKit) platform partial. Ported from Email.ios.cs: IsComposeSupported =
// MFMailComposeViewController.canSendMail OR UIApplication can open "mailto:". ComposeAsync presents an
// MFMailComposeViewController when mail can be sent (subject/body/recipients/attachments populated),
// else launches the RFC2368 GetMailToUri through the launcher. UI-SEAM NOTE (email.hpp): presenting
// the compose controller needs a current view controller, which the spawned gtest process lacks - so
// in the test process compose falls to the mailto launch (itself a no-op without a UIApplication that
// can open it); the compose UI is exercised only inside a real app. The ComposeAsync gate (throw
// feature_not_supported when unsupported) lives here, matching the C# EmailImplementation. Compiled as
// Objective-C++ with ARC for the ios backend.

#import <Foundation/Foundation.h>
#import <MessageUI/MessageUI.h>
#import <UIKit/UIKit.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/essentials/email.hpp"
#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/launcher.hpp"

#include "src/platform/apple_shared/essentials_url.hpp"

namespace maui::application_model::communication
{
    namespace
    {
        using maui::platform::apple_shared::to_ns_string;

        // The current key-window root view controller (nil in the spawned test process).
        UIViewController* current_view_controller()
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
                    return window.rootViewController;
                }
            }
            return nil;
        }

        bool can_open_mailto()
        {
            NSURL* const probe = [NSURL URLWithString:@"mailto:"];
            return probe != nil && [[UIApplication sharedApplication] canOpenURL:probe] == YES;
        }

        class ios_email final : public i_email
        {
        public:
            [[nodiscard]] bool is_compose_supported() const override
            {
                return [MFMailComposeViewController canSendMail] == YES || can_open_mailto();
            }

            void compose_async(const email_message& message, email_completion_callback on_complete) override
            {
                if (!is_compose_supported())
                {
                    throw maui::application_model::feature_not_supported();
                }

                UIViewController* const host = current_view_controller();
                if ([MFMailComposeViewController canSendMail] == YES && host != nil)
                {
                    present_compose(message, host);
                }
                else
                {
                    // ComposeWithUrl: launch the mailto: URL (the no-VC / no-mail-account fallback).
                    maui::application_model::launcher::default_().open_async(detail::get_mail_to_uri(message),
                                                                             [](bool) {});
                }
                if (on_complete)
                {
                    on_complete();
                }
            }

        private:
            static void present_compose(const email_message& message, UIViewController* host)
            {
                MFMailComposeViewController* const controller = [[MFMailComposeViewController alloc] init];
                if (!message.body().empty())
                {
                    [controller setMessageBody:to_ns_string(message.body())
                                        isHTML:message.body_format() == email_body_format::html];
                }
                if (!message.subject().empty())
                {
                    [controller setSubject:to_ns_string(message.subject())];
                }
                [controller setToRecipients:to_ns_array(message.to())];
                [controller setCcRecipients:to_ns_array(message.cc())];
                [controller setBccRecipients:to_ns_array(message.bcc())];
                [host presentViewController:controller animated:YES completion:nil];
            }

            static NSArray<NSString*>* to_ns_array(const std::vector<std::string>& values)
            {
                NSMutableArray<NSString*>* const array = [NSMutableArray arrayWithCapacity:values.size()];
                for (const std::string& value : values)
                {
                    [array addObject:to_ns_string(value)];
                }
                return array;
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_email> make_email()
        {
            return std::make_shared<ios_email>();
        }
    } // namespace detail
} // namespace maui::application_model::communication
