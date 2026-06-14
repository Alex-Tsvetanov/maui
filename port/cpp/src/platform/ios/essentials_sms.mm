// sms - iOS (UIKit) platform partial. Ported from Sms.ios.cs: IsComposeSupported =
// MFMessageComposeViewController.canSendText; ComposeAsync presents an MFMessageComposeViewController
// (body + recipients populated) on the current view controller. UI-SEAM NOTE (sms.hpp): the compose
// controller needs a current view controller, which the spawned gtest process lacks, and the simulator
// reports canSendText = false (no Messages account) - so the on-simulator suite asserts the
// unsupported gate (compose throws feature_not_supported); the compose UI is exercised only inside a
// real app. The gate lives here, matching the C# SmsImplementation. Compiled as Objective-C++ with ARC
// for the ios backend.

#import <Foundation/Foundation.h>
#import <MessageUI/MessageUI.h>
#import <UIKit/UIKit.h>

#include <memory>
#include <string>
#include <vector>

#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/sms.hpp"

#include "src/platform/apple_shared/essentials_url.hpp"

namespace maui::application_model::communication
{
    namespace
    {
        using maui::platform::apple_shared::to_ns_string;

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

        class ios_sms final : public i_sms
        {
        public:
            [[nodiscard]] bool is_compose_supported() const override
            {
                return [MFMessageComposeViewController canSendText] == YES;
            }

            void compose_async(const sms_message& message, sms_completion_callback on_complete) override
            {
                if (!is_compose_supported())
                {
                    throw maui::application_model::feature_not_supported();
                }

                UIViewController* const host = current_view_controller();
                if (host != nil)
                {
                    MFMessageComposeViewController* const controller = [[MFMessageComposeViewController alloc] init];
                    if (!message.body().empty())
                    {
                        controller.body = to_ns_string(message.body());
                    }
                    NSMutableArray<NSString*>* const recipients =
                        [NSMutableArray arrayWithCapacity:message.recipients().size()];
                    for (const std::string& recipient : message.recipients())
                    {
                        [recipients addObject:to_ns_string(recipient)];
                    }
                    controller.recipients = recipients;
                    [host presentViewController:controller animated:YES completion:nil];
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
        std::shared_ptr<i_sms> make_sms()
        {
            return std::make_shared<ios_sms>();
        }
    } // namespace detail
} // namespace maui::application_model::communication
