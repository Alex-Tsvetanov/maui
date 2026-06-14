// phone_dialer - iOS (UIKit) platform partial. Ported 1:1 from PhoneDialer.ios.cs: IsSupported =
// UIApplication can open a "tel:<10 zeros>" URL (false on the simulator, which has no phone, so the
// on-simulator suite asserts the unsupported gate); Open validates (the shared ValidateOpen gate) then
// opens "tel:<number>" through the launcher. Compiled as Objective-C++ with ARC for the ios backend.

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include <memory>
#include <string>
#include <string_view>

#include "maui/essentials/launcher.hpp"
#include "maui/essentials/phone_dialer.hpp"

namespace maui::application_model::communication
{
    namespace
    {
        class ios_phone_dialer final : public i_phone_dialer
        {
        public:
            [[nodiscard]] bool is_supported() const override
            {
                NSURL* const probe = [NSURL URLWithString:@"tel:0000000000"];
                return probe != nil && [[UIApplication sharedApplication] canOpenURL:probe] == YES;
            }

            void open(std::string_view number) override
            {
                detail::validate_phone_dialer_open(number, is_supported());
                const std::string tel = "tel:" + std::string(number);
                // The C# ignores the returned Task; the port supplies a no-op completion (the launcher
                // invokes its callback unconditionally).
                maui::application_model::launcher::default_().open_async(tel, [](bool) {});
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_phone_dialer> make_phone_dialer()
        {
            return std::make_shared<ios_phone_dialer>();
        }
    } // namespace detail
} // namespace maui::application_model::communication
