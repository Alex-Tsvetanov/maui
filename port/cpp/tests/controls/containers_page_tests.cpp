// Tests for the containers_page demo (examples/gallery/pages/containers_page.hpp) — backend-agnostic:
// the page is pure cross-platform control wiring, so this suite compiles in every preset and proves
// the demo's structure (scroll_view → stack → border/frame/content_view, each hosting its content)
// and interactions (scrolled → readout, scroll_to_completed → the done marker) without a hosting
// main.
#include "examples/gallery/pages/containers_page.hpp"

#include <gtest/gtest.h>

namespace
{
    using maui::samples::containers_page;

    TEST(containers_page, builds_the_container_tree)
    {
        containers_page demo;
        EXPECT_EQ(demo.page().content(), &demo.scroller());
        EXPECT_EQ(demo.scroller().content(), &demo.stack());
        EXPECT_EQ(demo.stack().count(), 4); // readout + border + frame + content_view
        EXPECT_EQ(demo.bordered().content(), &demo.bordered_text());
        EXPECT_EQ(demo.framed().content(), &demo.framed_text());
        EXPECT_EQ(demo.wrapper().content_child(), demo.wrapped_text());
    }

    TEST(containers_page, border_and_frame_carry_their_decoration)
    {
        containers_page demo;
        ASSERT_NE(demo.bordered().stroke(), nullptr);
        EXPECT_EQ(demo.bordered().stroke_thickness(), 2.0);
        EXPECT_EQ(demo.bordered().stroke_dash_pattern().size(), 2U);

        ASSERT_TRUE(demo.framed().border_color().has_value());
        EXPECT_EQ(demo.framed().corner_radius(), 6.0F);
        EXPECT_NE(demo.framed().shadow(), nullptr); // HasShadow default true
    }

    TEST(containers_page, scrolling_updates_the_readout)
    {
        containers_page demo;
        demo.scroller().set_scrolled_position(0, 42);
        EXPECT_EQ(demo.readout().text(), "Scrolled to: 0 / 42");
    }

    TEST(containers_page, scroll_completion_marks_the_readout)
    {
        containers_page demo;
        demo.scroller().set_scrolled_position(0, 10);
        demo.scroller().scroll_finished(); // the platform ack (the ScrollToAsync Task stand-in)
        EXPECT_EQ(demo.readout().text(), "Scrolled to: 0 / 10 (done)");
    }
} // namespace
