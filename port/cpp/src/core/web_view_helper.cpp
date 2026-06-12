// escape_js_string — the WebViewHelper.EscapeJsString port (see web_view_helper.hpp). The C# original
// short-circuits when nothing needs escaping and then runs a Replace per special; the port does a single
// scan-and-append pass with identical output.

#include "maui/core/web_view_helper.hpp"

#include <cstddef>
#include <string>
#include <string_view>

namespace maui::core
{
    std::string escape_js_string(std::string_view js)
    {
        // UTF-8 encodings of U+2028 LINE SEPARATOR / U+2029 PARAGRAPH SEPARATOR (C# escapes both).
        static constexpr std::string_view line_separator = "\xE2\x80\xA8";
        static constexpr std::string_view paragraph_separator = "\xE2\x80\xA9";

        std::string result;
        result.reserve(js.size());
        for (std::size_t i = 0; i < js.size(); ++i)
        {
            const char current = js[i];
            switch (current)
            {
                case '\\':
                    result += "\\\\";
                    continue;
                case '\'':
                    result += "\\'";
                    continue;
                case '\n':
                    result += "\\n";
                    continue;
                case '\r':
                    result += "\\r";
                    continue;
                default:
                    break;
            }
            if (js.substr(i, line_separator.size()) == line_separator)
            {
                result += "\\u2028";
                i += line_separator.size() - 1;
                continue;
            }
            if (js.substr(i, paragraph_separator.size()) == paragraph_separator)
            {
                result += "\\u2029";
                i += paragraph_separator.size() - 1;
                continue;
            }
            result += current;
        }
        return result;
    }
} // namespace maui::core
