// maui::controls::shell_route_parameters — out-of-line bodies. See shell_route_parameters.hpp.

#include "maui/controls/shell/shell_route_parameters.hpp"

#include <any>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

namespace maui::controls
{
    namespace
    {
        // The WebUtility.UrlDecode subset the query strings need: '+' → space and %XX percent-decoding
        // (an invalid escape is kept verbatim, like the lenient C# decoder).
        std::string url_decode(std::string_view value)
        {
            std::string decoded;
            decoded.reserve(value.size());
            for (std::size_t i = 0; i < value.size(); ++i)
            {
                const char c = value[i];
                if (c == '+')
                {
                    decoded.push_back(' ');
                    continue;
                }
                if (c == '%' && i + 2 < value.size())
                {
                    const auto hex = [](char h) -> int {
                        if (h >= '0' && h <= '9')
                        {
                            return h - '0';
                        }
                        if (h >= 'a' && h <= 'f')
                        {
                            return h - 'a' + 10;
                        }
                        if (h >= 'A' && h <= 'F')
                        {
                            return h - 'A' + 10;
                        }
                        return -1;
                    };
                    const int high = hex(value[i + 1]);
                    const int low = hex(value[i + 2]);
                    if (high >= 0 && low >= 0)
                    {
                        decoded.push_back(static_cast<char>((high * 16) + low));
                        i += 2;
                        continue;
                    }
                }
                decoded.push_back(c);
            }
            return decoded;
        }
    } // namespace

    shell_route_parameters::shell_route_parameters(const shell_route_parameters& query, std::string_view prefix)
    {
        for (const auto& [key, value] : query)
        {
            if (!std::string_view{key}.starts_with(prefix))
            {
                continue;
            }
            std::string stripped = key.substr(prefix.size());
            if (stripped.contains('.'))
            {
                continue; // belongs to a deeper route's element
            }
            values_.insert_or_assign(std::move(stripped), value);
        }
    }

    void shell_route_parameters::set_query_string_parameters(std::string_view query)
    {
        if (query.starts_with('?'))
        {
            query = query.substr(1);
        }
        while (!query.empty())
        {
            const std::size_t amp = query.find('&');
            const std::string_view pair = amp == std::string_view::npos ? query : query.substr(0, amp);
            query = amp == std::string_view::npos ? std::string_view{} : query.substr(amp + 1);
            if (pair.empty())
            {
                continue;
            }
            const std::size_t eq = pair.find('=');
            std::string key = url_decode(eq == std::string_view::npos ? pair : pair.substr(0, eq));
            std::string value = eq == std::string_view::npos ? std::string{} : url_decode(pair.substr(eq + 1));
            if (key.empty() || contains(key))
            {
                continue; // existing keys win (C# only adds missing ones)
            }
            values_.insert_or_assign(std::move(key), std::any{std::move(value)});
        }
    }

    const std::any* shell_route_parameters::try_get(std::string_view key) const
    {
        const auto it = values_.find(key);
        return it == values_.end() ? nullptr : &it->second;
    }

    std::string shell_route_parameters::get_string(std::string_view key) const
    {
        const std::any* value = try_get(key);
        if (value == nullptr)
        {
            return {};
        }
        if (const auto* str = std::any_cast<std::string>(value))
        {
            return *str;
        }
        return {};
    }

    void shell_route_parameters::erase(std::string_view key)
    {
        const auto it = values_.find(key);
        if (it != values_.end())
        {
            values_.erase(it);
        }
    }
} // namespace maui::controls
