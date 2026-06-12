// Tests for the maui::xaml string -> value converters. Ported from the C# converter suites:
//   src/Controls/tests/Core.UnitTests/ThicknessTests.cs (TestThicknessTypeConverter + Doubles)
//   src/Controls/tests/Core.UnitTests/GridLengthTypeConverterTests.cs
//   src/Controls/tests/Core.UnitTests/ColumnDefinitionCollectionTypeConverterUnitTests.cs
//   src/Controls/tests/Core.UnitTests/LayoutOptionsUnitTests.cs (TestTypeConverter)
//   src/Graphics/tests/Graphics.Tests/ColorTypeConverterTests.cs (delegation; formats are covered
//       by tests/graphics/color_converter_tests.cpp — here we add the converter's error behavior)
// plus cases derived directly from the converter sources where C# has no unit test:
//   ThicknessTypeConverter/CornerRadiusTypeConverter CSS branches, FontSizeConverter,
//   FlowDirectionConverter, VisualElement.VisibilityConverter, TypeConversionExtensions built-ins,
//   Point/Rect/Size/SizeF.TryParse. Invalid inputs assert the single M7 error channel
//   (xaml_convert_error replaces C#'s InvalidOperationException / FormatException).

#include "maui/xaml/xaml_converters.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

// row/column_definition are reached through xaml_converters.hpp's return types (the .height()/.width()
// accessors below); include-cleaner treats their headers as indirect, so they are not re-included here.
#include "maui/animations/easing.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/clear_button_visibility.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/grid_unit_type.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/return_type.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/text_decorations.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/size_f.hpp"

namespace
{
    namespace xaml = maui::xaml;
    namespace colors = maui::graphics::colors;
    using maui::core::grid_length;
    using maui::core::grid_unit_type;
    using maui::core::layout_alignment;
    using maui::core::thickness;
    using maui::graphics::color;
    using maui::graphics::corner_radius;
    using maui::xaml::xaml_convert_error;

    // ---- color (ColorTypeConverter -> Color.Parse; formats already covered at the graphics layer) ----

    TEST(xaml_convert_color, parses_named_hex_and_functional_forms)
    {
        EXPECT_EQ(xaml::convert_color("Black"), color());
        EXPECT_EQ(xaml::convert_color("#512BD4"), color::from_argb("#512BD4"));
        EXPECT_EQ(xaml::convert_color("#F00"), color::from_argb("#F00")); // short #RGB form
        EXPECT_EQ(xaml::convert_color("#ff000000"), color());
        EXPECT_EQ(xaml::convert_color("rgb(81,43,212)"), color::from_argb("#512BD4"));
        EXPECT_EQ(xaml::convert_color("rgba(0,0,0,0)"), colors::transparent);
        EXPECT_EQ(xaml::convert_color("hsl(253,66,50)"), color::from_argb("#4F2BD3"));
        EXPECT_EQ(xaml::convert_color("hsla(0,0,0,1)"), color());
    }

    TEST(xaml_convert_color, invalid_throws)
    {
        // C#: Color.Parse -> InvalidOperationException.
        EXPECT_THROW((void)xaml::convert_color("not-a-color"), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_color(""), xaml_convert_error);
    }

    // ---- thickness (ThicknessTypeConverter) ----

    // C# ThicknessTests.TestThicknessTypeConverter.
    TEST(xaml_convert_thickness, type_converter_cases)
    {
        EXPECT_EQ(xaml::convert_thickness("1"), thickness(1));
        EXPECT_EQ(xaml::convert_thickness("1, 2"), thickness(1, 2));
        EXPECT_EQ(xaml::convert_thickness("1, 2, 3, 4"), thickness(1, 2, 3, 4));
        EXPECT_EQ(xaml::convert_thickness("1.1,2"), thickness(1.1, 2));
        EXPECT_THROW((void)xaml::convert_thickness(""), xaml_convert_error);
    }

    // C# ThicknessTests.ThicknessTypeConverterDoubles.
    TEST(xaml_convert_thickness, doubles)
    {
        EXPECT_EQ(xaml::convert_thickness("1.3"), thickness(1.3));
        EXPECT_EQ(xaml::convert_thickness("1.4, 2.8"), thickness(1.4, 2.8));
        EXPECT_EQ(xaml::convert_thickness(" 1.6 , 2.1, 3.8, 4.2"), thickness(1.6, 2.1, 3.8, 4.2));
    }

    // Derived from ThicknessTypeConverter.cs's CSS (space-separated) branch.
    TEST(xaml_convert_thickness, css_space_forms)
    {
        EXPECT_EQ(xaml::convert_thickness("1 2"), thickness(2, 1));           // "v h" -> Thickness(h, v)
        EXPECT_EQ(xaml::convert_thickness("1 2 3"), thickness(2, 1, 2, 3));   // "t h b" -> (h, t, h, b)
        EXPECT_EQ(xaml::convert_thickness("1 2 3 4"), thickness(4, 1, 2, 3)); // "t r b l" -> (l, t, r, b)
    }

    TEST(xaml_convert_thickness, invalid_throws)
    {
        EXPECT_THROW((void)xaml::convert_thickness("foo"), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_thickness("1,2,3"), xaml_convert_error); // 3 comma parts: no case
        EXPECT_THROW((void)xaml::convert_thickness("1,2,3,4,5"), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_thickness("1,a"), xaml_convert_error);
        // C# Split(' ') keeps empty entries: "1  2" is THREE parts with an empty middle -> throw.
        EXPECT_THROW((void)xaml::convert_thickness("1  2"), xaml_convert_error);
    }

    // ---- corner radius (CornerRadiusTypeConverter; cases derived from the converter source) ----

    TEST(xaml_convert_corner_radius, uniform_and_four_values)
    {
        EXPECT_EQ(xaml::convert_corner_radius("1"), corner_radius(1));
        EXPECT_EQ(xaml::convert_corner_radius(" 2.5 "), corner_radius(2.5));
        EXPECT_EQ(xaml::convert_corner_radius("1, 2, 3, 4"), corner_radius(1, 2, 3, 4)); // tl,tr,bl,br
    }

    TEST(xaml_convert_corner_radius, two_or_three_comma_values_collapse_to_uniform_first)
    {
        // C# quirk (kept): 1 < parts < 4 -> uniform CornerRadius from the FIRST value only.
        EXPECT_EQ(xaml::convert_corner_radius("1,2"), corner_radius(1));
        EXPECT_EQ(xaml::convert_corner_radius("1,2,3"), corner_radius(1));
        // ...so the unparsed tail is never validated:
        EXPECT_EQ(xaml::convert_corner_radius("1,x"), corner_radius(1));
    }

    TEST(xaml_convert_corner_radius, css_space_forms)
    {
        EXPECT_EQ(xaml::convert_corner_radius("1 2"), corner_radius(1, 2, 2, 1));   // (t, b, b, t)
        EXPECT_EQ(xaml::convert_corner_radius("1 2 3"), corner_radius(1, 2, 2, 3)); // (tl, trbl, trbl, br)
        EXPECT_EQ(xaml::convert_corner_radius("1 2 3 4"), corner_radius(1, 2, 3, 4));
    }

    TEST(xaml_convert_corner_radius, invalid_throws)
    {
        EXPECT_THROW((void)xaml::convert_corner_radius(""), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_corner_radius("x"), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_corner_radius("1,2,3,4,5"), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_corner_radius("x,2"), xaml_convert_error); // first must parse
        EXPECT_THROW((void)xaml::convert_corner_radius("1 x"), xaml_convert_error); // CSS: all must parse
    }

    // ---- grid length (GridLengthTypeConverterTests.cs) ----

    TEST(xaml_convert_grid_length, absolute)
    {
        EXPECT_EQ(xaml::convert_grid_length("42"), grid_length(42));
        EXPECT_EQ(xaml::convert_grid_length("42.2"), grid_length(42.2));
        // C#: FormatException.
        EXPECT_THROW((void)xaml::convert_grid_length("foo"), xaml_convert_error);
    }

    TEST(xaml_convert_grid_length, auto_keyword)
    {
        EXPECT_EQ(xaml::convert_grid_length("auto"), grid_length::automatic());
        EXPECT_EQ(xaml::convert_grid_length(" AuTo "), grid_length::automatic());
    }

    TEST(xaml_convert_grid_length, star)
    {
        EXPECT_EQ(xaml::convert_grid_length("*"), grid_length(1, grid_unit_type::star));
        EXPECT_EQ(xaml::convert_grid_length("42*"), grid_length(42, grid_unit_type::star));
    }

    TEST(xaml_convert_grid_length, value)
    {
        EXPECT_EQ(xaml::convert_grid_length("3.3"), grid_length(3.3));
    }

    TEST(xaml_convert_grid_length, value_star)
    {
        EXPECT_EQ(xaml::convert_grid_length("32.3*"), grid_length(32.3, grid_unit_type::star));
    }

    TEST(xaml_convert_grid_length, invalid_throws)
    {
        EXPECT_THROW((void)xaml::convert_grid_length(""), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_grid_length("**"), xaml_convert_error);
        // The grid_length ctor's own validation (C# ArgumentException for negatives) is folded into
        // the single M7 error channel.
        EXPECT_THROW((void)xaml::convert_grid_length("-5"), xaml_convert_error);
    }

    // ---- row / column definition collections ----

    TEST(xaml_convert_definitions, row_definitions_parse_each_length)
    {
        const auto rows = xaml::convert_row_definitions("auto,*,2*,100");
        ASSERT_EQ(rows.size(), 4U);
        EXPECT_EQ(rows[0].height(), grid_length::automatic());
        EXPECT_EQ(rows[1].height(), grid_length::star());
        EXPECT_EQ(rows[2].height(), grid_length(2, grid_unit_type::star));
        EXPECT_EQ(rows[3].height(), grid_length(100));
    }

    TEST(xaml_convert_definitions, column_definitions_parse_each_length)
    {
        const auto columns = xaml::convert_column_definitions("auto, *, 2*, 100");
        ASSERT_EQ(columns.size(), 4U);
        EXPECT_EQ(columns[0].width(), grid_length::automatic());
        EXPECT_EQ(columns[1].width(), grid_length::star());
        EXPECT_EQ(columns[2].width(), grid_length(2, grid_unit_type::star));
        EXPECT_EQ(columns[3].width(), grid_length(100));
    }

    TEST(xaml_convert_definitions, empty_string_yields_empty_collection)
    {
        // The C# fast path. (C# null instead throws InvalidOperationException —
        // ColumnDefinitionCollectionTypeConverterUnitTests.ConvertNullTest — but a string_view
        // cannot be null; see the header deviations.)
        EXPECT_TRUE(xaml::convert_row_definitions("").empty());
        EXPECT_TRUE(xaml::convert_column_definitions("").empty());
    }

    TEST(xaml_convert_definitions, single_definition)
    {
        const auto rows = xaml::convert_row_definitions("*");
        ASSERT_EQ(rows.size(), 1U);
        EXPECT_EQ(rows[0].height(), grid_length::star());
    }

    TEST(xaml_convert_definitions, invalid_entry_throws)
    {
        EXPECT_THROW((void)xaml::convert_row_definitions("auto,,100"), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_column_definitions("foo"), xaml_convert_error);
    }

    // ---- layout alignment (LayoutOptionsUnitTests.TestTypeConverter) ----

    TEST(xaml_convert_layout_alignment, names_and_qualified_names)
    {
        EXPECT_EQ(xaml::convert_layout_alignment("LayoutOptions.Center"), layout_alignment::center);
        EXPECT_EQ(xaml::convert_layout_alignment("Center"), layout_alignment::center);
        EXPECT_EQ(xaml::convert_layout_alignment("Start"), layout_alignment::start);
        EXPECT_EQ(xaml::convert_layout_alignment("End"), layout_alignment::end);
        EXPECT_EQ(xaml::convert_layout_alignment("Fill"), layout_alignment::fill);
    }

    TEST(xaml_convert_layout_alignment, legacy_and_expand_names_map_to_base_alignment)
    {
        // C# LayoutOptions.<X>AndExpand has Alignment == <X> (the port drops the Expands flag —
        // see the header deviations; C#'s NotEqual(CenterAndExpand, Center) is the flag, not the
        // alignment).
        EXPECT_EQ(xaml::convert_layout_alignment("StartAndExpand"), layout_alignment::start);
        EXPECT_EQ(xaml::convert_layout_alignment("CenterAndExpand"), layout_alignment::center);
        EXPECT_EQ(xaml::convert_layout_alignment("EndAndExpand"), layout_alignment::end);
        EXPECT_EQ(xaml::convert_layout_alignment("FillAndExpand"), layout_alignment::fill);
        EXPECT_EQ(xaml::convert_layout_alignment("LayoutOptions.FillAndExpand"), layout_alignment::fill);
    }

    TEST(xaml_convert_layout_alignment, invalid_throws)
    {
        EXPECT_THROW((void)xaml::convert_layout_alignment("foo"), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_layout_alignment("foo.bar"), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_layout_alignment("foo.bar.baz"), xaml_convert_error);
        // The C# converter never trims its input.
        EXPECT_THROW((void)xaml::convert_layout_alignment(" Center"), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_layout_alignment("LayoutOptions."), xaml_convert_error);
    }

    // ---- font size (FontSizeConverter) ----

    TEST(xaml_convert_font_size, numeric)
    {
        EXPECT_EQ(xaml::convert_font_size("14"), 14.0);
        EXPECT_EQ(xaml::convert_font_size("14.5"), 14.5);
        EXPECT_EQ(xaml::convert_font_size(" 11 "), 11.0);
    }

    TEST(xaml_convert_font_size, named_sizes_resolve_via_apple_table)
    {
        // Device.GetNamedSize(named, typeof(Label), false) with the Apple FontNamedSizeService's
        // AppKit-branch constants (see the header).
        EXPECT_EQ(xaml::convert_font_size("Default"), 17.0);
        EXPECT_EQ(xaml::convert_font_size("Micro"), 12.0);
        EXPECT_EQ(xaml::convert_font_size("Small"), 14.0);
        EXPECT_EQ(xaml::convert_font_size("Medium"), 17.0);
        EXPECT_EQ(xaml::convert_font_size("Large"), 22.0);
        EXPECT_EQ(xaml::convert_font_size("Body"), 23.0);
        EXPECT_EQ(xaml::convert_font_size("Caption"), 18.0);
        EXPECT_EQ(xaml::convert_font_size("Header"), 23.0);
        EXPECT_EQ(xaml::convert_font_size("Subtitle"), 28.0);
        EXPECT_EQ(xaml::convert_font_size("Title"), 34.0);
    }

    TEST(xaml_convert_font_size, invalid_throws)
    {
        // The C# name comparisons are Ordinal (case-sensitive).
        EXPECT_THROW((void)xaml::convert_font_size("small"), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_font_size("foo"), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_font_size(""), xaml_convert_error);
    }

    // ---- point / rect / size / size_f (Graphics converters over the value types' TryParse) ----

    TEST(xaml_convert_point, parses_x_comma_y)
    {
        EXPECT_EQ(xaml::convert_point("1,2"), maui::graphics::point(1, 2));
        EXPECT_EQ(xaml::convert_point("1.5, 2.5"), maui::graphics::point(1.5, 2.5));
        EXPECT_THROW((void)xaml::convert_point("1"), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_point("1,2,3"), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_point(""), xaml_convert_error);
    }

    TEST(xaml_convert_rect, parses_x_y_w_h)
    {
        EXPECT_EQ(xaml::convert_rect("1,2,3,4"), maui::graphics::rect(1, 2, 3, 4));
        EXPECT_EQ(xaml::convert_rect("0, 0, 100.5, 200"), maui::graphics::rect(0, 0, 100.5, 200));
        EXPECT_THROW((void)xaml::convert_rect("1,2,3"), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_rect("a,b,c,d"), xaml_convert_error);
    }

    TEST(xaml_convert_size, parses_w_comma_h)
    {
        EXPECT_EQ(xaml::convert_size("10,20"), maui::graphics::size(10, 20));
        EXPECT_EQ(xaml::convert_size_f("1.5, 2.5"), maui::graphics::size_f(1.5F, 2.5F));
        EXPECT_THROW((void)xaml::convert_size("10"), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_size_f("x,y"), xaml_convert_error);
    }

    // ---- is-visible (VisualElement.VisibilityConverter: string -> bool) ----

    TEST(xaml_convert_is_visible, true_and_visible_aliases)
    {
        EXPECT_TRUE(xaml::convert_is_visible("true"));
        EXPECT_TRUE(xaml::convert_is_visible("True"));
        EXPECT_TRUE(xaml::convert_is_visible("visible"));
        EXPECT_TRUE(xaml::convert_is_visible(" Visible "));
    }

    TEST(xaml_convert_is_visible, false_hidden_and_collapse_aliases)
    {
        EXPECT_FALSE(xaml::convert_is_visible("false"));
        EXPECT_FALSE(xaml::convert_is_visible("HIDDEN"));
        EXPECT_FALSE(xaml::convert_is_visible("collapse"));
    }

    TEST(xaml_convert_is_visible, invalid_throws)
    {
        // C# matches "collapse" exactly — "collapsed" is NOT accepted.
        EXPECT_THROW((void)xaml::convert_is_visible("collapsed"), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_is_visible(""), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_is_visible("yes"), xaml_convert_error);
    }

    // ---- built-in passthroughs (TypeConversionExtensions.ConvertTo) ----

    TEST(xaml_convert_bool, boolean_parse_semantics)
    {
        EXPECT_TRUE(xaml::convert_bool("true"));
        EXPECT_TRUE(xaml::convert_bool("True"));
        EXPECT_TRUE(xaml::convert_bool(" TRUE "));
        EXPECT_FALSE(xaml::convert_bool("false"));
        EXPECT_FALSE(xaml::convert_bool("False"));
        EXPECT_THROW((void)xaml::convert_bool("1"), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_bool(""), xaml_convert_error);
    }

    TEST(xaml_convert_double, double_parse_semantics)
    {
        EXPECT_EQ(xaml::convert_double("1.5"), 1.5);
        EXPECT_EQ(xaml::convert_double("-2"), -2.0);
        EXPECT_EQ(xaml::convert_double(" 3 "), 3.0);
        EXPECT_EQ(xaml::convert_double("+4"), 4.0);
        EXPECT_EQ(xaml::convert_double("1e3"), 1000.0); // Double.Parse allows exponents
        EXPECT_THROW((void)xaml::convert_double("x"), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_double(""), xaml_convert_error);
    }

    TEST(xaml_convert_float, single_parse_semantics)
    {
        EXPECT_EQ(xaml::convert_float("2.5"), 2.5F);
        EXPECT_THROW((void)xaml::convert_float("2.5f"), xaml_convert_error); // no C# suffixes
    }

    TEST(xaml_convert_int, int32_parse_semantics)
    {
        EXPECT_EQ(xaml::convert_int("42"), 42);
        EXPECT_EQ(xaml::convert_int("-7"), -7);
        EXPECT_EQ(xaml::convert_int(" 8 "), 8);
        EXPECT_EQ(xaml::convert_int("+9"), 9);
        EXPECT_THROW((void)xaml::convert_int("1.5"), xaml_convert_error); // Integer style: no decimals
        EXPECT_THROW((void)xaml::convert_int("x"), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_int("99999999999"), xaml_convert_error); // OverflowException
    }

    TEST(xaml_convert_string, passthrough_and_markup_escape)
    {
        // BuiltInConversions.xaml: Text="foobar" stays "foobar".
        EXPECT_EQ(xaml::convert_string("foobar"), "foobar");
        // The "{}" escape prefix is stripped; everything after it is literal.
        EXPECT_EQ(xaml::convert_string("{}{Binding}"), "{Binding}");
        EXPECT_EQ(xaml::convert_string("{}"), "");
        // Without the escape the string passes through untouched.
        EXPECT_EQ(xaml::convert_string("{Binding}"), "{Binding}");
    }

    // ---- enums (Enum.Parse via name tables) ----

    TEST(xaml_convert_enums, text_alignment_names)
    {
        EXPECT_EQ(xaml::convert_text_alignment("Start"), maui::core::text_alignment::start);
        EXPECT_EQ(xaml::convert_text_alignment("Center"), maui::core::text_alignment::center);
        EXPECT_EQ(xaml::convert_text_alignment("End"), maui::core::text_alignment::end);
        EXPECT_EQ(xaml::convert_text_alignment("Justify"), maui::core::text_alignment::justify);
        // Enum.Parse trims its value...
        EXPECT_EQ(xaml::convert_text_alignment(" Center "), maui::core::text_alignment::center);
        // ...accepts the underlying numeric value of a defined member...
        EXPECT_EQ(xaml::convert_text_alignment("2"), maui::core::text_alignment::end);
        // ...and is case-SENSITIVE through the XAML loader (ignoreCase defaults to false).
        EXPECT_THROW((void)xaml::convert_text_alignment("center"), xaml_convert_error);
        // Port deviation (documented): undefined numeric values are rejected.
        EXPECT_THROW((void)xaml::convert_text_alignment("5"), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_text_alignment(""), xaml_convert_error);
    }

    TEST(xaml_convert_enums, aspect_names)
    {
        EXPECT_EQ(xaml::convert_aspect("AspectFit"), maui::core::aspect::aspect_fit);
        EXPECT_EQ(xaml::convert_aspect("AspectFill"), maui::core::aspect::aspect_fill);
        EXPECT_EQ(xaml::convert_aspect("Fill"), maui::core::aspect::fill);
        EXPECT_EQ(xaml::convert_aspect("Center"), maui::core::aspect::center);
        EXPECT_THROW((void)xaml::convert_aspect("Fit"), xaml_convert_error);
    }

    TEST(xaml_convert_enums, visibility_names)
    {
        EXPECT_EQ(xaml::convert_visibility("Visible"), maui::core::visibility::visible);
        EXPECT_EQ(xaml::convert_visibility("Hidden"), maui::core::visibility::hidden);
        EXPECT_EQ(xaml::convert_visibility("Collapsed"), maui::core::visibility::collapsed);
        EXPECT_THROW((void)xaml::convert_visibility("Gone"), xaml_convert_error);
    }

    TEST(xaml_convert_enums, return_type_names)
    {
        EXPECT_EQ(xaml::convert_return_type("Default"), maui::core::return_type::default_);
        EXPECT_EQ(xaml::convert_return_type("Done"), maui::core::return_type::done);
        EXPECT_EQ(xaml::convert_return_type("Go"), maui::core::return_type::go);
        EXPECT_EQ(xaml::convert_return_type("Next"), maui::core::return_type::next);
        EXPECT_EQ(xaml::convert_return_type("Search"), maui::core::return_type::search);
        EXPECT_EQ(xaml::convert_return_type("Send"), maui::core::return_type::send);
        EXPECT_THROW((void)xaml::convert_return_type("Enter"), xaml_convert_error);
    }

    // FlowDirectionConverter: enum names + the ltr/rtl/inherit aliases.
    TEST(xaml_convert_enums, flow_direction_names_and_aliases)
    {
        EXPECT_EQ(xaml::convert_flow_direction("LeftToRight"), maui::core::flow_direction::left_to_right);
        EXPECT_EQ(xaml::convert_flow_direction("RightToLeft"), maui::core::flow_direction::right_to_left);
        EXPECT_EQ(xaml::convert_flow_direction("MatchParent"), maui::core::flow_direction::match_parent);
        // The aliases are case-insensitive...
        EXPECT_EQ(xaml::convert_flow_direction("ltr"), maui::core::flow_direction::left_to_right);
        EXPECT_EQ(xaml::convert_flow_direction("RTL"), maui::core::flow_direction::right_to_left);
        EXPECT_EQ(xaml::convert_flow_direction("Inherit"), maui::core::flow_direction::match_parent);
        // ...but, unlike the enum names, NOT trimmed (C# compares the raw string).
        EXPECT_THROW((void)xaml::convert_flow_direction(" ltr "), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_flow_direction("foo"), xaml_convert_error);
    }

    // The generic helper itself, with a caller-supplied table (the per-enum convert_* functions
    // above are thin wrappers over this).
    enum class sample_enum : std::uint8_t
    {
        alpha = 0,
        beta = 1,
    };

    TEST(xaml_parse_enum, generic_table_driven_helper)
    {
        static constexpr std::array<xaml::enum_entry<sample_enum>, 2> names{{
            {.name = "Alpha", .value = sample_enum::alpha},
            {.name = "Beta", .value = sample_enum::beta},
        }};
        const std::span<const xaml::enum_entry<sample_enum>> table{names};
        EXPECT_EQ(xaml::parse_enum(std::string_view("Beta"), table, "sample_enum"), sample_enum::beta);
        EXPECT_EQ(xaml::parse_enum(std::string_view("1"), table, "sample_enum"), sample_enum::beta);
        EXPECT_EQ(xaml::try_parse_enum(std::string_view("nope"), table), std::nullopt);
        EXPECT_THROW((void)xaml::parse_enum(std::string_view("nope"), table, "sample_enum"), xaml_convert_error);
    }

    // ---- easing (EasingTypeConverter; oracle: src/Core/tests/UnitTests/Animations/EasingTests.cs) ----

    // The C# asserts compare the converted Easing against the static singleton by reference; the
    // port's easing is a copyable value, so identity is asserted behaviorally — same curve samples.
    void expect_same_easing(const maui::animations::easing& actual, const maui::animations::easing& expected)
    {
        for (const double v : {0.0, 0.2, 0.45, 0.7, 1.0})
        {
            EXPECT_DOUBLE_EQ(actual.ease(v), expected.ease(v));
        }
    }

    TEST(xaml_convert_easing, can_convert_from_easing_name_to_easing)
    {
        using maui::animations::easing;
        // EasingTests.CanConvertFromEasingNameToEasing: the exact name, the all-lowercase spelling,
        // and the "Easing."-qualified form, for every named easing.
        struct easing_case
        {
            std::string_view name;
            std::string_view lower;
            std::string_view qualified;
            const easing& expected;
        };
        const std::array<easing_case, 11> cases{{
            {"Linear", "linear", "Easing.Linear", easing::linear()},
            {"SinOut", "sinout", "Easing.SinOut", easing::sin_out()},
            {"SinIn", "sinin", "Easing.SinIn", easing::sin_in()},
            {"SinInOut", "sininout", "Easing.SinInOut", easing::sin_in_out()},
            {"CubicOut", "cubicout", "Easing.CubicOut", easing::cubic_out()},
            {"CubicIn", "cubicin", "Easing.CubicIn", easing::cubic_in()},
            {"CubicInOut", "cubicinout", "Easing.CubicInOut", easing::cubic_in_out()},
            {"BounceOut", "bounceout", "Easing.BounceOut", easing::bounce_out()},
            {"BounceIn", "bouncein", "Easing.BounceIn", easing::bounce_in()},
            {"SpringOut", "springout", "Easing.SpringOut", easing::spring_out()},
            {"SpringIn", "springin", "Easing.SpringIn", easing::spring_in()},
        }};
        for (const easing_case& test_case : cases)
        {
            expect_same_easing(xaml::convert_easing(test_case.name), test_case.expected);
            expect_same_easing(xaml::convert_easing(test_case.lower), test_case.expected);
            expect_same_easing(xaml::convert_easing(test_case.qualified), test_case.expected);
        }
        // The qualifier itself is case-insensitive too (Compare(parts[0], nameof(Easing))).
        expect_same_easing(xaml::convert_easing("easing.linear"), easing::linear());
    }

    TEST(xaml_convert_easing, invalid_easing_names_throw)
    {
        // EasingTests.InvalidEasingNamesThrow (InvalidOperationException -> the single error channel).
        EXPECT_THROW((void)xaml::convert_easing("WrongEasingName"), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_easing("Easing.Linear.SinInOut"), xaml_convert_error);
    }

    TEST(xaml_convert_easing, empty_and_whitespace_throw)
    {
        // EasingTests.NonTextEasingsAreNull returns a NULL Easing in C#; the port's easing has no
        // null form — the documented deviation throws instead (xaml_converters.hpp).
        EXPECT_THROW((void)xaml::convert_easing(""), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_easing(" "), xaml_convert_error);
    }

    // ---- text decorations (TextDecorationConverter; oracle: TextDecorationUnitTests.cs) ----

    TEST(xaml_convert_text_decorations, test_text_decoration_converter)
    {
        using maui::core::text_decorations;
        constexpr auto both = static_cast<text_decorations>(std::to_underlying(text_decorations::underline) |
                                                            std::to_underlying(text_decorations::strikethrough));
        // TextDecorationUnitTests.TestTextDecorationConverter, row for row.
        EXPECT_EQ(xaml::convert_text_decorations("strikethrough"), text_decorations::strikethrough);
        EXPECT_EQ(xaml::convert_text_decorations("underline"), text_decorations::underline);
        EXPECT_EQ(xaml::convert_text_decorations("line-through"), text_decorations::strikethrough);
        EXPECT_EQ(xaml::convert_text_decorations("none"), text_decorations::none);
        EXPECT_EQ(xaml::convert_text_decorations("strikethrough underline"), both);
        EXPECT_EQ(xaml::convert_text_decorations("underline strikethrough"), both);
        EXPECT_EQ(xaml::convert_text_decorations("underline line-through"), both);
        EXPECT_EQ(xaml::convert_text_decorations("line-through underline"), both);
        // The comma spelling and the PascalCase member names.
        EXPECT_EQ(xaml::convert_text_decorations("Underline,Strikethrough"), both);
        EXPECT_EQ(xaml::convert_text_decorations("Underline, Strikethrough"), both);
    }

    TEST(xaml_convert_text_decorations, invalid_throws)
    {
        EXPECT_THROW((void)xaml::convert_text_decorations("wavy"), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_text_decorations(""), xaml_convert_error);
        // C# Split keeps empty entries, so doubled separators throw there too.
        EXPECT_THROW((void)xaml::convert_text_decorations("underline  strikethrough"), xaml_convert_error);
        // The "line-through" alias is compared UNtrimmed (the C# quirk).
        EXPECT_THROW((void)xaml::convert_text_decorations("underline, line-through"), xaml_convert_error);
    }

    // ---- clear button visibility (the TypeConversionExtensions generic Enum.Parse path) ----

    TEST(xaml_convert_enums, clear_button_visibility_names)
    {
        EXPECT_EQ(xaml::convert_clear_button_visibility("Never"), maui::core::clear_button_visibility::never);
        EXPECT_EQ(xaml::convert_clear_button_visibility("WhileEditing"),
                  maui::core::clear_button_visibility::while_editing);
        EXPECT_THROW((void)xaml::convert_clear_button_visibility("whileediting"), xaml_convert_error);
        EXPECT_THROW((void)xaml::convert_clear_button_visibility("Always"), xaml_convert_error);
    }
} // namespace
