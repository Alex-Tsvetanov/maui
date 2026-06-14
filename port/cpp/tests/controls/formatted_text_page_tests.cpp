// Tests for the formatted_text_page demo (src/samples/pages/formatted_text_page.hpp) — backend-agnostic:
// the page is pure cross-platform control wiring, so this suite compiles in every preset and proves the
// rich-text demo's structure (three styled spans on the rich label, the plain label's Text path).
#include "src/samples/pages/formatted_text_page.hpp"

#include "maui/core/font_attributes.hpp"
#include "maui/core/text_decorations.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::samples::formatted_text_page;

    TEST(formatted_text_page, builds_the_two_label_stack)
    {
        formatted_text_page demo;
        EXPECT_EQ(demo.page().content(), &demo.stack());
        EXPECT_EQ(demo.stack().count(), 2); // rich + plain labels
    }

    TEST(formatted_text_page, rich_label_has_three_resolved_runs)
    {
        formatted_text_page demo;
        ASSERT_NE(demo.rich_label().formatted_text(), nullptr);
        EXPECT_EQ(demo.rich_label().formatted_text()->to_string(), "Bold red italic underlined k e r n e d");

        const auto& runs = demo.rich_label().formatted_text_runs();
        ASSERT_EQ(runs.size(), 3U);
        EXPECT_EQ(runs[0].run_font.weight(), maui::core::font_weight::bold);
        EXPECT_EQ(runs[1].decorations, maui::core::text_decorations::underline);
        EXPECT_EQ(runs[2].character_spacing, 2.5);
    }

    TEST(formatted_text_page, plain_label_uses_the_text_path)
    {
        formatted_text_page demo;
        EXPECT_EQ(demo.plain_label().formatted_text(), nullptr);
        EXPECT_EQ(demo.plain_label().text(), "Plain text label");
    }
} // namespace
