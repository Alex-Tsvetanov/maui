#pragma once
// maui::xaml string -> value converters (M7)  <=  the C# TypeConverters the XAML loader applies.
//
// One free function per target type, each porting that type's ConvertFrom(InvariantString) path:
//
//   convert_color             <=  Microsoft.Maui.Graphics.Converters.ColorTypeConverter  (Color.Parse)
//   convert_point             <=  Microsoft.Maui.Graphics.Converters.PointTypeConverter  (Point.TryParse)
//   convert_rect              <=  Microsoft.Maui.Graphics.Converters.RectTypeConverter   (Rect.TryParse)
//   convert_size              <=  Microsoft.Maui.Graphics.Converters.SizeTypeConverter   (Size.TryParse)
//   convert_size_f            <=  Microsoft.Maui.Graphics.Converters.SizeFTypeConverter  (SizeF.TryParse)
//   convert_thickness         <=  Microsoft.Maui.Converters.ThicknessTypeConverter
//   convert_corner_radius     <=  Microsoft.Maui.Converters.CornerRadiusTypeConverter
//   convert_grid_length       <=  Microsoft.Maui.Converters.GridLengthTypeConverter.ParseStringToGridLength
//   convert_row_definitions   <=  Microsoft.Maui.Controls.RowDefinitionCollectionTypeConverter
//   convert_column_definitions<=  Microsoft.Maui.Controls.ColumnDefinitionCollectionTypeConverter
//   convert_layout_alignment  <=  Microsoft.Maui.Controls.LayoutOptionsConverter
//   convert_font_size         <=  Microsoft.Maui.Controls.FontSizeConverter
//   convert_is_visible        <=  Microsoft.Maui.Controls.VisualElement.VisibilityConverter (string -> bool)
//   convert_bool/double/float/int/string
//                             <=  Microsoft.Maui.Controls.Xaml.TypeConversionExtensions.ConvertTo's
//                                 built-in Boolean/Double/Single/Int32/String conversions
//   parse_enum / convert_text_alignment / convert_aspect / convert_visibility /
//   convert_flow_direction / convert_return_type
//                             <=  Enum.Parse(toType, str, ignoreCase: false) in TypeConversionExtensions
//                                 (FlowDirection additionally via Microsoft.Maui.Controls.FlowDirectionConverter)
//
// ERROR CHANNEL (the M7-wide contract): every converter throws maui::xaml::xaml_convert_error when
// the input cannot be converted. C# distinguishes InvalidOperationException / FormatException /
// ArgumentException per converter; the XAML loader (TypeConversionExtensions.ConvertTo) catches them
// all and surfaces one XamlParseException, so the distinction carries no information — the port folds
// them into this single exception type. An exception (not std::expected) because the C# converters
// throw, so call sites port 1:1, and a conversion failure is always fatal to the load.
//
// Deliberate deviations (all documented against the C# source):
//  - Numeric tokens use the port's standard rule (std::from_chars, chars_format::general, whole
//    trimmed token — the same rule as the shipped point/rect/size try_parse ports): C#'s
//    NumberStyles.Number thousands separators and trailing signs are not honored; exponent notation
//    and the (C#-also-accepted) infinity/NaN spellings are accepted.
//  - parse_enum accepts the numeric form only for DEFINED enumerator values; C# Enum.Parse would
//    also produce undefined values ((TextAlignment)5). Comma-combined [Flags] values are not
//    supported — no enum in the v1 table is [Flags].
//  - Named font sizes resolve via the Apple FontNamedSizeService table (see convert_font_size).
//  - convert_layout_alignment maps the legacy *AndExpand names to their base alignment; the C#
//    LayoutOptions.Expands flag has no representation in the port (the modern MAUI layout engine
//    ignores it outside legacy Compatibility layouts, and the port's i_view carries only
//    layout_alignment).
//  - C# Keyboard is a class with named INSTANCES (KeyboardTypeConverter), not an enum; the port has
//    no keyboard type yet, so its converter is deferred with it.
//  - C# converters throw on a null input string; std::string_view cannot be null — an empty view
//    follows each converter's empty-STRING behavior (throw everywhere except the row/column
//    definition collections, where "" yields an empty collection).

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
    // The single conversion-failure error of the XAML layer (see the header comment).
    class xaml_convert_error : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };

    // ---- graphics values ----
    [[nodiscard]] maui::graphics::color convert_color(std::string_view text);
    [[nodiscard]] maui::graphics::point convert_point(std::string_view text);   // "x,y"
    [[nodiscard]] maui::graphics::rect convert_rect(std::string_view text);     // "x,y,w,h"
    [[nodiscard]] maui::graphics::size convert_size(std::string_view text);     // "w,h"
    [[nodiscard]] maui::graphics::size_f convert_size_f(std::string_view text); // "w,h"

    // ThicknessTypeConverter: "a" uniform | "h,v" | "l,t,r,b" (XAML commas) and the CSS
    // space-separated forms "v h" | "t h b" | "t r b l".
    [[nodiscard]] maui::core::thickness convert_thickness(std::string_view text);

    // CornerRadiusTypeConverter: "a" uniform | "tl,tr,bl,br"; 2-3 comma values collapse to a uniform
    // radius from the FIRST value (C# quirk, kept); CSS space forms "t b" -> (t,b,b,t),
    // "tl trbl br" -> (tl,trbl,trbl,br), "tl tr bl br".
    [[nodiscard]] maui::graphics::corner_radius convert_corner_radius(std::string_view text);

    // GridLengthTypeConverter.ParseStringToGridLength: "auto" (any case) | "*" | "2*" | "123".
    [[nodiscard]] maui::core::grid_length convert_grid_length(std::string_view text);

    // Row/ColumnDefinitionCollectionTypeConverter: a comma-separated list of grid lengths
    // ("auto,*,2*,100"); "" yields an empty collection (the C# fast path).
    [[nodiscard]] std::vector<maui::controls::row_definition> convert_row_definitions(std::string_view text);
    [[nodiscard]] std::vector<maui::controls::column_definition> convert_column_definitions(std::string_view text);

    // LayoutOptionsConverter: "Start"/"Center"/"End"/"Fill" (+ the obsolete *AndExpand legacy names,
    // mapped to their base alignment) with an optional "LayoutOptions." qualifier. NOT trimmed and
    // case-sensitive, exactly like the C# converter.
    [[nodiscard]] maui::core::layout_alignment convert_layout_alignment(std::string_view text);

    // FontSizeConverter: a number, or a NamedSize name (case-sensitive: Default/Micro/Small/Medium/
    // Large/Body/Caption/Header/Subtitle/Title). C# resolves names through
    // Device.GetNamedSize(named, typeof(Label), false) -> the platform IFontNamedSizeService; the
    // port uses the Apple service's AppKit branch constants (accessibility scaling 1) from
    // src/Controls/src/Core/Compatibility/iOS/FontNamedSizeService.cs: Default=17 Micro=12 Small=14
    // Medium=17 Large=22 Body=23 Caption=18 Header=23 Subtitle=28 Title=34. (C#'s per-target-type
    // Default (Button -> 15) needs an IProvideValueTarget; like C#'s ConvertFrom path, the free
    // function always resolves against Label.)
    [[nodiscard]] double convert_font_size(std::string_view text);

    // VisualElement.VisibilityConverter — the [TypeConverter] on VisualElement.IsVisibleProperty:
    // "true"/"visible" -> true, "false"/"hidden"/"collapse" -> false (trimmed, case-insensitive;
    // note C# matches "collapse", NOT "collapsed").
    [[nodiscard]] bool convert_is_visible(std::string_view text);

    // ---- TypeConversionExtensions built-in passthroughs ----
    [[nodiscard]] bool convert_bool(std::string_view text);     // Boolean.Parse: trimmed, case-insensitive
    [[nodiscard]] double convert_double(std::string_view text); // Double.Parse(InvariantCulture)
    [[nodiscard]] float convert_float(std::string_view text);   // Single.Parse(InvariantCulture)
    [[nodiscard]] int convert_int(std::string_view text);       // Int32.Parse(InvariantCulture)
    // String passthrough: a leading "{}" (the XAML markup-extension escape) is stripped.
    [[nodiscard]] std::string convert_string(std::string_view text);

    // ---- enum parsing (no reflection: an explicit name table per enum) ----

    // One row of an enum's name table ("Center" -> text_alignment::center). Names are the C#
    // PascalCase member names, the values the port's enum class members.
    template <class E> struct enum_entry
    {
        std::string_view name;
        E value;
    };

    namespace detail
    {
        // String.Trim() over the .NET whitespace set that matters for ASCII input.
        [[nodiscard]] constexpr std::string_view trim(std::string_view s) noexcept
        {
            constexpr std::string_view whitespace = " \t\n\v\f\r";
            const auto begin = s.find_first_not_of(whitespace);
            if (begin == std::string_view::npos)
            {
                return {};
            }
            const auto end = s.find_last_not_of(whitespace);
            return s.substr(begin, end - begin + 1);
        }

        // Enum.Parse's numeric form: an optionally signed integral token.
        [[nodiscard]] inline bool try_parse_enum_number(std::string_view s, long long& out)
        {
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
            const auto [ptr, ec] = std::from_chars(first, last, out);
            return ec == std::errc{} && ptr == last;
        }
    } // namespace detail

    // C# Enum.Parse(type, str, ignoreCase: false) as the XAML loader uses it
    // (TypeConversionExtensions.ConvertTo): the value is trimmed, names match case-SENSITIVELY, and
    // a numeric token selects the enumerator with that underlying value (defined values only — see
    // the header deviations). Returns nullopt when nothing matches.
    template <class E>
    [[nodiscard]] std::optional<E> try_parse_enum(std::string_view text, std::span<const enum_entry<E>> names)
    {
        const std::string_view value = detail::trim(text);
        for (const auto& entry : names)
        {
            if (entry.name == value)
            {
                return entry.value;
            }
        }
        long long numeric = 0;
        if (detail::try_parse_enum_number(value, numeric))
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

    // The throwing form: the generic helper behind every convert_<enum> below.
    template <class E>
    [[nodiscard]] E parse_enum(std::string_view text, std::span<const enum_entry<E>> names,
                               std::string_view enum_type_name)
    {
        if (const auto parsed = try_parse_enum(text, names))
        {
            return *parsed;
        }
        throw xaml_convert_error(std::format("Cannot convert \"{}\" into {}", text, enum_type_name));
    }

    // Name tables for the enums the v1 control set exposes (C# member spellings).
    [[nodiscard]] maui::core::text_alignment convert_text_alignment(std::string_view text);
    [[nodiscard]] maui::core::aspect convert_aspect(std::string_view text);
    [[nodiscard]] maui::core::visibility convert_visibility(std::string_view text);
    [[nodiscard]] maui::core::return_type convert_return_type(std::string_view text);
    // FlowDirectionConverter: the enum names (via Enum.TryParse) plus the case-insensitive
    // "ltr" / "rtl" / "inherit" aliases (these aliases are NOT trimmed, matching C#).
    [[nodiscard]] maui::core::flow_direction convert_flow_direction(std::string_view text);
} // namespace maui::xaml
