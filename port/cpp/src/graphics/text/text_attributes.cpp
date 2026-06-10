// maui::graphics::text::text_attributes — out-of-line definitions. See text_attributes.hpp.
// Ported from src/Graphics/src/Graphics/Text/{TextAttributes,TextAttributesExtensions,
// TextAttributeExtensions}.cs. Parsing mirrors C#: int.TryParse / float.TryParse (invariant) fall
// back to the default on failure; bool.TryParse is case-insensitive over "true"/"false"; enum names
// round-trip through their C# ToString() spelling.

#include "maui/graphics/text/text_attributes.hpp"

#include "maui/detail/charconv_compat.hpp"

#include <cctype>
#include <charconv>
#include <cstddef>
#include <format>
#include <iterator>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

namespace maui::graphics::text
{
    namespace
    {
        bool equals_ascii_ignore_case(std::string_view a, std::string_view b)
        {
            if (a.size() != b.size())
            {
                return false;
            }
            for (std::size_t i = 0; i < a.size(); i++)
            {
                const auto x = static_cast<unsigned char>(a[i]);
                const auto y = static_cast<unsigned char>(b[i]);
                if (std::tolower(x) != std::tolower(y))
                {
                    return false;
                }
            }
            return true;
        }

        // C# MarkerType.ToString() names — the wire format SetMarker stores and Enum.TryParse reads.
        constexpr std::string_view marker_name(marker_type value)
        {
            switch (value)
            {
                case marker_type::open_circle:
                    return "OpenCircle";
                case marker_type::hyphen:
                    return "Hyphen";
                case marker_type::closed_circle:
                default:
                    return "ClosedCircle";
            }
        }
    } // namespace

    text_attributes::text_attributes(const text_attributes& first, const text_attributes& second)
    {
        // C# TextAttributes(first, second): copy first, then second overwrites on collision.
        for (const auto& [key, value] : first.entries_)
        {
            entries_[key] = value;
        }
        for (const auto& [key, value] : second.entries_)
        {
            entries_[key] = value;
        }
    }

    std::size_t text_attributes::size() const
    {
        return entries_.size();
    }

    bool text_attributes::empty() const
    {
        return entries_.empty();
    }

    bool text_attributes::contains(text_attribute type) const
    {
        return entries_.contains(type);
    }

    const std::map<text_attribute, std::string>& text_attributes::entries() const
    {
        return entries_;
    }

    std::optional<std::string> text_attributes::get_attribute(text_attribute type) const
    {
        const auto found = entries_.find(type);
        if (found == entries_.end())
        {
            return std::nullopt;
        }
        return found->second;
    }

    std::string text_attributes::get_attribute(text_attribute type, std::string_view default_value) const
    {
        const auto found = entries_.find(type);
        if (found == entries_.end())
        {
            return std::string(default_value);
        }
        return found->second;
    }

    void text_attributes::set_attribute(text_attribute type, std::string value)
    {
        entries_[type] = std::move(value);
    }

    void text_attributes::remove_attribute(text_attribute type)
    {
        entries_.erase(type);
    }

    int text_attributes::get_int_attribute(text_attribute type, int default_value) const
    {
        const auto value = get_attribute(type);
        if (value.has_value())
        {
            const char* const first = value->data();
            const char* const last = std::next(first, static_cast<std::ptrdiff_t>(value->size()));
            int parsed = 0;
            const auto [ptr, ec] = std::from_chars(first, last, parsed);
            if (ec == std::errc{} && ptr == last)
            {
                return parsed;
            }
        }
        return default_value;
    }

    void text_attributes::set_int_attribute(text_attribute type, int value, int default_value)
    {
        // C# SetIntAttribute: storing the default removes the entry.
        if (value == default_value)
        {
            remove_attribute(type);
        }
        else
        {
            set_attribute(type, std::format("{}", value));
        }
    }

    float text_attributes::get_float_attribute(text_attribute type, float default_value) const
    {
        // C# float.TryParse (invariant): std::from_chars is locale-independent; the whole token
        // must be consumed (the same convention as the XAML converters). Routed through the
        // charconv_compat shim — NDK r27's libc++ 18 lacks the floating-point overload.
        const auto value = get_attribute(type);
        if (value.has_value())
        {
            const char* const first = value->data();
            const char* const last = std::next(first, static_cast<std::ptrdiff_t>(value->size()));
            float parsed = 0;
            const auto [ptr, ec] = maui::detail::from_chars_general(first, last, parsed);
            if (ec == std::errc{} && ptr == last)
            {
                return parsed;
            }
        }
        return default_value;
    }

    void text_attributes::set_float_attribute(text_attribute type, float value, float default_value)
    {
        // C# SetFloatAttribute: storing the default removes the entry (float == compare, as C#).
        if (value == default_value)
        {
            remove_attribute(type);
        }
        else
        {
            set_attribute(type, std::format("{}", value));
        }
    }

    bool text_attributes::get_bool_attribute(text_attribute type, bool default_value) const
    {
        const auto value = get_attribute(type);
        if (value.has_value())
        {
            // C# bool.TryParse: case-insensitive "True"/"False"; anything else fails to the default.
            if (equals_ascii_ignore_case(*value, "true"))
            {
                return true;
            }
            if (equals_ascii_ignore_case(*value, "false"))
            {
                return false;
            }
        }
        return default_value;
    }

    void text_attributes::set_bool_attribute(text_attribute type, bool value, bool default_value)
    {
        if (value == default_value)
        {
            remove_attribute(type);
        }
        else
        {
            // C# bool.ToString(): "True" / "False".
            set_attribute(type, value ? "True" : "False");
        }
    }

    std::optional<std::string> text_attributes::get_font_name() const
    {
        return get_attribute(text_attribute::font_name);
    }

    void text_attributes::set_font_name(const std::string& value)
    {
        set_attribute(text_attribute::font_name, value);
    }

    float text_attributes::get_font_size(std::optional<float> font_size) const
    {
        // C# GetFontSize(fontSize ?? DefaultFontSize).
        return get_float_attribute(text_attribute::font_size, font_size.value_or(default_font_size));
    }

    void text_attributes::set_font_size(float value)
    {
        set_float_attribute(text_attribute::font_size, value, default_font_size);
    }

    bool text_attributes::get_underline() const
    {
        return get_bool_attribute(text_attribute::underline);
    }

    void text_attributes::set_underline(bool value)
    {
        set_bool_attribute(text_attribute::underline, value);
    }

    bool text_attributes::get_bold() const
    {
        return get_bool_attribute(text_attribute::bold);
    }

    void text_attributes::set_bold(bool value)
    {
        set_bool_attribute(text_attribute::bold, value);
    }

    bool text_attributes::get_italic() const
    {
        return get_bool_attribute(text_attribute::italic);
    }

    void text_attributes::set_italic(bool value)
    {
        set_bool_attribute(text_attribute::italic, value);
    }

    bool text_attributes::get_unordered_list() const
    {
        return get_bool_attribute(text_attribute::unordered_list);
    }

    void text_attributes::set_unordered_list(bool value)
    {
        set_bool_attribute(text_attribute::unordered_list, value);
    }

    marker_type text_attributes::get_marker() const
    {
        // C# GetMarker: GetEnumAttribute<MarkerType>(TextAttribute.UnorderedList, ClosedCircle) —
        // keyed under UnorderedList (a C# quirk, ported as-is). Enum.TryParse is case-insensitive
        // only with the ignoreCase overload; the parameterless TryParse used here is case-SENSITIVE.
        const auto value = get_attribute(text_attribute::unordered_list);
        if (value.has_value())
        {
            if (*value == marker_name(marker_type::open_circle))
            {
                return marker_type::open_circle;
            }
            if (*value == marker_name(marker_type::hyphen))
            {
                return marker_type::hyphen;
            }
            if (*value == marker_name(marker_type::closed_circle))
            {
                return marker_type::closed_circle;
            }
        }
        return marker_type::closed_circle;
    }

    void text_attributes::set_marker(marker_type value)
    {
        // C# SetMarker: SetEnumAttribute(TextAttribute.UnorderedList, value, ClosedCircle) — the
        // default value removes the entry.
        if (value == marker_type::closed_circle)
        {
            remove_attribute(text_attribute::unordered_list);
        }
        else
        {
            set_attribute(text_attribute::unordered_list, std::string(marker_name(value)));
        }
    }

    bool text_attributes::get_strikethrough() const
    {
        return get_bool_attribute(text_attribute::strikethrough);
    }

    void text_attributes::set_strikethrough(bool value)
    {
        set_bool_attribute(text_attribute::strikethrough, value);
    }

    bool text_attributes::get_superscript() const
    {
        return get_bool_attribute(text_attribute::superscript);
    }

    void text_attributes::set_superscript(bool value)
    {
        set_bool_attribute(text_attribute::superscript, value);
    }

    bool text_attributes::get_subscript() const
    {
        return get_bool_attribute(text_attribute::subscript);
    }

    void text_attributes::set_subscript(bool value)
    {
        set_bool_attribute(text_attribute::subscript, value);
    }

    std::optional<std::string> text_attributes::get_foreground_color() const
    {
        return get_attribute(text_attribute::color);
    }

    void text_attributes::set_foreground_color(const std::string& value)
    {
        set_attribute(text_attribute::color, value);
    }

    std::optional<std::string> text_attributes::get_background_color() const
    {
        return get_attribute(text_attribute::background);
    }

    void text_attributes::set_background_color(const std::string& value)
    {
        set_attribute(text_attribute::background, value);
    }
} // namespace maui::graphics::text
