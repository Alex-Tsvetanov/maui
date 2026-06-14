// The cross-platform half of the email facade: the lazily-created implementation slot behind
// Email.Default / SetDefault, plus the RFC2368 `mailto:` builder GetMailToUri ported 1:1 from
// EmailImplementation.shared (Email.shared.cs). The implementation itself is the per-backend partial
// (src/platform/<backend>/essentials_email.{cpp,mm}) via detail::make_email().

#include "maui/essentials/email.hpp"

#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace maui::application_model::communication
{
    namespace
    {
        std::shared_ptr<i_email>& email_storage()
        {
            static std::shared_ptr<i_email> storage;
            return storage;
        }

        // Uri.EscapeDataString: leave the RFC 3986 unreserved set (ALPHA / DIGIT / "-" / "." / "_" /
        // "~") unescaped; percent-encode every other byte as %XX with UPPERCASE hex (matching .NET).
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

        // string.IsNullOrWhiteSpace: empty or all-whitespace.
        [[nodiscard]] bool is_blank(std::string_view value)
        {
            return value.find_first_not_of(" \t\n\v\f\r") == std::string_view::npos;
        }

        // Recipients(addresses) => string.Join(",", addresses.Select(Uri.EscapeDataString)).
        std::string recipients(const std::vector<std::string>& addresses)
        {
            std::string joined;
            for (std::size_t index = 0; index < addresses.size(); ++index)
            {
                if (index != 0)
                {
                    joined.push_back(',');
                }
                joined += escape_data_string(addresses[index]);
            }
            return joined;
        }
    } // namespace

    i_email& email::default_()
    {
        auto& storage = email_storage();
        if (storage == nullptr)
        {
            storage = detail::make_email();
        }
        return *storage;
    }

    void email::set_default(std::shared_ptr<i_email> implementation)
    {
        email_storage() = std::move(implementation);
    }

    namespace detail
    {
        std::string get_mail_to_uri(const email_message& message)
        {
            // "mailto:?" + string.Join("&", Parameters(message)) — Parameters() yields, in this
            // order, the non-empty to/cc/bcc (recipients) then subject/body (escaped), skipping any
            // blank/empty field. Attachments and BodyFormat are intentionally never emitted.
            std::vector<std::string> parameters;
            if (!message.to().empty())
            {
                parameters.push_back("to=" + recipients(message.to()));
            }
            if (!message.cc().empty())
            {
                parameters.push_back("cc=" + recipients(message.cc()));
            }
            if (!message.bcc().empty())
            {
                parameters.push_back("bcc=" + recipients(message.bcc()));
            }
            if (!is_blank(message.subject()))
            {
                parameters.push_back("subject=" + escape_data_string(message.subject()));
            }
            if (!is_blank(message.body()))
            {
                parameters.push_back("body=" + escape_data_string(message.body()));
            }

            std::string uri = "mailto:?";
            for (std::size_t index = 0; index < parameters.size(); ++index)
            {
                if (index != 0)
                {
                    uri.push_back('&');
                }
                uri += parameters[index];
            }
            return uri;
        }
    } // namespace detail
} // namespace maui::application_model::communication
