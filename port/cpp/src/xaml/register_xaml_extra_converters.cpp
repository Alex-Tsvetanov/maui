// maui::xaml — shared extra converter registrations.
//
// This TU contains every value converter that (a) is NOT already in the standard table built by
// register_standard_xaml_converters (xaml_standard_types.cpp) and (b) is needed by MORE THAN ONE
// of the per-group registration files, OR is a natural companion to a group-specific type that
// belongs in one central place rather than repeated across TUs.
//
// Each converter here ports the corresponding C# TypeConverter / Enum.Parse path.  The shared
// helpers (parse_enum, try_parse_enum, the anonymous utility functions) follow the same patterns
// as xaml_converters.cpp — see that file for the documented deviations and the error-channel rule.
//
// CONVERTERS DELIBERATELY ABSENT FROM THIS FILE (and why):
//   maui::core::font              — font sub-attributes deferred (xaml_standard_types.cpp §deferrals).
//   std::optional<std::string>    — use a register_property<TControl,std::string> lambda per control.
//   std::optional<date_time>      — see register_optional_date_time / register_optional_time_span below;
//                                   the non-optional date_time / time_span converters ARE registered.
//   std::shared_ptr<paint>        — text path routes through std::shared_ptr<brush> (already in the
//                                   standard table); Fill/Stroke shape attributes use register_property
//                                   lambdas that call set_fill_brush / set_stroke_brush instead.
//   std::shared_ptr<i_image_source>  — binding-only; no text converter.
//   std::shared_ptr<i_drawable>      — binding-only; no text converter.
//   std::shared_ptr<web_view_source> — binding-only; no text converter.
//   std::shared_ptr<i_shape> (StrokeShape) — property-element only; no text converter.

#include "register_xaml_groups.hpp"

#include "maui/detail/charconv_compat.hpp" // portable float from_chars (Android NDK libc++ lacks FP)

#include <array>
#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iterator>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "maui/controls/editor_auto_size_option.hpp"
#include "maui/controls/flyout_layout_behavior.hpp"
#include "maui/controls/indicator_shape.hpp"
#include "maui/controls/shapes/fill_rule.hpp"
#include "maui/controls/shapes/path_segment.hpp" // point_collection typedef
#include "maui/controls/shell/flyout_behavior.hpp"
#include "maui/controls/shell/flyout_header_behavior.hpp"
#include "maui/core/date_time.hpp"
#include "maui/core/path_aspect.hpp"
#include "maui/core/scroll_bar_visibility.hpp"
#include "maui/core/scroll_orientation.hpp"
#include "maui/core/swipe_transition_mode.hpp"
#include "maui/graphics/line_cap.hpp"
#include "maui/graphics/line_join.hpp"
#include "maui/graphics/point.hpp"
#include "maui/layouts/absolute_layout_flags.hpp"
#include "maui/layouts/flex_enums.hpp" // flex_position
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_converters.hpp"      // parse_enum, try_parse_enum, enum_entry, xaml_convert_error
#include "maui/xaml/xaml_parse_exception.hpp" // xaml_parse_exception (registry bridge below)

namespace maui::xaml
{
    namespace
    {
        // ---- registry_converter bridge (mirrors the anonymous helper in xaml_standard_types.cpp) ----
        //
        // Translates xaml_convert_error (the xaml_converters.hpp contract) into xaml_parse_exception
        // (the xaml_converter_registry's stored-converter contract), exactly as the standard table does.
        template <class T> [[nodiscard]] auto registry_converter(T (*convert)(std::string_view))
        {
            return [convert](const std::string& text) -> T {
                try
                {
                    return convert(text);
                }
                catch (const xaml_convert_error& err)
                {
                    throw xaml_parse_exception(err.what());
                }
            };
        }

        // ---- local numeric helpers (mirror the patterns in xaml_converters.cpp) ----

        [[noreturn]] void throw_cannot_convert_local(std::string_view text, std::string_view type_name)
        {
            throw xaml_convert_error(std::format("Cannot convert \"{}\" into {}", text, type_name));
        }

        template <class T> [[nodiscard]] bool try_parse_number(std::string_view s, T& out)
        {
            // Trim leading whitespace and a spurious '+'.
            const auto begin = s.find_first_not_of(" \t\n\v\f\r");
            if (begin == std::string_view::npos)
            {
                return false;
            }
            s = s.substr(begin);
            const auto end_ws = s.find_last_not_of(" \t\n\v\f\r");
            if (end_ws != std::string_view::npos)
            {
                s = s.substr(0, end_ws + 1);
            }
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
            // Floating T must use the portable shim — the Android NDK libc++ deletes FP from_chars.
            if constexpr (std::floating_point<T>)
            {
                const auto [ptr, ec] = maui::detail::from_chars_general(first, last, out);
                return ec == std::errc{} && ptr == last;
            }
            else
            {
                const auto [ptr, ec] = std::from_chars(first, last, out);
                return ec == std::errc{} && ptr == last;
            }
        }

        [[nodiscard]] constexpr char to_lower_ascii_local(char c) noexcept
        {
            return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
        }

        [[nodiscard]] bool equals_ignore_case_local(std::string_view a, std::string_view b) noexcept
        {
            if (a.size() != b.size())
            {
                return false;
            }
            for (std::size_t i = 0; i < a.size(); ++i)
            {
                if (to_lower_ascii_local(a[i]) != to_lower_ascii_local(b[i]))
                {
                    return false;
                }
            }
            return true;
        }

        // ---- text_input group ----

        // Microsoft.Maui.Controls.EditorAutoSizeOption — EditorAutoSizeOptionConverter.ConvertFrom
        // (TypeConversionExtensions generic Enum.Parse path; ignoreCase: false).
        // C# members: Disabled (0) / TextChanges (1).
        maui::controls::editor_auto_size_option convert_editor_auto_size_option(std::string_view text)
        {
            using maui::controls::editor_auto_size_option;
            static constexpr std::array<enum_entry<editor_auto_size_option>, 2> names{{
                {.name = "Disabled", .value = editor_auto_size_option::disabled},
                {.name = "TextChanges", .value = editor_auto_size_option::text_changes},
            }};
            return parse_enum<editor_auto_size_option>(text, names, "maui::controls::editor_auto_size_option");
        }

        // ---- scrolling_interactive group ----

        // Microsoft.Maui.ScrollOrientation — TypeConversionExtensions generic Enum.Parse path.
        // C# members: Vertical (0) / Horizontal / Both / Neither.
        maui::core::scroll_orientation convert_scroll_orientation(std::string_view text)
        {
            using maui::core::scroll_orientation;
            static constexpr std::array<enum_entry<scroll_orientation>, 4> names{{
                {.name = "Vertical", .value = scroll_orientation::vertical},
                {.name = "Horizontal", .value = scroll_orientation::horizontal},
                {.name = "Both", .value = scroll_orientation::both},
                {.name = "Neither", .value = scroll_orientation::neither},
            }};
            return parse_enum<scroll_orientation>(text, names, "maui::core::scroll_orientation");
        }

        // Microsoft.Maui.ScrollBarVisibility — TypeConversionExtensions generic Enum.Parse path.
        // C# members: Default (0) / Always / Never.  Port spells Default as default_ (C++ keyword).
        maui::core::scroll_bar_visibility convert_scroll_bar_visibility(std::string_view text)
        {
            using maui::core::scroll_bar_visibility;
            static constexpr std::array<enum_entry<scroll_bar_visibility>, 3> names{{
                {.name = "Default", .value = scroll_bar_visibility::default_},
                {.name = "Always", .value = scroll_bar_visibility::always},
                {.name = "Never", .value = scroll_bar_visibility::never},
            }};
            return parse_enum<scroll_bar_visibility>(text, names, "maui::core::scroll_bar_visibility");
        }

        // Microsoft.Maui.SwipeTransitionMode — TypeConversionExtensions generic Enum.Parse path.
        // C# members: Reveal (0) / Drag (1).
        maui::core::swipe_transition_mode convert_swipe_transition_mode(std::string_view text)
        {
            using maui::core::swipe_transition_mode;
            static constexpr std::array<enum_entry<swipe_transition_mode>, 2> names{{
                {.name = "Reveal", .value = swipe_transition_mode::reveal},
                {.name = "Drag", .value = swipe_transition_mode::drag},
            }};
            return parse_enum<swipe_transition_mode>(text, names, "maui::core::swipe_transition_mode");
        }

        // Microsoft.Maui.Controls.IndicatorShape — TypeConversionExtensions generic Enum.Parse path.
        // C# members: Circle (0) / Square.
        maui::controls::indicator_shape convert_indicator_shape(std::string_view text)
        {
            using maui::controls::indicator_shape;
            static constexpr std::array<enum_entry<indicator_shape>, 2> names{{
                {.name = "Circle", .value = indicator_shape::circle},
                {.name = "Square", .value = indicator_shape::square},
            }};
            return parse_enum<indicator_shape>(text, names, "maui::controls::indicator_shape");
        }

        // ---- shapes group ----

        // Microsoft.Maui.Graphics.LineCap — TypeConversionExtensions generic Enum.Parse path.
        // C# PenLineCap members: Flat (0) / Round (1) / Square (2).
        // The port's type is maui::graphics::line_cap with members butt/round/square (Flat maps to Butt
        // per C# PenLineCap.Flat == 0 == line_cap::butt).  Accept both "Flat" and "Butt" for value 0
        // so XAML written against PenLineCap and authors who use MAUI's documented "Butt" both work.
        maui::graphics::line_cap convert_line_cap(std::string_view text)
        {
            using maui::graphics::line_cap;
            static constexpr std::array<enum_entry<line_cap>, 4> names{{
                {.name = "Flat", .value = line_cap::butt}, // C# PenLineCap.Flat = 0 = butt
                {.name = "Butt", .value = line_cap::butt}, // explicit "Butt" alias
                {.name = "Round", .value = line_cap::round},
                {.name = "Square", .value = line_cap::square},
            }};
            return parse_enum<line_cap>(text, names, "maui::graphics::line_cap");
        }

        // Microsoft.Maui.Graphics.LineJoin — TypeConversionExtensions generic Enum.Parse path.
        // C# PenLineJoin members: Miter (0) / Round (1) / Bevel (2).
        maui::graphics::line_join convert_line_join(std::string_view text)
        {
            using maui::graphics::line_join;
            static constexpr std::array<enum_entry<line_join>, 3> names{{
                {.name = "Miter", .value = line_join::miter},
                {.name = "Round", .value = line_join::round},
                {.name = "Bevel", .value = line_join::bevel},
            }};
            return parse_enum<line_join>(text, names, "maui::graphics::line_join");
        }

        // Microsoft.Maui.PathAspect / Shape.cs Stretch -> IShapeView.Aspect mapping.
        // In XAML the Aspect attribute on shape views uses the C# Stretch enum names:
        //   None (0)        -> path_aspect::none
        //   Center          -> path_aspect::center   (not a standard Stretch member, port extension)
        //   Fill            -> path_aspect::stretch  (Stretch.Fill — stretch ignoring aspect ratio)
        //   Uniform         -> path_aspect::aspect_fit   (Stretch.Uniform — preserve aspect, letterbox)
        //   UniformToFill   -> path_aspect::aspect_fill  (Stretch.UniformToFill — preserve aspect, clip)
        // "Center" is the port extension value; it appears in the port's path_aspect enum but has no
        // C# Stretch counterpart — accept it by name for completeness.
        maui::core::path_aspect convert_path_aspect(std::string_view text)
        {
            using maui::core::path_aspect;
            static constexpr std::array<enum_entry<path_aspect>, 5> names{{
                {.name = "None", .value = path_aspect::none},
                {.name = "Center", .value = path_aspect::center},
                {.name = "Fill", .value = path_aspect::stretch},
                {.name = "Uniform", .value = path_aspect::aspect_fit},
                {.name = "UniformToFill", .value = path_aspect::aspect_fill},
            }};
            return parse_enum<path_aspect>(text, names, "maui::core::path_aspect");
        }

        // Microsoft.Maui.Controls.Shapes.DoubleCollectionConverter.ConvertFrom — StrokeDashArray.
        // Splits on ',' (XAML) or ' ' (CSS) and parses each token as a double.  Empty string yields
        // an empty collection (the C# fast path).
        std::vector<double> convert_stroke_dash_array(std::string_view text)
        {
            const auto ws_begin = text.find_first_not_of(" \t\n\v\f\r");
            if (ws_begin == std::string_view::npos)
            {
                return {};
            }
            const auto ws_end = text.find_last_not_of(" \t\n\v\f\r");
            std::string_view value = text.substr(ws_begin, ws_end - ws_begin + 1);

            // Choose delimiter: ',' takes priority (XAML style) when present, else ' ' (CSS style).
            const char delimiter = value.contains(',') ? ',' : ' ';

            std::vector<double> result;
            std::size_t begin = 0;
            while (true)
            {
                const auto pos = value.find(delimiter, begin);
                const std::string_view token =
                    value.substr(begin, pos == std::string_view::npos ? std::string_view::npos : pos - begin);
                double d = 0;
                if (!try_parse_number(token, d))
                {
                    throw_cannot_convert_local(token, "double (stroke dash array element)");
                }
                result.push_back(d);
                if (pos == std::string_view::npos)
                {
                    break;
                }
                begin = pos + 1;
            }
            return result;
        }

        // Microsoft.Maui.Controls.Shapes.PointCollectionConverter.ConvertFrom — Polygon/Polyline Points.
        // Parses a space-or-comma-separated sequence of "x,y" coordinate pairs.
        // Accepted forms: "10,100 60,100 35,0" (XAML pairs) and "10 100 60 100 35 0" (flat doubles).
        // The C# PointCollectionConverter splits on whitespace, then on ',' to get individual x and y.
        maui::controls::shapes::point_collection convert_point_collection(std::string_view text)
        {
            // Tokenise on whitespace and commas simultaneously.  Strategy: normalise to a flat
            // sequence of numeric tokens (replace ',' with ' ', then split on ' ').
            const auto ws_begin = text.find_first_not_of(" \t\n\v\f\r");
            if (ws_begin == std::string_view::npos)
            {
                return {};
            }
            // Build a mutable copy with commas replaced by spaces so we can split once.
            std::string normalised{text};
            for (char& c : normalised)
            {
                if (c == ',')
                {
                    c = ' ';
                }
            }

            std::vector<double> coords;
            std::size_t pos = 0;
            while (pos < normalised.size())
            {
                // Skip whitespace
                while (pos < normalised.size() &&
                       (normalised[pos] == ' ' || normalised[pos] == '\t' || normalised[pos] == '\n' ||
                        normalised[pos] == '\v' || normalised[pos] == '\f' || normalised[pos] == '\r'))
                {
                    ++pos;
                }
                if (pos >= normalised.size())
                {
                    break;
                }
                // Find end of token
                const std::size_t token_start = pos;
                while (pos < normalised.size() && normalised[pos] != ' ' && normalised[pos] != '\t' &&
                       normalised[pos] != '\n' && normalised[pos] != '\v' && normalised[pos] != '\f' &&
                       normalised[pos] != '\r')
                {
                    ++pos;
                }
                const std::string_view token{normalised.data() + token_start, pos - token_start};
                double d = 0;
                if (!try_parse_number(token, d))
                {
                    throw_cannot_convert_local(token, "double (point collection coordinate)");
                }
                coords.push_back(d);
            }

            if (coords.size() % 2 != 0)
            {
                throw xaml_convert_error(
                    std::format("Cannot convert \"{}\" into point_collection: odd number of coordinates ({})", text,
                                coords.size()));
            }

            maui::controls::shapes::point_collection points;
            points.reserve(coords.size() / 2);
            for (std::size_t i = 0; i < coords.size(); i += 2)
            {
                points.push_back({coords[i], coords[i + 1]});
            }
            return points;
        }

        // Microsoft.Maui.Controls.Shapes.FillRule — TypeConversionExtensions generic Enum.Parse path.
        // C# members: EvenOdd (0) / Nonzero.
        maui::controls::shapes::fill_rule convert_fill_rule(std::string_view text)
        {
            using maui::controls::shapes::fill_rule;
            static constexpr std::array<enum_entry<fill_rule>, 2> names{{
                {.name = "EvenOdd", .value = fill_rule::even_odd},
                {.name = "Nonzero", .value = fill_rule::nonzero},
            }};
            return parse_enum<fill_rule>(text, names, "maui::controls::shapes::fill_rule");
        }

        // ---- layouts group ----

        // Microsoft.Maui.FlexPosition — FlexEnumsConverters, case-INsensitive Enum.TryParse.
        // C# members: Relative (0) / Absolute (1).
        maui::layouts::flex_position convert_flex_position(std::string_view text)
        {
            using maui::layouts::flex_position;
            static constexpr std::array<enum_entry<flex_position>, 2> names{{
                {.name = "Relative", .value = flex_position::relative},
                {.name = "Absolute", .value = flex_position::absolute},
            }};
            // FlexEnumsConverters uses case-insensitive Enum.TryParse (ignoreCase: true).
            const std::string_view trimmed = detail::trim(text);
            for (const auto& entry : names)
            {
                if (equals_ignore_case_local(entry.name, trimmed))
                {
                    return entry.value;
                }
            }
            long long numeric = 0;
            if (detail::try_parse_enum_number(trimmed, numeric))
            {
                for (const auto& entry : names)
                {
                    if (static_cast<long long>(entry.value) == numeric)
                    {
                        return entry.value;
                    }
                }
            }
            throw_cannot_convert_local(text, "maui::layouts::flex_position");
        }

        // Microsoft.Maui.Layouts.AbsoluteLayoutFlags — TypeConversionExtensions generic Enum.Parse path.
        // [Flags] enum: a single name or a comma-separated list of names OR'd together.
        // C# member names: None / XProportional / YProportional / WidthProportional / HeightProportional /
        //                  PositionProportional / SizeProportional / All.
        maui::layouts::absolute_layout_flags convert_absolute_layout_flags(std::string_view text)
        {
            using maui::layouts::absolute_layout_flags;
            static constexpr std::array<enum_entry<absolute_layout_flags>, 8> names{{
                {.name = "None", .value = absolute_layout_flags::none},
                {.name = "XProportional", .value = absolute_layout_flags::x_proportional},
                {.name = "YProportional", .value = absolute_layout_flags::y_proportional},
                {.name = "WidthProportional", .value = absolute_layout_flags::width_proportional},
                {.name = "HeightProportional", .value = absolute_layout_flags::height_proportional},
                {.name = "PositionProportional", .value = absolute_layout_flags::position_proportional},
                {.name = "SizeProportional", .value = absolute_layout_flags::size_proportional},
                {.name = "All", .value = absolute_layout_flags::all},
            }};

            auto result = absolute_layout_flags::none;
            // Split on ',' — the [Flags] enum.Parse separates parts by comma.
            std::size_t begin = 0;
            while (true)
            {
                const auto pos = text.find(',', begin);
                const std::string_view part_raw =
                    text.substr(begin, pos == std::string_view::npos ? std::string_view::npos : pos - begin);
                const std::string_view part = detail::trim(part_raw);
                if (const auto parsed = try_parse_enum<absolute_layout_flags>(part, names))
                {
                    result = result | *parsed;
                }
                else
                {
                    throw_cannot_convert_local(part, "maui::layouts::absolute_layout_flags");
                }
                if (pos == std::string_view::npos)
                {
                    break;
                }
                begin = pos + 1;
            }
            return result;
        }

        // ---- pages group ----

        // Microsoft.Maui.Controls.FlyoutLayoutBehavior — TypeConversionExtensions generic Enum.Parse.
        // C# members: Default (0) / SplitOnLandscape (1) / Split (2) / Popover (3) / SplitOnPortrait (4).
        // Port maps Default to default_ (C++ keyword avoidance).
        maui::controls::flyout_layout_behavior convert_flyout_layout_behavior(std::string_view text)
        {
            using maui::controls::flyout_layout_behavior;
            static constexpr std::array<enum_entry<flyout_layout_behavior>, 5> names{{
                {.name = "Default", .value = flyout_layout_behavior::default_},
                {.name = "SplitOnLandscape", .value = flyout_layout_behavior::split_on_landscape},
                {.name = "Split", .value = flyout_layout_behavior::split},
                {.name = "Popover", .value = flyout_layout_behavior::popover},
                {.name = "SplitOnPortrait", .value = flyout_layout_behavior::split_on_portrait},
            }};
            return parse_enum<flyout_layout_behavior>(text, names, "maui::controls::flyout_layout_behavior");
        }

        // Microsoft.Maui.FlyoutBehavior — TypeConversionExtensions generic Enum.Parse path.
        // C# members: Disabled (0) / Flyout (1) / Locked (2).
        maui::controls::flyout_behavior convert_flyout_behavior(std::string_view text)
        {
            using maui::controls::flyout_behavior;
            static constexpr std::array<enum_entry<flyout_behavior>, 3> names{{
                {.name = "Disabled", .value = flyout_behavior::disabled},
                {.name = "Flyout", .value = flyout_behavior::flyout},
                {.name = "Locked", .value = flyout_behavior::locked},
            }};
            return parse_enum<flyout_behavior>(text, names, "maui::controls::flyout_behavior");
        }

        // Microsoft.Maui.Controls.FlyoutHeaderBehavior — TypeConversionExtensions generic Enum.Parse.
        // C# members: Default (0) / Fixed (1) / Scroll (2) / CollapseOnScroll (3).
        // Port uses default_behavior / fixed_behavior (C++ keyword / clash avoidance).
        maui::controls::flyout_header_behavior convert_flyout_header_behavior(std::string_view text)
        {
            using maui::controls::flyout_header_behavior;
            static constexpr std::array<enum_entry<flyout_header_behavior>, 4> names{{
                {.name = "Default", .value = flyout_header_behavior::default_behavior},
                {.name = "Fixed", .value = flyout_header_behavior::fixed_behavior},
                {.name = "Scroll", .value = flyout_header_behavior::scroll},
                {.name = "CollapseOnScroll", .value = flyout_header_behavior::collapse_on_scroll},
            }};
            return parse_enum<flyout_header_behavior>(text, names, "maui::controls::flyout_header_behavior");
        }

        // ---- pickers group (date/time value types) ----

        // maui::core::date_time from a XAML attribute string.  The DatePicker's Date/MinimumDate/
        // MaximumDate properties are typed std::optional<date_time>; the optional wrapper is
        // registered below.  This function ports the C# DateTime.Parse invariant-culture path the
        // DatePicker validates against — accepting the ISO 8601 date form (YYYY-MM-DD) and the default
        // DatePicker format "M/d/yyyy" (e.g. "1/27/2024").  Both are the forms MAUI XAML authors
        // actually write.
        maui::core::date_time convert_date_time(std::string_view text)
        {
            const std::string_view v = detail::trim(text);
            // Try "YYYY-MM-DD" (ISO 8601).
            if (v.size() == 10 && v[4] == '-' && v[7] == '-')
            {
                int year = 0;
                unsigned month = 0;
                unsigned day = 0;
                if (try_parse_number(v.substr(0, 4), year) && try_parse_number(v.substr(5, 2), month) &&
                    try_parse_number(v.substr(8, 2), day))
                {
                    return maui::core::date_time{year, month, day};
                }
            }
            // Try "M/d/yyyy" (DatePicker default format "d", en-US short date).
            {
                const auto slash1 = v.find('/');
                if (slash1 != std::string_view::npos)
                {
                    const auto slash2 = v.find('/', slash1 + 1);
                    if (slash2 != std::string_view::npos)
                    {
                        unsigned month = 0;
                        unsigned day = 0;
                        int year = 0;
                        if (try_parse_number(v.substr(0, slash1), month) &&
                            try_parse_number(v.substr(slash1 + 1, slash2 - slash1 - 1), day) &&
                            try_parse_number(v.substr(slash2 + 1), year))
                        {
                            return maui::core::date_time{year, month, day};
                        }
                    }
                }
            }
            throw_cannot_convert_local(text, "maui::core::date_time");
        }

        // std::optional<maui::core::date_time>: the type used by DatePicker date properties.
        // An empty / whitespace string maps to std::nullopt (no date set).
        std::optional<maui::core::date_time> convert_optional_date_time(std::string_view text)
        {
            if (detail::trim(text).empty())
            {
                return std::nullopt;
            }
            return convert_date_time(text);
        }

        // maui::core::time_span from a XAML attribute string.  The TimePicker's Time property is typed
        // std::optional<time_span>.  Parses "h:mm:ss" / "hh:mm:ss" (the standard TimeSpan string form
        // that .NET DateTime.ToString("t"/"T") and MAUI XAML authors use).
        maui::core::time_span convert_time_span(std::string_view text)
        {
            const std::string_view v = detail::trim(text);
            // Parse "H:MM:SS" or "HH:MM:SS" — allow arbitrary-width hour field.
            const auto colon1 = v.find(':');
            if (colon1 != std::string_view::npos)
            {
                const auto colon2 = v.find(':', colon1 + 1);
                if (colon2 != std::string_view::npos)
                {
                    int hours = 0;
                    int minutes = 0;
                    int seconds = 0;
                    if (try_parse_number(v.substr(0, colon1), hours) &&
                        try_parse_number(v.substr(colon1 + 1, colon2 - colon1 - 1), minutes) &&
                        try_parse_number(v.substr(colon2 + 1), seconds))
                    {
                        return maui::core::time_span{hours, minutes, seconds};
                    }
                }
            }
            throw_cannot_convert_local(text, "maui::core::time_span");
        }

        // std::optional<maui::core::time_span>: the type used by TimePicker's Time property.
        std::optional<maui::core::time_span> convert_optional_time_span(std::string_view text)
        {
            if (detail::trim(text).empty())
            {
                return std::nullopt;
            }
            return convert_time_span(text);
        }

    } // anonymous namespace

    void register_xaml_extra_converters(xaml_converter_registry& converters)
    {
        // ---- text_input group ----
        // maui::controls::editor_auto_size_option  (Editor.AutoSize)
        converters.register_converter<maui::controls::editor_auto_size_option>(
            registry_converter(&convert_editor_auto_size_option));

        // ---- scrolling_interactive group ----
        // maui::core::scroll_orientation  (ScrollView.Orientation)
        converters.register_converter<maui::core::scroll_orientation>(registry_converter(&convert_scroll_orientation));
        // maui::core::scroll_bar_visibility  (ScrollView.HorizontalScrollBarVisibility /
        //                                     ScrollView.VerticalScrollBarVisibility)
        converters.register_converter<maui::core::scroll_bar_visibility>(
            registry_converter(&convert_scroll_bar_visibility));
        // maui::core::swipe_transition_mode  (SwipeView.SwipeTransitionMode)
        converters.register_converter<maui::core::swipe_transition_mode>(
            registry_converter(&convert_swipe_transition_mode));
        // maui::controls::indicator_shape  (IndicatorView.IndicatorsShape)
        converters.register_converter<maui::controls::indicator_shape>(registry_converter(&convert_indicator_shape));

        // ---- shapes group ----
        // maui::graphics::line_cap  (shape StrokeLineCap)
        converters.register_converter<maui::graphics::line_cap>(registry_converter(&convert_line_cap));
        // maui::graphics::line_join  (shape StrokeLineJoin)
        converters.register_converter<maui::graphics::line_join>(registry_converter(&convert_line_join));
        // maui::core::path_aspect  (shape Aspect)
        converters.register_converter<maui::core::path_aspect>(registry_converter(&convert_path_aspect));
        // std::vector<double>  (shape StrokeDashArray / Border.StrokeDashArray)
        converters.register_converter<std::vector<double>>(registry_converter(&convert_stroke_dash_array));
        // maui::controls::shapes::point_collection  (Polygon/Polyline Points)
        converters.register_converter<maui::controls::shapes::point_collection>(
            registry_converter(&convert_point_collection));
        // maui::controls::shapes::fill_rule  (Polygon/Polyline FillRule)
        converters.register_converter<maui::controls::shapes::fill_rule>(registry_converter(&convert_fill_rule));

        // ---- layouts group ----
        // maui::layouts::flex_position  (FlexLayout.Position)
        converters.register_converter<maui::layouts::flex_position>(registry_converter(&convert_flex_position));
        // maui::layouts::absolute_layout_flags  (AbsoluteLayout.LayoutFlags attached property)
        converters.register_converter<maui::layouts::absolute_layout_flags>(
            registry_converter(&convert_absolute_layout_flags));

        // ---- pages group ----
        // maui::controls::flyout_layout_behavior  (FlyoutPage.FlyoutLayoutBehavior)
        converters.register_converter<maui::controls::flyout_layout_behavior>(
            registry_converter(&convert_flyout_layout_behavior));
        // maui::controls::flyout_behavior  (Shell.FlyoutBehavior)
        converters.register_converter<maui::controls::flyout_behavior>(registry_converter(&convert_flyout_behavior));
        // maui::controls::flyout_header_behavior  (Shell.FlyoutHeaderBehavior)
        converters.register_converter<maui::controls::flyout_header_behavior>(
            registry_converter(&convert_flyout_header_behavior));

        // ---- pickers group (date/time value types) ----
        // maui::core::date_time  — the non-optional form (useful for direct use / testing)
        converters.register_converter<maui::core::date_time>(registry_converter(&convert_date_time));
        // std::optional<maui::core::date_time>  (DatePicker.Date / MinimumDate / MaximumDate)
        converters.register_converter<std::optional<maui::core::date_time>>(
            registry_converter(&convert_optional_date_time));
        // maui::core::time_span  — the non-optional form
        converters.register_converter<maui::core::time_span>(registry_converter(&convert_time_span));
        // std::optional<maui::core::time_span>  (TimePicker.Time)
        converters.register_converter<std::optional<maui::core::time_span>>(
            registry_converter(&convert_optional_time_span));
    }

} // namespace maui::xaml
