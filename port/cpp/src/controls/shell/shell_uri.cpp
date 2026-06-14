// maui::controls::shell_uri — out-of-line parsing. See shell_uri.hpp.

#include "maui/controls/shell/shell_uri.hpp"

#include <cctype>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

namespace maui::controls
{
    namespace
    {
        // A System.Uri-style scheme: [alpha][alnum|+|-|.]* followed by ':'. Returns the scheme length
        // (excluding the colon) or 0 when `value` has none.
        std::size_t scheme_length(std::string_view value)
        {
            if (value.empty() || (std::isalpha(static_cast<unsigned char>(value[0])) == 0))
            {
                return 0;
            }
            for (std::size_t i = 1; i < value.size(); ++i)
            {
                const char c = value[i];
                if (c == ':')
                {
                    return i;
                }
                if ((std::isalnum(static_cast<unsigned char>(c)) == 0) && c != '+' && c != '-' && c != '.')
                {
                    return 0;
                }
            }
            return 0;
        }
    } // namespace

    shell_uri shell_uri::parse(std::string value)
    {
        const std::size_t scheme_len = scheme_length(value);
        if (scheme_len == 0)
        {
            return relative(std::move(value));
        }

        shell_uri uri;
        uri.original_ = value;
        uri.is_absolute_ = true;
        uri.scheme_ = value.substr(0, scheme_len);

        std::string_view rest = std::string_view{value}.substr(scheme_len + 1);

        // Fragment first (everything after '#'), then query (after '?').
        if (const std::size_t hash = rest.find('#'); hash != std::string_view::npos)
        {
            uri.has_fragment_ = true;
            uri.fragment_ = std::string{rest.substr(hash + 1)};
            rest = rest.substr(0, hash);
        }
        if (const std::size_t question = rest.find('?'); question != std::string_view::npos)
        {
            uri.has_query_ = true;
            uri.query_ = std::string{rest.substr(question + 1)};
            rest = rest.substr(0, question);
        }

        // "//host/path" carries an authority; "/path" (or "path") has an empty host, like System.Uri.
        if (rest.starts_with("//"))
        {
            rest = rest.substr(2);
            const std::size_t slash = rest.find('/');
            if (slash == std::string_view::npos)
            {
                uri.host_ = std::string{rest};
                uri.path_ = "/";
            }
            else
            {
                uri.host_ = std::string{rest.substr(0, slash)};
                uri.path_ = std::string{rest.substr(slash)};
            }
        }
        else
        {
            uri.path_ = rest.starts_with('/') ? std::string{rest} : "/" + std::string{rest};
        }
        return uri;
    }

    shell_uri shell_uri::relative(std::string value)
    {
        shell_uri uri;
        uri.original_ = std::move(value);
        return uri;
    }

    std::string shell_uri::path_and_query() const
    {
        return has_query_ ? path_ + "?" + query_ : path_;
    }

    std::string shell_uri::query() const
    {
        return has_query_ ? "?" + query_ : std::string{};
    }

    std::string shell_uri::fragment() const
    {
        return has_fragment_ ? "#" + fragment_ : std::string{};
    }

    std::string shell_uri::to_string() const
    {
        if (!is_absolute_)
        {
            return original_;
        }
        return scheme_ + "://" + host_ + path_and_query() + fragment();
    }
} // namespace maui::controls
