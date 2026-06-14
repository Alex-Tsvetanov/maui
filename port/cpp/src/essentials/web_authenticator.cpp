// The cross-platform half of the web_authenticator facade: the lazily-created implementation slot
// behind WebAuthenticator.Default / SetDefault, plus the WebAuthenticatorResult query/fragment parser
// (WebUtils.ParseQueryString + Uri.UnescapeDataString) and the expires_in timestamp math. The
// implementation itself is the per-backend partial (src/platform/<backend>/
// essentials_web_authenticator.{cpp,mm}) via detail::make_web_authenticator().

#include "maui/essentials/web_authenticator.hpp"

#include <charconv>
#include <chrono>
#include <cstddef>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

namespace maui::authentication
{
    namespace
    {
        std::shared_ptr<i_web_authenticator>& web_authenticator_storage()
        {
            static std::shared_ptr<i_web_authenticator> storage;
            return storage;
        }

        // Uri.UnescapeDataString: turn %XX escapes back into bytes (leaving everything else as-is). A
        // malformed/short escape is passed through verbatim, matching .NET's lenient unescape.
        std::string unescape_data_string(std::string_view value)
        {
            const auto hex_value = [](char character) -> int {
                if (character >= '0' && character <= '9')
                {
                    return character - '0';
                }
                if (character >= 'A' && character <= 'F')
                {
                    return character - 'A' + 10;
                }
                if (character >= 'a' && character <= 'f')
                {
                    return character - 'a' + 10;
                }
                return -1;
            };

            std::string result;
            result.reserve(value.size());
            for (std::size_t index = 0; index < value.size(); ++index)
            {
                if (value[index] == '%' && index + 2 < value.size())
                {
                    const int high = hex_value(value[index + 1]);
                    const int low = hex_value(value[index + 2]);
                    if (high >= 0 && low >= 0)
                    {
                        result.push_back(static_cast<char>((high << 4) | low));
                        index += 2;
                        continue;
                    }
                }
                result.push_back(value[index]);
            }
            return result;
        }

        // WebUtils.UnpackParameters: split a query/fragment body on '&', each segment on the first
        // '=', map '+' -> ' ' in the value, unescape both name and value, store name -> value.
        void unpack_parameters(std::string_view query, std::map<std::string, std::string>& parameters)
        {
            while (!query.empty())
            {
                std::string_view segment;
                const std::size_t delimiter = query.find('&');
                if (delimiter != std::string_view::npos)
                {
                    segment = query.substr(0, delimiter);
                    query = query.substr(delimiter + 1);
                }
                else
                {
                    segment = query;
                    query = {};
                }

                if (segment.empty())
                {
                    continue;
                }

                std::string name;
                std::string value;
                const std::size_t equals = segment.find('=');
                if (equals != std::string_view::npos)
                {
                    name = segment.substr(0, equals);
                    std::string raw_value(segment.substr(equals + 1));
                    for (char& character : raw_value)
                    {
                        if (character == '+')
                        {
                            character = ' ';
                        }
                    }
                    value = unescape_data_string(raw_value);
                }
                else
                {
                    name = segment;
                }

                parameters[unescape_data_string(name)] = std::move(value);
            }
        }

        // int.TryParse: a base-10 integer (optional leading sign), nothing else. std::nullopt on
        // failure (empty / non-digit / overflow). std::from_chars is exception-free (no try/catch) and
        // rejects a leading '+' or surrounding whitespace exactly like int.TryParse's default styles.
        std::optional<int> try_parse_int(const std::string& value)
        {
            const char* const first = value.data();
            const char* const last = std::next(first, static_cast<std::ptrdiff_t>(value.size()));
            int parsed = 0;
            const auto [ptr, error] = std::from_chars(first, last, parsed);
            if (error != std::errc{} || ptr != last)
            {
                return std::nullopt;
            }
            return parsed;
        }
    } // namespace

    web_authenticator_result::web_authenticator_result(std::string_view callback_uri)
        : callback_uri_(callback_uri), timestamp_(std::chrono::system_clock::now())
    {
        // WebUtils.ParseQueryString: query (after the first '?') THEN fragment (after the first '#');
        // a key in both is overwritten by the fragment, matching the C# order.
        const std::size_t query_start = callback_uri_.find('?');
        const std::size_t fragment_start = callback_uri_.find('#');

        if (query_start != std::string::npos)
        {
            const std::size_t query_end = fragment_start != std::string::npos && fragment_start > query_start
                                              ? fragment_start
                                              : std::string::npos;
            const std::string_view query =
                std::string_view(callback_uri_)
                    .substr(query_start + 1,
                            query_end == std::string::npos ? std::string::npos : query_end - (query_start + 1));
            unpack_parameters(query, properties_);
        }
        if (fragment_start != std::string::npos)
        {
            unpack_parameters(std::string_view(callback_uri_).substr(fragment_start + 1), properties_);
        }
    }

    std::optional<web_authenticator_result::time_point> web_authenticator_result::expiry_from(
        const std::string& key) const
    {
        const auto entry = properties_.find(key);
        if (entry == properties_.end())
        {
            return std::nullopt;
        }
        const std::optional<int> seconds = try_parse_int(entry->second);
        if (!seconds.has_value())
        {
            return std::nullopt;
        }
        return timestamp_ + std::chrono::seconds(*seconds);
    }

    i_web_authenticator& web_authenticator::default_()
    {
        auto& storage = web_authenticator_storage();
        if (storage == nullptr)
        {
            storage = detail::make_web_authenticator();
        }
        return *storage;
    }

    void web_authenticator::set_default(std::shared_ptr<i_web_authenticator> implementation)
    {
        web_authenticator_storage() = std::move(implementation);
    }
} // namespace maui::authentication
