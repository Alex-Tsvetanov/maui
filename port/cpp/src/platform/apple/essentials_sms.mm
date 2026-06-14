// sms - Apple (AppKit / macOS) platform partial. Ported 1:1 from Sms.macos.cs: IsComposeSupported =
// NSWorkspace can resolve an app for "sms:"; PlatformComposeAsync builds an
// "sms:/open?addresses=<escaped,joined>&body=<escaped>" URL and opens it through NSWorkspace. The
// ComposeAsync gate (throw feature_not_supported when unsupported, plus the null-message / empty-
// recipients coalescing) lives in this i_sms::compose_async, matching the C# SmsImplementation.
// Compiled as Objective-C++ with ARC for the apple backend.

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/sms.hpp"

#include "src/platform/apple_shared/essentials_url.hpp"

namespace maui::application_model::communication
{
    namespace
    {
        using maui::platform::apple_shared::to_ns_url;

        // Uri.EscapeDataString (the same RFC 3986 unreserved set the email builder uses).
        std::string escape_data_string(std::string_view value)
        {
            static constexpr std::array<char, 16> hex_digits{'0', '1', '2', '3', '4', '5', '6', '7',
                                                             '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
            std::string result;
            result.reserve(value.size());
            for (const char character : value)
            {
                const auto byte = static_cast<unsigned char>(character);
                const bool unreserved = (byte >= 'A' && byte <= 'Z') || (byte >= 'a' && byte <= 'z') ||
                                        (byte >= '0' && byte <= '9') || byte == '-' || byte == '.' || byte == '_' ||
                                        byte == '~';
                if (unreserved)
                {
                    result.push_back(character);
                }
                else
                {
                    result.push_back('%');
                    result.push_back(hex_digits.at(byte >> 4U));
                    result.push_back(hex_digits.at(byte & 0x0FU));
                }
            }
            return result;
        }

        class apple_sms final : public i_sms
        {
        public:
            [[nodiscard]] bool is_compose_supported() const override
            {
                NSURL* const probe = [NSURL URLWithString:@"sms:"];
                return probe != nil && [[NSWorkspace sharedWorkspace] URLForApplicationToOpenURL:probe] != nil;
            }

            void compose_async(const sms_message& message, sms_completion_callback on_complete) override
            {
                if (!is_compose_supported())
                {
                    throw maui::application_model::feature_not_supported();
                }

                std::string recipients;
                for (std::size_t index = 0; index < message.recipients().size(); ++index)
                {
                    if (index != 0)
                    {
                        recipients.push_back(',');
                    }
                    recipients += escape_data_string(message.recipients()[index]);
                }

                std::string uri = "sms:/open?addresses=" + recipients;
                if (!message.body().empty())
                {
                    uri += "&body=" + escape_data_string(message.body());
                }

                NSURL* const url = to_ns_url(uri);
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
        std::shared_ptr<i_sms> make_sms()
        {
            return std::make_shared<apple_sms>();
        }
    } // namespace detail
} // namespace maui::application_model::communication
