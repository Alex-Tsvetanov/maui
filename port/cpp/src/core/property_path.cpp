// maui::core::property_path — string binding-path parsing (property_path.hpp).
// A 1:1 port of BindingExpression.ParsePath (src/Controls/src/Core/BindingExpression.cs).
#include "maui/core/property_path.hpp"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace maui::core
{
    namespace
    {
        [[nodiscard]] std::string_view trim(std::string_view text)
        {
            while (!text.empty() && std::string_view(" \t\r\n").contains(text.front()))
            {
                text.remove_prefix(1);
            }
            while (!text.empty() && std::string_view(" \t\r\n").contains(text.back()))
            {
                text.remove_suffix(1);
            }
            return text;
        }
    } // namespace

    property_path property_path::parse(std::string_view path)
    {
        property_path result;
        result.text_ = std::string{path};

        std::string_view p = trim(path);
        if (p.empty())
        {
            throw std::invalid_argument("property_path: path is empty");
        }

        // C# seeds the part list with the self part and consumes a leading '.'.
        result.parts_.push_back(part{.content = ".", .is_indexer = false, .is_self = true});
        if (p.front() == '.')
        {
            if (p.size() == 1)
            {
                return result;
            }
            p.remove_prefix(1);
        }

        // Split on '.', preserving empty segments (so "Foo." is detected as malformed, like C#).
        std::size_t start = 0;
        while (start <= p.size())
        {
            const std::size_t dot = p.find('.', start);
            const std::string_view segment =
                (dot == std::string_view::npos) ? p.substr(start) : p.substr(start, dot - start);
            std::string_view piece = trim(segment);
            if (piece.empty())
            {
                throw std::invalid_argument("property_path: path contains an empty part");
            }

            std::string indexer_content;
            bool has_indexer = false;
            const std::size_t lb_index = piece.find('[');
            if (lb_index != std::string_view::npos)
            {
                if (piece.back() != ']')
                {
                    throw std::invalid_argument("property_path: indexer did not contain closing bracket");
                }
                const std::size_t rb_index = piece.size() - 1;
                if (rb_index - lb_index - 1 == 0)
                {
                    throw std::invalid_argument("property_path: indexer did not contain arguments");
                }
                indexer_content = std::string{piece.substr(lb_index + 1, rb_index - lb_index - 1)};
                has_indexer = true;
                piece = trim(piece.substr(0, lb_index));
            }
            if (!piece.empty())
            {
                result.parts_.push_back(part{.content = std::string{piece}, .is_indexer = false, .is_self = false});
            }
            if (has_indexer)
            {
                result.parts_.push_back(
                    part{.content = std::move(indexer_content), .is_indexer = true, .is_self = false});
            }

            if (dot == std::string_view::npos)
            {
                break;
            }
            start = dot + 1;
            if (start == p.size())
            {
                // a trailing '.' leaves one empty segment ("Foo." in C# splits to ["Foo", ""])
                throw std::invalid_argument("property_path: path contains an empty part");
            }
        }
        return result;
    }
} // namespace maui::core
