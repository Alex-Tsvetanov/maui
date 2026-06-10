// maui::graphics::font — out-of-line definitions. See font.hpp. Ported from
// src/Graphics/src/Graphics/Font.cs: name/weight/style storage plus the case-insensitive equality.

#include "maui/graphics/font.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>

namespace maui::graphics
{
    namespace
    {
        // C# Name.Equals(other.Name, StringComparison.OrdinalIgnoreCase): ordinal (byte-wise)
        // comparison after ASCII case folding — locale-independent, exactly the ordinal semantics.
        bool equals_ordinal_ignore_case(const std::string& a, const std::string& b)
        {
            return a.size() == b.size() &&
                   std::equal(a.begin(), a.end(), b.begin(),
                              [](unsigned char x, unsigned char y) { return std::tolower(x) == std::tolower(y); });
        }
    } // namespace

    font::font(std::string name, int weight, font_style_type style_type)
        : name_(std::move(name)), weight_(weight), style_type_(style_type)
    {
    }

    font font::default_font()
    {
        // C# Font.Default => new Font(null).
        return font{};
    }

    font font::default_bold()
    {
        // C# Font.DefaultBold => new Font(null, FontWeights.Bold).
        font value;
        value.weight_ = font_weights::bold;
        return value;
    }

    const std::string& font::name() const
    {
        return name_;
    }

    int font::weight() const
    {
        return weight_;
    }

    font_style_type font::style_type() const
    {
        return style_type_;
    }

    bool font::is_default() const
    {
        // C# Font.IsDefault => string.IsNullOrEmpty(Name).
        return name_.empty();
    }

    bool font::equals(const font& other) const
    {
        // C# Font.Equals(IFont): StyleType == && Weight == && (both names null || OrdinalIgnoreCase).
        // The port's empty string stands in for null, so empty==empty covers the both-null branch.
        return style_type_ == other.style_type_ && weight_ == other.weight_ &&
               equals_ordinal_ignore_case(name_, other.name_);
    }
} // namespace maui::graphics
