// Tests for the web_view_page demo (src/samples/pages/web_view_page.hpp) — backend-agnostic: the page
// is pure cross-platform control wiring, so this suite compiles in every preset and proves the demo's
// interactions (source swap buttons, navigated → status label, eval_js → result label) without a
// hosting main. The handler-dependent paths (back/forward/reload, the real eval round trip) are covered
// by the web_view seam suites per backend.
#include "maui/core/web_navigation_event.hpp"
#include "maui/core/web_navigation_result.hpp"
#include "src/samples/pages/web_view_page.hpp"

#include <string>

#include "maui/controls/html_web_view_source.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::samples::web_view_page;

    TEST(web_view_page, builds_the_stack_with_the_browser_first)
    {
        web_view_page demo;
        EXPECT_EQ(demo.page().content(), &demo.stack());
        EXPECT_EQ(demo.stack().count(), 9); // browser + 2 labels + 6 buttons
    }

    TEST(web_view_page, starts_on_the_welcome_source)
    {
        web_view_page demo;
        auto* source = dynamic_cast<maui::controls::html_web_view_source*>(demo.browser().source());
        ASSERT_NE(source, nullptr);
        EXPECT_NE(source->html().find("Welcome"), std::string::npos);
        EXPECT_EQ(source->base_url(), "https://demo.test/welcome");
    }

    TEST(web_view_page, load_buttons_swap_the_html_source)
    {
        web_view_page demo;
        demo.load_b_button().send_clicked();
        auto* source = dynamic_cast<maui::controls::html_web_view_source*>(demo.browser().source());
        ASSERT_NE(source, nullptr);
        EXPECT_NE(source->html().find("Page B"), std::string::npos);
        EXPECT_EQ(source->base_url(), "https://demo.test/b");
    }

    TEST(web_view_page, navigated_event_updates_the_status_label)
    {
        web_view_page demo;
        demo.browser().send_navigated(maui::core::web_navigation_event::new_page, "https://demo.test/b",
                                      maui::core::web_navigation_result::success);
        EXPECT_EQ(demo.status().text(), "new_page -> https://demo.test/b");
    }

    TEST(web_view_page, eval_button_routes_the_result_into_the_label)
    {
        web_view_page demo;
        // Without a handler the round trip completes with the null result (see web_view::eval_js).
        demo.eval_button().send_clicked();
        EXPECT_EQ(demo.result().text(), "Eval result: <null>");
    }
} // namespace
