// phone_dialer - Apple (AppKit / macOS) platform partial. Ported 1:1 from PhoneDialer.macos.cs:
// IsSupported = NSWorkspace can resolve an app for "tel:0000000000"; Open validates (the shared
// ValidateOpen gate: blank -> invalid_argument, unsupported -> feature_not_supported) then opens
// "tel:<number>" through NSWorkspace. Compiled as Objective-C++ with ARC for the apple backend.

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#include <memory>
#include <string>
#include <string_view>

#include "maui/essentials/phone_dialer.hpp"

#include "src/platform/apple_shared/essentials_url.hpp"

namespace maui::application_model::communication
{
    namespace
    {
        using maui::platform::apple_shared::to_ns_url;

        class apple_phone_dialer final : public i_phone_dialer
        {
        public:
            [[nodiscard]] bool is_supported() const override
            {
                NSURL* const probe = [NSURL URLWithString:@"tel:0000000000"];
                return probe != nil && [[NSWorkspace sharedWorkspace] URLForApplicationToOpenURL:probe] != nil;
            }

            void open(std::string_view number) override
            {
                detail::validate_phone_dialer_open(number, is_supported());

                NSURL* const url = to_ns_url("tel:" + std::string(number));
                if (url != nil)
                {
                    [[NSWorkspace sharedWorkspace] openURL:url];
                }
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_phone_dialer> make_phone_dialer()
        {
            return std::make_shared<apple_phone_dialer>();
        }
    } // namespace detail
} // namespace maui::application_model::communication
