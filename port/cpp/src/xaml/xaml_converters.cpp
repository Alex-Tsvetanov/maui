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
#include <utility>
#include <vector>

#include "maui/animations/easing.hpp"
#include "maui/controls/column_definition.hpp"
#include "maui/controls/row_definition.hpp"
#include "maui/controls/stack_orientation.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/clear_button_visibility.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/grid_unit_type.hpp"
#include "maui/core/keyboard.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/return_type.hpp"
#include "maui/core/safe_area_edges.hpp"
#include "maui/core/safe_area_regions.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/text_decorations.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/detail/charconv_compat.hpp" // FP from_chars (general) with the libc++ < 20 fallback
#include "maui/graphics/color.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/size_f.hpp"
#include "maui/layouts/flex_basis.hpp"
#include "maui/layouts/flex_enums.hpp"

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
            const auto [ptr, ec] = maui::detail::from_chars_general(first, last, out);
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

        // Enum.TryParse(str, ignoreCase: true) as the Flex converters use it: a case-INSENSITIVE name
        // match over the member table, then the numeric form (a token whose value equals a defined
        // enumerator). nullopt when nothing matches. The C# converters do NOT trim, so neither does
        // this — the raw markup value is compared as-is.
        template <class E>
        [[nodiscard]] std::optional<E> try_parse_enum_ignore_case(std::string_view text,
                                                                  std::span<const enum_entry<E>> names)
        {
            for (const auto& entry : names)
            {
                if (equals_ignore_case(entry.name, text))
                {
                    return entry.value;
                }
            }
            long long numeric = 0;
            if (detail::try_parse_enum_number(text, numeric))
            {
                for (const auto& entry : names)
                {
                    if (static_cast<long long>(entry.value) == numeric)
                    {
                        return entry.value;
                    }
                }
            }
            return std::nullopt;
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

    // Microsoft.Maui.Controls.StackOrientation.
    maui::controls::stack_orientation convert_stack_orientation(std::string_view text)
    {
        using maui::controls::stack_orientation;
        static constexpr std::array<enum_entry<stack_orientation>, 2> names{{
            {.name = "Vertical", .value = stack_orientation::vertical},
            {.name = "Horizontal", .value = stack_orientation::horizontal},
        }};
        return parse_enum<stack_orientation>(text, names, "maui::controls::stack_orientation");
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

    // Microsoft.Maui.ClearButtonVisibility (no [TypeConverter] in C# — TypeConversionExtensions'
    // generic Enum.Parse path).
    maui::core::clear_button_visibility convert_clear_button_visibility(std::string_view text)
    {
        using maui::core::clear_button_visibility;
        static constexpr std::array<enum_entry<clear_button_visibility>, 2> names{{
            {.name = "Never", .value = clear_button_visibility::never},
            {.name = "WhileEditing", .value = clear_button_visibility::while_editing},
        }};
        return parse_enum<clear_button_visibility>(text, names, "maui::core::clear_button_visibility");
    }

    // Microsoft.Maui.Converters.EasingTypeConverter.ConvertFrom (src/Core/src/Converters/
    // EasingTypeConverter.cs): names compared OrdinalIgnoreCase; an exactly-two-part "Easing.<Name>"
    // spelling strips the qualifier first. C# returns null for null/whitespace input — the port's
    // easing has no null form, so that case throws (the header's documented deviation).
    maui::animations::easing convert_easing(std::string_view text)
    {
        using maui::animations::easing;
        if (detail::trim(text).empty())
        {
            throw_cannot_convert(text, "maui::animations::easing"); // C#: a null Easing
        }
        std::string_view value = text;
        // var parts = strValue.Split('.'); parts.Length == 2 && Compare(parts[0], nameof(Easing)).
        const std::size_t dot = value.find('.');
        if (dot != std::string_view::npos && value.find('.', dot + 1) == std::string_view::npos &&
            equals_ignore_case(value.substr(0, dot), "Easing"))
        {
            value.remove_prefix(dot + 1);
        }
        struct named_easing
        {
            std::string_view name;
            const easing& (*get)();
        };
        static constexpr std::array<named_easing, 11> names{{
            {.name = "Linear", .get = &easing::linear},
            {.name = "SinIn", .get = &easing::sin_in},
            {.name = "SinOut", .get = &easing::sin_out},
            {.name = "SinInOut", .get = &easing::sin_in_out},
            {.name = "CubicIn", .get = &easing::cubic_in},
            {.name = "CubicOut", .get = &easing::cubic_out},
            {.name = "CubicInOut", .get = &easing::cubic_in_out},
            {.name = "BounceIn", .get = &easing::bounce_in},
            {.name = "BounceOut", .get = &easing::bounce_out},
            {.name = "SpringIn", .get = &easing::spring_in},
            {.name = "SpringOut", .get = &easing::spring_out},
        }};
        for (const named_easing& entry : names)
        {
            if (equals_ignore_case(value, entry.name))
            {
                return entry.get();
            }
        }
        // C#'s InvalidOperationException carries the post-strip value.
        throw_cannot_convert(value, "maui::animations::easing");
    }

    // Microsoft.Maui.Controls.TextDecorationConverter.ConvertFrom (DecorableTextElement.cs): split
    // on ',' — or on ' ' when the comma split yields a single part — keeping EMPTY parts (C#
    // string.Split), each part trimmed and Enum.TryParse'd case-insensitively, with the untrimmed
    // "line-through" CSS alias, OR-combined into the [Flags] result.
    maui::core::text_decorations convert_text_decorations(std::string_view text)
    {
        using maui::core::text_decorations;
        static constexpr std::array<enum_entry<text_decorations>, 3> names{{
            {.name = "None", .value = text_decorations::none},
            {.name = "Underline", .value = text_decorations::underline},
            {.name = "Strikethrough", .value = text_decorations::strikethrough},
        }};
        const char separator = text.contains(',') ? ',' : ' ';
        auto result = std::to_underlying(text_decorations::none);
        std::size_t begin = 0;
        while (true)
        {
            const std::size_t end = text.find(separator, begin);
            const std::string_view item =
                text.substr(begin, end == std::string_view::npos ? std::string_view::npos : end - begin);
            const std::string_view trimmed = detail::trim(item);
            // Enum.TryParse(item.Trim(), ignoreCase: true): the case-insensitive name match (the
            // numeric form follows the port's defined-values rule via the case-sensitive helper).
            bool matched = false;
            for (const enum_entry<text_decorations>& entry : names)
            {
                if (equals_ignore_case(trimmed, entry.name))
                {
                    result |= std::to_underlying(entry.value);
                    matched = true;
                    break;
                }
            }
            if (!matched)
            {
                if (const auto numeric = try_parse_enum<text_decorations>(trimmed, names))
                {
                    result |= std::to_underlying(*numeric);
                }
                else if (equals_ignore_case(item, "line-through")) // the UNtrimmed C# alias check
                {
                    result |= std::to_underlying(text_decorations::strikethrough);
                }
                else
                {
                    throw_cannot_convert(item, "maui::core::text_decorations"); // C# names the ITEM
                }
            }
            if (end == std::string_view::npos)
            {
                break;
            }
            begin = end + 1;
        }
        return static_cast<text_decorations>(result);
    }

    // ---- X4 tail: Keyboard / Flex enums + FlexBasis / SafeAreaEdges ----

    // KeyboardTypeConverter.ConvertFrom: split on '.', accept a single part or "Keyboard.<name>", and
    // map the bare name to the matching static keyboard. C# reflects the static field/property by EXACT
    // (case-sensitive) name; the named-keyboard accessors here are that closed set.
    maui::core::keyboard convert_keyboard(std::string_view text)
    {
        using maui::core::keyboard;
        const std::vector<std::string_view> parts = split_keep_empty(text, '.');
        // parts.Length == 1, OR (Length == 2 AND parts[0] == "Keyboard"). Any other shape is unknown.
        std::string_view name;
        if (parts.size() == 1)
        {
            name = parts.front();
        }
        else if (parts.size() == 2 && parts.front() == "Keyboard")
        {
            name = parts.back();
        }
        else
        {
            throw_cannot_convert(text, "maui::core::keyboard");
        }
        // The C# Keyboard static members (case-sensitive name match — reflection by field/property name).
        if (name == "Default")
        {
            return keyboard::default_keyboard();
        }
        if (name == "Plain")
        {
            return keyboard::plain();
        }
        if (name == "Chat")
        {
            return keyboard::chat();
        }
        if (name == "Email")
        {
            return keyboard::email();
        }
        if (name == "Numeric")
        {
            return keyboard::numeric();
        }
        if (name == "Telephone")
        {
            return keyboard::telephone();
        }
        if (name == "Text")
        {
            return keyboard::text();
        }
        if (name == "Url")
        {
            return keyboard::url();
        }
        if (name == "Date")
        {
            return keyboard::date();
        }
        if (name == "Time")
        {
            return keyboard::time();
        }
        if (name == "Password")
        {
            return keyboard::password();
        }
        throw_cannot_convert(text, "maui::core::keyboard");
    }

    maui::layouts::flex_direction convert_flex_direction(std::string_view text)
    {
        using maui::layouts::flex_direction;
        static constexpr std::array<enum_entry<flex_direction>, 4> names{{
            {.name = "Column", .value = flex_direction::column},
            {.name = "ColumnReverse", .value = flex_direction::column_reverse},
            {.name = "Row", .value = flex_direction::row},
            {.name = "RowReverse", .value = flex_direction::row_reverse},
        }};
        if (const auto parsed = try_parse_enum_ignore_case<flex_direction>(text, names))
        {
            return *parsed;
        }
        if (equals_ignore_case(text, "row-reverse"))
        {
            return flex_direction::row_reverse;
        }
        if (equals_ignore_case(text, "column-reverse"))
        {
            return flex_direction::column_reverse;
        }
        throw_cannot_convert(text, "maui::layouts::flex_direction");
    }

    maui::layouts::flex_justify convert_flex_justify(std::string_view text)
    {
        using maui::layouts::flex_justify;
        static constexpr std::array<enum_entry<flex_justify>, 6> names{{
            {.name = "Start", .value = flex_justify::start},
            {.name = "Center", .value = flex_justify::center},
            {.name = "End", .value = flex_justify::end},
            {.name = "SpaceBetween", .value = flex_justify::space_between},
            {.name = "SpaceAround", .value = flex_justify::space_around},
            {.name = "SpaceEvenly", .value = flex_justify::space_evenly},
        }};
        if (const auto parsed = try_parse_enum_ignore_case<flex_justify>(text, names))
        {
            return *parsed;
        }
        if (equals_ignore_case(text, "flex-start"))
        {
            return flex_justify::start;
        }
        if (equals_ignore_case(text, "flex-end"))
        {
            return flex_justify::end;
        }
        if (equals_ignore_case(text, "space-between"))
        {
            return flex_justify::space_between;
        }
        if (equals_ignore_case(text, "space-around"))
        {
            return flex_justify::space_around;
        }
        throw_cannot_convert(text, "maui::layouts::flex_justify");
    }

    maui::layouts::flex_align_items convert_flex_align_items(std::string_view text)
    {
        using maui::layouts::flex_align_items;
        static constexpr std::array<enum_entry<flex_align_items>, 4> names{{
            {.name = "Stretch", .value = flex_align_items::stretch},
            {.name = "Center", .value = flex_align_items::center},
            {.name = "Start", .value = flex_align_items::start},
            {.name = "End", .value = flex_align_items::end},
        }};
        if (const auto parsed = try_parse_enum_ignore_case<flex_align_items>(text, names))
        {
            return *parsed;
        }
        if (equals_ignore_case(text, "flex-start"))
        {
            return flex_align_items::start;
        }
        if (equals_ignore_case(text, "flex-end"))
        {
            return flex_align_items::end;
        }
        throw_cannot_convert(text, "maui::layouts::flex_align_items");
    }

    maui::layouts::flex_align_content convert_flex_align_content(std::string_view text)
    {
        using maui::layouts::flex_align_content;
        static constexpr std::array<enum_entry<flex_align_content>, 7> names{{
            {.name = "Stretch", .value = flex_align_content::stretch},
            {.name = "Center", .value = flex_align_content::center},
            {.name = "Start", .value = flex_align_content::start},
            {.name = "End", .value = flex_align_content::end},
            {.name = "SpaceBetween", .value = flex_align_content::space_between},
            {.name = "SpaceAround", .value = flex_align_content::space_around},
            {.name = "SpaceEvenly", .value = flex_align_content::space_evenly},
        }};
        if (const auto parsed = try_parse_enum_ignore_case<flex_align_content>(text, names))
        {
            return *parsed;
        }
        if (equals_ignore_case(text, "flex-start"))
        {
            return flex_align_content::start;
        }
        if (equals_ignore_case(text, "flex-end"))
        {
            return flex_align_content::end;
        }
        if (equals_ignore_case(text, "space-between"))
        {
            return flex_align_content::space_between;
        }
        if (equals_ignore_case(text, "space-around"))
        {
            return flex_align_content::space_around;
        }
        throw_cannot_convert(text, "maui::layouts::flex_align_content");
    }

    maui::layouts::flex_align_self convert_flex_align_self(std::string_view text)
    {
        using maui::layouts::flex_align_self;
        static constexpr std::array<enum_entry<flex_align_self>, 5> names{{
            {.name = "Auto", .value = flex_align_self::auto_},
            {.name = "Stretch", .value = flex_align_self::stretch},
            {.name = "Center", .value = flex_align_self::center},
            {.name = "Start", .value = flex_align_self::start},
            {.name = "End", .value = flex_align_self::end},
        }};
        if (const auto parsed = try_parse_enum_ignore_case<flex_align_self>(text, names))
        {
            return *parsed;
        }
        if (equals_ignore_case(text, "flex-start"))
        {
            return flex_align_self::start;
        }
        if (equals_ignore_case(text, "flex-end"))
        {
            return flex_align_self::end;
        }
        throw_cannot_convert(text, "maui::layouts::flex_align_self");
    }

    maui::layouts::flex_wrap convert_flex_wrap(std::string_view text)
    {
        using maui::layouts::flex_wrap;
        static constexpr std::array<enum_entry<flex_wrap>, 3> names{{
            {.name = "NoWrap", .value = flex_wrap::no_wrap},
            {.name = "Wrap", .value = flex_wrap::wrap},
            {.name = "Reverse", .value = flex_wrap::reverse},
        }};
        if (const auto parsed = try_parse_enum_ignore_case<flex_wrap>(text, names))
        {
            return *parsed;
        }
        if (equals_ignore_case(text, "wrap-reverse"))
        {
            return flex_wrap::reverse;
        }
        throw_cannot_convert(text, "maui::layouts::flex_wrap");
    }

    // FlexBasisTypeConverter.ConvertFrom (the string path): trim, then "auto" / "N%" / "N".
    maui::layouts::flex_basis convert_flex_basis(std::string_view text)
    {
        using maui::layouts::flex_basis;
        const std::string_view trimmed = detail::trim(text);
        if (equals_ignore_case(trimmed, "auto"))
        {
            return flex_basis::auto_value;
        }
        if (trimmed.ends_with('%'))
        {
            float percentage = 0.0F;
            if (try_parse_component(trimmed.substr(0, trimmed.size() - 1), percentage))
            {
                // new FlexBasis(relflex / 100, isRelative: true) — the ctor validates [0,1].
                try
                {
                    return flex_basis{percentage / 100.0F, /*is_relative=*/true};
                }
                catch (const std::invalid_argument&)
                {
                    throw_cannot_convert(text, "maui::layouts::flex_basis");
                }
            }
            throw_cannot_convert(text, "maui::layouts::flex_basis");
        }
        float length = 0.0F;
        if (try_parse_component(trimmed, length))
        {
            // new FlexBasis(flex) — the absolute-length ctor; validates length >= 0.
            try
            {
                return flex_basis{length, /*is_relative=*/false};
            }
            catch (const std::invalid_argument&)
            {
                throw_cannot_convert(text, "maui::layouts::flex_basis");
            }
        }
        throw_cannot_convert(text, "maui::layouts::flex_basis");
    }

    // SafeAreaEdgesTypeConverter.ConvertFrom: trim whole, split on ',', map each part (case-insensitive),
    // then 1/2/4 parts -> uniform / horizontal,vertical / left,top,right,bottom.
    maui::core::safe_area_edges convert_safe_area_edges(std::string_view text)
    {
        using maui::core::safe_area_edges;
        using maui::core::safe_area_regions;
        const std::string_view trimmed = detail::trim(text);
        const std::vector<std::string_view> parts = split_keep_empty(trimmed, ',');
        std::vector<safe_area_regions> regions;
        regions.reserve(parts.size());
        for (const std::string_view raw : parts)
        {
            const std::string_view part = detail::trim(raw);
            if (equals_ignore_case(part, "All"))
            {
                regions.push_back(safe_area_regions::all);
            }
            else if (equals_ignore_case(part, "None"))
            {
                regions.push_back(safe_area_regions::none);
            }
            else if (equals_ignore_case(part, "Container"))
            {
                regions.push_back(safe_area_regions::container);
            }
            else if (equals_ignore_case(part, "SoftInput"))
            {
                regions.push_back(safe_area_regions::soft_input);
            }
            else if (equals_ignore_case(part, "Default"))
            {
                regions.push_back(safe_area_regions::default_value);
            }
            else
            {
                // C# FormatException names the offending PART, not the whole string.
                throw_cannot_convert(part, "maui::core::safe_area_regions");
            }
        }
        switch (regions.size())
        {
            case 1:
                return safe_area_edges{regions[0]};
            case 2:
                return safe_area_edges{regions[0], regions[1]}; // horizontal, vertical
            case 4:
                return safe_area_edges{regions[0], regions[1], regions[2], regions[3]};
            default:
                // C#: "SafeAreaEdges must have 1, 2, or 4 values, but got {n}".
                throw xaml_convert_error(
                    std::format("SafeAreaEdges must have 1, 2, or 4 values, but got {}", regions.size()));
        }
    }
} // namespace maui::xaml
