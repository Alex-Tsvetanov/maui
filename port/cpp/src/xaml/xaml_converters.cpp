// maui::xaml string -> value converters — implementation. The header carries the C# mapping table;
// each function body cites the converter it ports. Behavior is derived from:
//   src/Core/src/Converters/{ThicknessTypeConverter,CornerRadiusTypeConverter,GridLengthTypeConverter}.cs
//   src/Controls/src/Core/{RowDefinitionCollectionTypeConverter,ColumnDefinitionCollectionTypeConverter,
//                          LayoutOptionsConverter,FontSizeConverter,FlowDirectionConverter}.cs
//   src/Controls/src/Core/VisualElement/VisualElement.cs (VisibilityConverter)
//   src/Controls/src/Core/Xaml/TypeConversionExtensions.cs (built-in + enum conversions)
//   src/Graphics/src/Graphics/Converters/*.cs (+ Color.Parse / Point.TryParse / Rect.TryParse /
//                          Size.TryParse / SizeF.TryParse in the Graphics value types)

#include "maui/xaml/xaml_converters.hpp"

#include <array>
#include <charconv>
#include <cstddef>
#include <format>
#include <iterator>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "maui/controls/column_definition.hpp"
#include "maui/controls/row_definition.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/grid_unit_type.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/return_type.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/size_f.hpp"

namespace maui::xaml
{
    namespace
    {
        [[noreturn]] void throw_cannot_convert(std::string_view text, std::string_view type_name)
        {
            // C#: throw new InvalidOperationException($"Cannot convert \"{strValue}\" into {type}").
            throw xaml_convert_error(std::format("Cannot convert \"{}\" into {}", text, type_name));
        }

        // double.TryParse(s, NumberStyles.Number, CultureInfo.InvariantCulture) stand-in for the
        // split-out components, using the port's standard numeric-token rule (see the header
        // deviations note): trim, then std::from_chars over the whole remaining token. C# Number
        // style also allows a leading '+', which from_chars does not — strip it.
        template <class T> [[nodiscard]] bool try_parse_component(std::string_view s, T& out)
        {
            s = detail::trim(s);
            if (s.size() >= 2 && s.front() == '+' && s[1] != '+' && s[1] != '-')
            {
                s.remove_prefix(1);
            }
            if (s.empty())
            {
                return false;
            }
            const char* first = s.data();
            const char* last = std::next(first, static_cast<std::ptrdiff_t>(s.size()));
            const auto [ptr, ec] = std::from_chars(first, last, out, std::chars_format::general);
            return ec == std::errc{} && ptr == last;
        }

        // C# string.Split(delimiter) — empty entries are KEPT (so "1  2" splits into three parts,
        // the middle one empty, and fails the per-part parse exactly like C#).
        [[nodiscard]] std::vector<std::string_view> split_keep_empty(std::string_view s, char delimiter)
        {
            std::vector<std::string_view> parts;
            std::size_t begin = 0;
            while (true)
            {
                const auto pos = s.find(delimiter, begin);
                if (pos == std::string_view::npos)
                {
                    parts.push_back(s.substr(begin));
                    return parts;
                }
                parts.push_back(s.substr(begin, pos - begin));
                begin = pos + 1;
            }
        }

        [[nodiscard]] constexpr char to_lower_ascii(char c) noexcept
        {
            return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
        }

        // StringComparison.OrdinalIgnoreCase over the ASCII range (every keyword compared here is ASCII).
        [[nodiscard]] bool equals_ignore_case(std::string_view a, std::string_view b) noexcept
        {
            if (a.size() != b.size())
            {
                return false;
            }
            for (std::size_t i = 0; i < a.size(); ++i)
            {
                if (to_lower_ascii(a[i]) != to_lower_ascii(b[i]))
                {
                    return false;
                }
            }
            return true;
        }
    } // namespace

    // ---- graphics values ----

    // Microsoft.Maui.Graphics.Converters.ColorTypeConverter.ConvertFrom -> Color.Parse. Reuses the
    // port's color::try_parse (hex #RGB/#ARGB/#RRGGBB/#AARRGGBB, rgb()/rgba(), hsl()/hsla(),
    // hsv()/hsva(), and the 147 named colors), adding only the converter's error behavior.
    maui::graphics::color convert_color(std::string_view text)
    {
        maui::graphics::color parsed;
        if (maui::graphics::color::try_parse(text, parsed))
        {
            return parsed;
        }
        throw_cannot_convert(text, "maui::graphics::color");
    }

    // Microsoft.Maui.Graphics.Converters.PointTypeConverter.ConvertFrom -> Point.TryParse ("x,y").
    maui::graphics::point convert_point(std::string_view text)
    {
        maui::graphics::point parsed;
        if (maui::graphics::point::try_parse(text, parsed))
        {
            return parsed;
        }
        throw_cannot_convert(text, "maui::graphics::point");
    }

    // Microsoft.Maui.Graphics.Converters.RectTypeConverter.ConvertFrom -> Rect.TryParse ("x,y,w,h").
    maui::graphics::rect convert_rect(std::string_view text)
    {
        maui::graphics::rect parsed;
        if (maui::graphics::rect::try_parse(text, parsed))
        {
            return parsed;
        }
        throw_cannot_convert(text, "maui::graphics::rect");
    }

    // Microsoft.Maui.Graphics.Converters.SizeTypeConverter.ConvertFrom -> Size.TryParse ("w,h").
    maui::graphics::size convert_size(std::string_view text)
    {
        maui::graphics::size parsed;
        if (maui::graphics::size::try_parse(text, parsed))
        {
            return parsed;
        }
        throw_cannot_convert(text, "maui::graphics::size");
    }

    // Microsoft.Maui.Graphics.Converters.SizeFTypeConverter.ConvertFrom -> SizeF.TryParse ("w,h").
    maui::graphics::size_f convert_size_f(std::string_view text)
    {
        maui::graphics::size_f parsed;
        if (maui::graphics::size_f::try_parse(text, parsed))
        {
            return parsed;
        }
        throw_cannot_convert(text, "maui::graphics::size_f");
    }

    // Microsoft.Maui.Converters.ThicknessTypeConverter.ConvertFrom. Branch order matches C#: the
    // trimmed value is split on ',' (XAML) when it contains one, else on ' ' (CSS) when it contains
    // one, else parsed as a single uniform thickness. A branch that matches but fails to parse falls
    // through to the throw — it never tries another branch.
    maui::core::thickness convert_thickness(std::string_view text)
    {
        const std::string_view value = detail::trim(text);
        if (value.contains(','))
        { // XAML
            const auto parts = split_keep_empty(value, ',');
            if (parts.size() == 2)
            {
                double h = 0;
                double v = 0;
                if (try_parse_component(parts[0], h) && try_parse_component(parts[1], v))
                {
                    return {h, v};
                }
            }
            else if (parts.size() == 4)
            {
                double l = 0;
                double t = 0;
                double r = 0;
                double b = 0;
                if (try_parse_component(parts[0], l) && try_parse_component(parts[1], t) &&
                    try_parse_component(parts[2], r) && try_parse_component(parts[3], b))
                {
                    return {l, t, r, b};
                }
            }
        }
        else if (value.contains(' '))
        { // CSS
            const auto parts = split_keep_empty(value, ' ');
            if (parts.size() == 2)
            { // "v h"
                double v = 0;
                double h = 0;
                if (try_parse_component(parts[0], v) && try_parse_component(parts[1], h))
                {
                    return {h, v};
                }
            }
            else if (parts.size() == 3)
            { // "t h b"
                double t = 0;
                double h = 0;
                double b = 0;
                if (try_parse_component(parts[0], t) && try_parse_component(parts[1], h) &&
                    try_parse_component(parts[2], b))
                {
                    return {h, t, h, b};
                }
            }
            else if (parts.size() == 4)
            { // "t r b l"
                double t = 0;
                double r = 0;
                double b = 0;
                double l = 0;
                if (try_parse_component(parts[0], t) && try_parse_component(parts[1], r) &&
                    try_parse_component(parts[2], b) && try_parse_component(parts[3], l))
                {
                    return {l, t, r, b};
                }
            }
        }
        else
        { // single uniform thickness
            double uniform = 0;
            if (try_parse_component(value, uniform))
            {
                return {uniform};
            }
        }
        throw_cannot_convert(text, "maui::core::thickness");
    }

    // Microsoft.Maui.Converters.CornerRadiusTypeConverter.ConvertFrom. Same branch structure as the
    // thickness converter; note the C# quirk (kept): 2 or 3 comma-separated values produce a UNIFORM
    // radius from the first value only — the rest are not even parsed.
    maui::graphics::corner_radius convert_corner_radius(std::string_view text)
    {
        const std::string_view value = detail::trim(text);
        if (value.contains(','))
        { // XAML
            const auto parts = split_keep_empty(value, ',');
            if (parts.size() == 4)
            {
                double tl = 0;
                double tr = 0;
                double bl = 0;
                double br = 0;
                if (try_parse_component(parts[0], tl) && try_parse_component(parts[1], tr) &&
                    try_parse_component(parts[2], bl) && try_parse_component(parts[3], br))
                {
                    return {tl, tr, bl, br};
                }
            }
            if (parts.size() > 1 && parts.size() < 4)
            {
                double uniform = 0;
                if (try_parse_component(parts[0], uniform))
                {
                    return {uniform};
                }
            }
        }
        else if (value.contains(' '))
        { // CSS
            const auto parts = split_keep_empty(value, ' ');
            if (parts.size() == 2)
            { // "t b" -> (t, b, b, t)
                double t = 0;
                double b = 0;
                if (try_parse_component(parts[0], t) && try_parse_component(parts[1], b))
                {
                    return {t, b, b, t};
                }
            }
            if (parts.size() == 3)
            { // "tl trbl br" -> (tl, trbl, trbl, br)
                double tl = 0;
                double trbl = 0;
                double br = 0;
                if (try_parse_component(parts[0], tl) && try_parse_component(parts[1], trbl) &&
                    try_parse_component(parts[2], br))
                {
                    return {tl, trbl, trbl, br};
                }
            }
            if (parts.size() == 4)
            {
                double tl = 0;
                double tr = 0;
                double bl = 0;
                double br = 0;
                if (try_parse_component(parts[0], tl) && try_parse_component(parts[1], tr) &&
                    try_parse_component(parts[2], bl) && try_parse_component(parts[3], br))
                {
                    return {tl, tr, bl, br};
                }
            }
        }
        else
        { // single uniform corner radius
            double uniform = 0;
            if (try_parse_component(value, uniform))
            {
                return {uniform};
            }
        }
        throw_cannot_convert(text, "maui::graphics::corner_radius");
    }

    // Microsoft.Maui.Converters.GridLengthTypeConverter.ParseStringToGridLength. The grid_length
    // constructor's own validation (negative / NaN -> std::invalid_argument; ArgumentException in
    // C#) is folded into the single M7 error channel.
    maui::core::grid_length convert_grid_length(std::string_view text)
    {
        const std::string_view value = detail::trim(text);
        try
        {
            if (!value.empty())
            {
                if (value.size() == 4 && equals_ignore_case(value, "auto"))
                {
                    return maui::core::grid_length::automatic();
                }
                if (value.size() == 1 && value.front() == '*')
                {
                    return maui::core::grid_length::star();
                }
                if (value.back() == '*')
                {
                    const std::string_view prefix = value.substr(0, value.size() - 1);
                    double star_weight = 0;
                    if (try_parse_component(prefix, star_weight))
                    {
                        return {star_weight, maui::core::grid_unit_type::star};
                    }
                }
                double absolute = 0;
                if (try_parse_component(value, absolute))
                {
                    return {absolute};
                }
            }
        }
        catch (const std::invalid_argument& error)
        {
            throw xaml_convert_error(error.what());
        }
        // C#: throw new FormatException($"Invalid GridLength format: {value}").
        throw xaml_convert_error(std::format("Invalid grid_length format: {}", text));
    }

    namespace
    {
        // The shared body of the Row/ColumnDefinitionCollectionTypeConverter twins: split on ','
        // and parse each entry as a grid length. ("" is the C# fast path for an empty collection.)
        template <class Definition> [[nodiscard]] std::vector<Definition> convert_definitions(std::string_view text)
        {
            std::vector<Definition> definitions;
            if (text.empty())
            {
                return definitions;
            }
            const auto parts = split_keep_empty(text, ',');
            definitions.reserve(parts.size());
            for (const auto part : parts)
            {
                definitions.emplace_back(convert_grid_length(part));
            }
            return definitions;
        }
    } // namespace

    // Microsoft.Maui.Controls.RowDefinitionCollectionTypeConverter.ConvertFrom.
    std::vector<maui::controls::row_definition> convert_row_definitions(std::string_view text)
    {
        return convert_definitions<maui::controls::row_definition>(text);
    }

    // Microsoft.Maui.Controls.ColumnDefinitionCollectionTypeConverter.ConvertFrom — the column twin.
    std::vector<maui::controls::column_definition> convert_column_definitions(std::string_view text)
    {
        return convert_definitions<maui::controls::column_definition>(text);
    }

    // Microsoft.Maui.Controls.LayoutOptionsConverter.ConvertFrom. The C# converter does NOT trim;
    // it splits on '.', allows at most one "LayoutOptions" qualifier, then matches the 8 static
    // LayoutOptions field names exactly. (Its reflection fallback can only rediscover those same 8
    // public static fields, so the table is complete.) The *AndExpand legacy names map to the same
    // LayoutAlignment with Expands=true in C#; the port keeps the alignment (header deviations).
    maui::core::layout_alignment convert_layout_alignment(std::string_view text)
    {
        std::string_view value = text;
        const auto first_dot = text.find('.');
        if (first_dot != std::string_view::npos)
        {
            const bool more_dots = text.find('.', first_dot + 1) != std::string_view::npos;
            if (more_dots || text.substr(0, first_dot) != "LayoutOptions")
            {
                throw_cannot_convert(text, "maui::core::layout_alignment");
            }
            value = text.substr(first_dot + 1);
        }
        using maui::core::layout_alignment;
        static constexpr std::array<enum_entry<layout_alignment>, 8> options{{
            {.name = "Start", .value = layout_alignment::start},
            {.name = "Center", .value = layout_alignment::center},
            {.name = "End", .value = layout_alignment::end},
            {.name = "Fill", .value = layout_alignment::fill},
            {.name = "StartAndExpand", .value = layout_alignment::start},
            {.name = "CenterAndExpand", .value = layout_alignment::center},
            {.name = "EndAndExpand", .value = layout_alignment::end},
            {.name = "FillAndExpand", .value = layout_alignment::fill},
        }};
        for (const auto& entry : options)
        {
            if (entry.name == value)
            {
                return entry.value;
            }
        }
        throw_cannot_convert(text, "maui::core::layout_alignment");
    }

    // Microsoft.Maui.Controls.FontSizeConverter.ConvertFrom: a number wins outright; otherwise the
    // trimmed value is matched (Ordinal, case-sensitive) against the NamedSize names and resolved
    // through the Apple FontNamedSizeService constants (see the header). C#'s final
    // Enum.TryParse(NamedSize) fallback adds nothing: all ten names were just checked and numeric
    // strings were already returned as sizes by the first step.
    double convert_font_size(std::string_view text)
    {
        double size = 0;
        if (try_parse_component(text, size))
        {
            return size;
        }
        const std::string_view value = detail::trim(text);
        struct named_size
        {
            std::string_view name;
            double size;
        };
        static constexpr std::array<named_size, 10> named_sizes{{
            {.name = "Default", .size = 17.0},
            {.name = "Micro", .size = 12.0},
            {.name = "Small", .size = 14.0},
            {.name = "Medium", .size = 17.0},
            {.name = "Large", .size = 22.0},
            {.name = "Body", .size = 23.0},
            {.name = "Caption", .size = 18.0},
            {.name = "Header", .size = 23.0},
            {.name = "Subtitle", .size = 28.0},
            {.name = "Title", .size = 34.0},
        }};
        for (const auto& entry : named_sizes)
        {
            if (entry.name == value)
            {
                return entry.size;
            }
        }
        // C#: throw new InvalidOperationException(... into typeof(double)).
        throw_cannot_convert(text, "double (font size)");
    }

    // Microsoft.Maui.Controls.VisualElement.VisibilityConverter.ConvertFrom (string -> bool for
    // IsVisible): trimmed, case-insensitive; "collapse" (not "collapsed") is the false alias.
    bool convert_is_visible(std::string_view text)
    {
        const std::string_view value = detail::trim(text);
        if (!value.empty())
        {
            if (equals_ignore_case(value, "true") || equals_ignore_case(value, "visible"))
            {
                return true;
            }
            if (equals_ignore_case(value, "false") || equals_ignore_case(value, "hidden") ||
                equals_ignore_case(value, "collapse"))
            {
                return false;
            }
        }
        throw_cannot_convert(text, "bool (is_visible)");
    }

    // ---- TypeConversionExtensions built-in passthroughs ----

    // Boolean.Parse: trimmed, case-insensitive "True"/"False"; FormatException otherwise.
    bool convert_bool(std::string_view text)
    {
        const std::string_view value = detail::trim(text);
        if (equals_ignore_case(value, "true"))
        {
            return true;
        }
        if (equals_ignore_case(value, "false"))
        {
            return false;
        }
        throw_cannot_convert(text, "bool");
    }

    // Double.Parse(str, CultureInfo.InvariantCulture) — NumberStyles.Float | AllowThousands, so
    // exponent notation is valid here (and the port's numeric-token rule accepts it everywhere).
    double convert_double(std::string_view text)
    {
        double value = 0;
        if (try_parse_component(text, value))
        {
            return value;
        }
        throw_cannot_convert(text, "double");
    }

    // Single.Parse(str, CultureInfo.InvariantCulture).
    float convert_float(std::string_view text)
    {
        float value = 0;
        if (try_parse_component(text, value))
        {
            return value;
        }
        throw_cannot_convert(text, "float");
    }

    // Int32.Parse(str, CultureInfo.InvariantCulture) — NumberStyles.Integer: surrounding whitespace,
    // a leading sign, and decimal digits only. Out-of-range input throws like C#'s OverflowException
    // (folded into the single error channel).
    int convert_int(std::string_view text)
    {
        std::string_view value = detail::trim(text);
        if (value.size() >= 2 && value.front() == '+' && value[1] != '+' && value[1] != '-')
        {
            value.remove_prefix(1);
        }
        if (!value.empty())
        {
            const char* first = value.data();
            const char* last = std::next(first, static_cast<std::ptrdiff_t>(value.size()));
            int parsed = 0;
            const auto [ptr, ec] = std::from_chars(first, last, parsed);
            if (ec == std::errc{} && ptr == last)
            {
                return parsed;
            }
        }
        throw_cannot_convert(text, "int");
    }

    // The String built-in: a leading "{}" (the XAML escape for a literal brace) is stripped; any
    // other string passes through unchanged.
    std::string convert_string(std::string_view text)
    {
        if (text.starts_with("{}"))
        {
            text.remove_prefix(2);
        }
        return std::string(text);
    }

    // ---- enum name tables (Enum.Parse with ignoreCase: false; C# member spellings) ----

    // Microsoft.Maui.TextAlignment.
    maui::core::text_alignment convert_text_alignment(std::string_view text)
    {
        using maui::core::text_alignment;
        static constexpr std::array<enum_entry<text_alignment>, 4> names{{
            {.name = "Start", .value = text_alignment::start},
            {.name = "Center", .value = text_alignment::center},
            {.name = "End", .value = text_alignment::end},
            {.name = "Justify", .value = text_alignment::justify},
        }};
        return parse_enum<text_alignment>(text, names, "maui::core::text_alignment");
    }

    // Microsoft.Maui.Aspect.
    maui::core::aspect convert_aspect(std::string_view text)
    {
        using maui::core::aspect;
        static constexpr std::array<enum_entry<aspect>, 4> names{{
            {.name = "AspectFit", .value = aspect::aspect_fit},
            {.name = "AspectFill", .value = aspect::aspect_fill},
            {.name = "Fill", .value = aspect::fill},
            {.name = "Center", .value = aspect::center},
        }};
        return parse_enum<aspect>(text, names, "maui::core::aspect");
    }

    // Microsoft.Maui.Visibility (the Core enum — distinct from convert_is_visible's string -> bool).
    maui::core::visibility convert_visibility(std::string_view text)
    {
        using maui::core::visibility;
        static constexpr std::array<enum_entry<visibility>, 3> names{{
            {.name = "Visible", .value = visibility::visible},
            {.name = "Hidden", .value = visibility::hidden},
            {.name = "Collapsed", .value = visibility::collapsed},
        }};
        return parse_enum<visibility>(text, names, "maui::core::visibility");
    }

    // Microsoft.Maui.ReturnType.
    maui::core::return_type convert_return_type(std::string_view text)
    {
        using maui::core::return_type;
        static constexpr std::array<enum_entry<return_type>, 6> names{{
            {.name = "Default", .value = return_type::default_},
            {.name = "Done", .value = return_type::done},
            {.name = "Go", .value = return_type::go},
            {.name = "Next", .value = return_type::next},
            {.name = "Search", .value = return_type::search},
            {.name = "Send", .value = return_type::send},
        }};
        return parse_enum<return_type>(text, names, "maui::core::return_type");
    }

    // Microsoft.Maui.Controls.FlowDirectionConverter.ConvertFrom: Enum.TryParse first (the table,
    // trimmed + case-sensitive), then the case-insensitive "ltr"/"rtl"/"inherit" aliases compared
    // against the RAW (untrimmed) value, exactly like C#'s string.Equals calls.
    maui::core::flow_direction convert_flow_direction(std::string_view text)
    {
        using maui::core::flow_direction;
        static constexpr std::array<enum_entry<flow_direction>, 3> names{{
            {.name = "MatchParent", .value = flow_direction::match_parent},
            {.name = "LeftToRight", .value = flow_direction::left_to_right},
            {.name = "RightToLeft", .value = flow_direction::right_to_left},
        }};
        if (const auto parsed = try_parse_enum<flow_direction>(text, names))
        {
            return *parsed;
        }
        if (equals_ignore_case(text, "ltr"))
        {
            return flow_direction::left_to_right;
        }
        if (equals_ignore_case(text, "rtl"))
        {
            return flow_direction::right_to_left;
        }
        if (equals_ignore_case(text, "inherit"))
        {
            return flow_direction::match_parent;
        }
        throw_cannot_convert(text, "maui::core::flow_direction");
    }
} // namespace maui::xaml
