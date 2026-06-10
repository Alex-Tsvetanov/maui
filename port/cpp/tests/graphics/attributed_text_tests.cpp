// Tests for maui::graphics::text — attributed_text / attributed_text_run / text_attributes.
// Characterization tests: src/Graphics/tests has no GPU-free unit tests for Graphics.Text, so the
// behavior is derived directly from the C# source (src/Graphics/src/Graphics/Text/{AttributedText,
// AttributedTextRun,AttributedTextRunExtensions,AttributedTextExtensions,TextAttributes,
// TextAttributesExtensions,TextAttributeExtensions}.cs).

#include "maui/graphics/text/attributed_text.hpp"

#include <optional>
#include <string>
#include <vector>

#include "maui/graphics/text/attributed_text_run.hpp"
#include "maui/graphics/text/text_attributes.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::graphics::text::attributed_text;
    using maui::graphics::text::attributed_text_block;
    using maui::graphics::text::attributed_text_run;
    using maui::graphics::text::marker_type;
    using maui::graphics::text::text_attribute;
    using maui::graphics::text::text_attributes;

    text_attributes bold_attributes()
    {
        text_attributes attributes;
        attributes.set_bold(true);
        return attributes;
    }

    text_attributes italic_attributes()
    {
        text_attributes attributes;
        attributes.set_italic(true);
        return attributes;
    }

    // ---- text_attributes (TextAttributes + the typed extension accessors) ----

    TEST(text_attributes_test, starts_empty)
    {
        const text_attributes attributes;
        EXPECT_TRUE(attributes.empty());
        EXPECT_EQ(attributes.get_attribute(text_attribute::bold), std::nullopt);
        EXPECT_EQ(attributes.get_attribute(text_attribute::bold, "fallback"), "fallback");
    }

    TEST(text_attributes_test, bools_store_csharp_to_string_and_parse_case_insensitively)
    {
        text_attributes attributes;
        attributes.set_bold(true);
        // C# bool.ToString() spelling.
        EXPECT_EQ(attributes.get_attribute(text_attribute::bold), "True");
        EXPECT_TRUE(attributes.get_bold());

        // C# bool.TryParse is case-insensitive.
        attributes.set_attribute(text_attribute::italic, "TRUE");
        EXPECT_TRUE(attributes.get_italic());
        attributes.set_attribute(text_attribute::italic, "garbage");
        EXPECT_FALSE(attributes.get_italic()); // parse failure -> default(false)
    }

    TEST(text_attributes_test, setting_the_default_removes_the_entry)
    {
        text_attributes attributes;
        attributes.set_bold(true);
        attributes.set_bold(false); // default -> removed
        EXPECT_TRUE(attributes.empty());

        attributes.set_font_size(20);
        EXPECT_FALSE(attributes.empty());
        attributes.set_font_size(text_attributes::default_font_size); // 12 is the default -> removed
        EXPECT_TRUE(attributes.empty());
    }

    TEST(text_attributes_test, font_size_defaults_to_twelve_and_round_trips)
    {
        text_attributes attributes;
        EXPECT_FLOAT_EQ(attributes.get_font_size(), 12.0F);
        EXPECT_FLOAT_EQ(attributes.get_font_size(9.0F), 9.0F); // explicit context default

        attributes.set_font_size(14.5F);
        EXPECT_FLOAT_EQ(attributes.get_font_size(), 14.5F);
    }

    TEST(text_attributes_test, int_and_float_parse_failures_fall_back)
    {
        text_attributes attributes;
        attributes.set_attribute(text_attribute::font_size, "abc");
        EXPECT_FLOAT_EQ(attributes.get_float_attribute(text_attribute::font_size, 3.0F), 3.0F);
        EXPECT_EQ(attributes.get_int_attribute(text_attribute::font_size, 7), 7);

        attributes.set_int_attribute(text_attribute::font_size, 42, 0);
        EXPECT_EQ(attributes.get_int_attribute(text_attribute::font_size, 0), 42);
    }

    // C# GetMarker/SetMarker store the enum NAME under TextAttribute.UnorderedList (a quirk
    // preserved by the port).
    TEST(text_attributes_test, marker_round_trips_under_the_unordered_list_key)
    {
        text_attributes attributes;
        EXPECT_EQ(attributes.get_marker(), marker_type::closed_circle); // default

        attributes.set_marker(marker_type::hyphen);
        EXPECT_EQ(attributes.get_attribute(text_attribute::unordered_list), "Hyphen");
        EXPECT_EQ(attributes.get_marker(), marker_type::hyphen);

        attributes.set_marker(marker_type::closed_circle); // default -> removed
        EXPECT_TRUE(attributes.empty());
    }

    TEST(text_attributes_test, union_ctor_lets_second_win)
    {
        text_attributes first;
        first.set_font_name("Helvetica");
        first.set_bold(true);
        text_attributes second;
        second.set_font_name("Courier");

        const text_attributes merged(first, second);
        EXPECT_EQ(merged.get_font_name(), "Courier"); // second overwrites
        EXPECT_TRUE(merged.get_bold());               // first's unique entry survives
    }

    TEST(text_attributes_test, colors_travel_as_strings)
    {
        text_attributes attributes;
        attributes.set_foreground_color("#ff0000");
        attributes.set_background_color("#00ff00");
        EXPECT_EQ(attributes.get_foreground_color(), "#ff0000");
        EXPECT_EQ(attributes.get_background_color(), "#00ff00");
        EXPECT_EQ(attributes.get_attribute(text_attribute::color), "#ff0000");
        EXPECT_EQ(attributes.get_attribute(text_attribute::background), "#00ff00");
    }

    // ---- attributed_text_run (+ the extension members) ----

    TEST(attributed_text_run_test, accessors_and_get_end)
    {
        const attributed_text_run run(3, 4, bold_attributes());
        EXPECT_EQ(run.start(), 3);
        EXPECT_EQ(run.length(), 4);
        EXPECT_EQ(run.get_end(), 7);
        EXPECT_TRUE(run.attributes().get_bold());
    }

    TEST(attributed_text_run_test, intersects_half_open_semantics)
    {
        const attributed_text_run run(0, 5, {});
        EXPECT_TRUE(run.intersects(attributed_text_run(3, 4, {})));
        EXPECT_TRUE(run.intersects(attributed_text_run(4, 1, {})));
        EXPECT_FALSE(run.intersects(attributed_text_run(5, 2, {}))); // touching ends don't intersect
        EXPECT_TRUE(run.intersects(3, 4));
        EXPECT_FALSE(run.intersects(5, 2));
    }

    TEST(attributed_text_run_test, intersects_exactly_is_same_range)
    {
        const attributed_text_run run(2, 3, {});
        EXPECT_TRUE(run.intersects_exactly(attributed_text_run(2, 3, bold_attributes())));
        EXPECT_FALSE(run.intersects_exactly(attributed_text_run(2, 4, {})));
        EXPECT_TRUE(run.intersects_exactly(2, 3));
        EXPECT_FALSE(run.intersects_exactly(3, 3));
    }

    TEST(attributed_text_run_test, calculated_intersections_same_range_merges)
    {
        const attributed_text_run first(0, 5, bold_attributes());
        const attributed_text_run second(0, 5, italic_attributes());

        const std::vector<attributed_text_run> result = first.calculated_intersections(second);
        ASSERT_EQ(result.size(), 1U);
        EXPECT_EQ(result[0].start(), 0);
        EXPECT_EQ(result[0].length(), 5);
        EXPECT_TRUE(result[0].attributes().get_bold());
        EXPECT_TRUE(result[0].attributes().get_italic());
    }

    TEST(attributed_text_run_test, calculated_intersections_same_start_splits_the_tail)
    {
        const attributed_text_run first(0, 8, bold_attributes());
        const attributed_text_run second(0, 5, italic_attributes());

        const std::vector<attributed_text_run> result = first.calculated_intersections(second);
        ASSERT_EQ(result.size(), 2U);
        // [0,5): combined; [5,8): first's own attributes.
        EXPECT_EQ(result[0].start(), 0);
        EXPECT_EQ(result[0].length(), 5);
        EXPECT_TRUE(result[0].attributes().get_bold());
        EXPECT_TRUE(result[0].attributes().get_italic());
        EXPECT_EQ(result[1].start(), 5);
        EXPECT_EQ(result[1].length(), 3);
        EXPECT_TRUE(result[1].attributes().get_bold());
        EXPECT_FALSE(result[1].attributes().get_italic());
    }

    TEST(attributed_text_run_test, calculated_intersections_same_end_splits_the_head)
    {
        const attributed_text_run first(0, 8, bold_attributes());
        const attributed_text_run second(3, 5, italic_attributes());

        const std::vector<attributed_text_run> result = first.calculated_intersections(second);
        ASSERT_EQ(result.size(), 2U);
        EXPECT_EQ(result[0].start(), 0);
        EXPECT_EQ(result[0].length(), 3); // first only
        EXPECT_FALSE(result[0].attributes().get_italic());
        EXPECT_EQ(result[1].start(), 3);
        EXPECT_EQ(result[1].length(), 5); // combined
        EXPECT_TRUE(result[1].attributes().get_bold());
        EXPECT_TRUE(result[1].attributes().get_italic());
    }

    TEST(attributed_text_run_test, calculated_intersections_contained_run_splits_three_way)
    {
        // The general (different start, different end) branch — sane when one run contains the
        // other: head (first only), middle (combined), tail (whichever run reaches further).
        const attributed_text_run first(0, 10, bold_attributes());
        const attributed_text_run second(4, 3, italic_attributes());

        const std::vector<attributed_text_run> result = first.calculated_intersections(second);
        ASSERT_EQ(result.size(), 3U);
        EXPECT_EQ(result[0].start(), 0);
        EXPECT_EQ(result[0].length(), 4); // first only
        EXPECT_FALSE(result[0].attributes().get_italic());
        EXPECT_EQ(result[1].start(), 4);
        EXPECT_EQ(result[1].length(), 3); // combined
        EXPECT_TRUE(result[1].attributes().get_bold());
        EXPECT_TRUE(result[1].attributes().get_italic());
        EXPECT_EQ(result[2].start(), 7);
        EXPECT_EQ(result[2].length(), 3); // first reaches further -> its attributes
        EXPECT_TRUE(result[2].attributes().get_bold());
        EXPECT_FALSE(result[2].attributes().get_italic());
    }

    // ---- optimize_runs (AttributedTextRunExtensions.Optimize) ----

    TEST(optimize_runs_test, clamps_to_text_bounds_and_drops_empty)
    {
        std::vector<attributed_text_run> runs{
            // C# clamps the START to 0 but keeps the LENGTH (the run shifts to [0, 5)).
            attributed_text_run(-2, 5, bold_attributes()),
            attributed_text_run(8, 10, italic_attributes()), // clamps to [8, 10)
            attributed_text_run(12, 3, {}),                  // fully outside -> dropped
        };
        optimize_runs(runs, 10);

        ASSERT_EQ(runs.size(), 2U);
        EXPECT_EQ(runs[0].start(), 0);
        EXPECT_EQ(runs[0].length(), 5);
        EXPECT_EQ(runs[1].start(), 8);
        EXPECT_EQ(runs[1].length(), 2);
    }

    TEST(optimize_runs_test, sorts_by_start_then_length)
    {
        std::vector<attributed_text_run> runs{
            attributed_text_run(6, 2, {}),
            attributed_text_run(0, 3, {}),
        };
        optimize_runs(runs, 10);

        ASSERT_EQ(runs.size(), 2U);
        EXPECT_EQ(runs[0].start(), 0);
        EXPECT_EQ(runs[1].start(), 6);
    }

    TEST(optimize_runs_test, exact_overlaps_merge_their_attributes)
    {
        std::vector<attributed_text_run> runs{
            attributed_text_run(0, 5, bold_attributes()),
            attributed_text_run(0, 5, italic_attributes()),
        };
        optimize_runs(runs, 10);

        ASSERT_EQ(runs.size(), 1U);
        EXPECT_TRUE(runs[0].attributes().get_bold());
        EXPECT_TRUE(runs[0].attributes().get_italic());
    }

    TEST(optimize_runs_test, partial_overlaps_split_into_disjoint_runs)
    {
        std::vector<attributed_text_run> runs{
            attributed_text_run(0, 10, bold_attributes()),
            attributed_text_run(4, 3, italic_attributes()),
        };
        optimize_runs(runs, 10);

        ASSERT_EQ(runs.size(), 3U);
        EXPECT_EQ(runs[0].start(), 0);
        EXPECT_EQ(runs[0].length(), 4);
        EXPECT_EQ(runs[1].start(), 4);
        EXPECT_EQ(runs[1].length(), 3);
        EXPECT_TRUE(runs[1].attributes().get_bold());
        EXPECT_TRUE(runs[1].attributes().get_italic());
        EXPECT_EQ(runs[2].start(), 7);
        EXPECT_EQ(runs[2].length(), 3);
        EXPECT_TRUE(runs[2].attributes().get_bold());
        EXPECT_FALSE(runs[2].attributes().get_italic());
    }

    // ---- attributed_text + the extensions ----

    TEST(attributed_text_test, ctor_accessors_and_optimal_flag)
    {
        const attributed_text value("Hello", {attributed_text_run(0, 5, bold_attributes())});
        EXPECT_EQ(value.text(), "Hello");
        ASSERT_EQ(value.runs().size(), 1U);
        EXPECT_FALSE(value.optimal());

        const attributed_text optimal("Hi", {}, true);
        EXPECT_TRUE(optimal.optimal());
    }

    TEST(attributed_text_test, optimize_redistributes_and_flags_optimal)
    {
        const attributed_text value("HelloWorld", {attributed_text_run(0, 5, bold_attributes()),
                                                   attributed_text_run(5, 5, italic_attributes())});
        const attributed_text optimized = optimize(value);
        EXPECT_TRUE(optimized.optimal());
        EXPECT_EQ(optimized.text(), "HelloWorld");
        ASSERT_EQ(optimized.runs().size(), 2U);
        EXPECT_EQ(optimized.runs()[0].start(), 0);
        EXPECT_EQ(optimized.runs()[1].start(), 5);
    }

    TEST(attributed_text_test, optimize_passes_an_optimal_text_through)
    {
        const attributed_text value("Hi", {attributed_text_run(0, 2, bold_attributes())}, true);
        const attributed_text optimized = optimize(value);
        EXPECT_TRUE(optimized.optimal());
        EXPECT_EQ(optimized.runs().size(), 1U);
    }

    TEST(attributed_text_test, create_paragraphs_rebases_runs_per_line)
    {
        const attributed_text value("Hello\nWorld", {attributed_text_run(0, 5, bold_attributes()),
                                                     attributed_text_run(6, 5, italic_attributes())});
        const std::vector<attributed_text> paragraphs = create_paragraphs(value);

        ASSERT_EQ(paragraphs.size(), 2U);
        EXPECT_EQ(paragraphs[0].text(), "Hello");
        ASSERT_EQ(paragraphs[0].runs().size(), 1U);
        EXPECT_EQ(paragraphs[0].runs()[0].start(), 0);
        EXPECT_EQ(paragraphs[0].runs()[0].length(), 5);
        EXPECT_TRUE(paragraphs[0].runs()[0].attributes().get_bold());

        EXPECT_EQ(paragraphs[1].text(), "World");
        ASSERT_EQ(paragraphs[1].runs().size(), 1U);
        EXPECT_EQ(paragraphs[1].runs()[0].start(), 0); // rebased to the paragraph
        EXPECT_TRUE(paragraphs[1].runs()[0].attributes().get_italic());
    }

    TEST(attributed_text_test, create_paragraphs_spreads_a_covering_run)
    {
        // One run covering everything: its attributes apply to both paragraphs (the run is longer
        // than each line, so CreateParagraphRun keeps it for the next paragraph).
        const attributed_text value("ab\ncd", {attributed_text_run(0, 5, bold_attributes())});
        const std::vector<attributed_text> paragraphs = create_paragraphs(value);

        ASSERT_EQ(paragraphs.size(), 2U);
        ASSERT_EQ(paragraphs[0].runs().size(), 1U);
        EXPECT_EQ(paragraphs[0].runs()[0].length(), 2);
        ASSERT_EQ(paragraphs[1].runs().size(), 1U);
        EXPECT_TRUE(paragraphs[1].runs()[0].attributes().get_bold());
    }

    TEST(attributed_text_test, create_blocks_cuts_attributed_and_gap_blocks)
    {
        const attributed_text value("Hello World", {attributed_text_run(6, 5, bold_attributes())});
        const std::vector<attributed_text_block> blocks = create_blocks(value);

        ASSERT_EQ(blocks.size(), 2U);
        EXPECT_EQ(blocks[0].text(), "Hello ");
        EXPECT_EQ(blocks[0].attributes(), std::nullopt);
        EXPECT_EQ(blocks[1].text(), "World");
        EXPECT_EQ(blocks[1].attributes(), std::optional{bold_attributes()});
    }

    TEST(attributed_text_test, create_blocks_emits_the_unattributed_tail)
    {
        const attributed_text value("Hello World", {attributed_text_run(0, 5, bold_attributes())});
        const std::vector<attributed_text_block> blocks = create_blocks(value);

        ASSERT_EQ(blocks.size(), 2U);
        EXPECT_EQ(blocks[0].text(), "Hello");
        EXPECT_EQ(blocks[0].attributes(), std::optional{bold_attributes()});
        EXPECT_EQ(blocks[1].text(), " World");
        EXPECT_EQ(blocks[1].attributes(), std::nullopt);
    }

    TEST(attributed_text_test, create_paragraph_run_returns_the_resume_index)
    {
        const attributed_text value("HelloWorld", {attributed_text_run(0, 5, bold_attributes()),
                                                   attributed_text_run(5, 5, italic_attributes())});
        std::vector<attributed_text_run> runs;
        // First line [0, 5): consumes exactly run 0 -> resume at index 1.
        const int next = create_paragraph_run(value, 0, 5, runs, 0);
        EXPECT_EQ(next, 1);
        ASSERT_EQ(runs.size(), 1U);
        EXPECT_TRUE(runs[0].attributes().get_bold());
    }
} // namespace
