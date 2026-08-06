// Tests for the web_view_page demo (examples/gallery/pages/web_view_page.hpp) — backend-agnostic: the page
// is pure cross-platform control wiring, so this suite compiles in every preset and proves the demo's
// interactions (source swap buttons, navigated → status label, eval_js → result label) without a
// hosting main. The handler-dependent paths (back/forward/reload, the real eval round trip) are covered
// by the web_view seam suites per backend.
#include "examples/gallery/pages/web_view_page.hpp"
#include "maui/core/web_navigation_event.hpp"
#include "maui/core/web_navigation_result.hpp"

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

    // Regression net for the windows dark render. The html source reaches WebView2 through
    // NavigateToString, whose document brings no opaque canvas, so on a dark host the base paints through
    // and the page scored SSIM 0.7260 / 29.43% against MAUI's white one. `<meta color-scheme>` does not
    // cover it — only an explicit background does. Both the initial source and the ones the Page A/Page B
    // buttons swap in must carry it, or the same page goes red again on whichever one is showing.
    TEST(web_view_page, every_html_source_declares_an_opaque_background)
    {
        web_view_page demo;
        const auto declares_background = [&] {
            auto* source = dynamic_cast<maui::controls::html_web_view_source*>(demo.browser().source());
            return source != nullptr && source->html().find("html{background:#fff}") != std::string::npos;
        };
        EXPECT_TRUE(declares_background()) << "initial source";
        demo.load_a_button().send_clicked();
        EXPECT_TRUE(declares_background()) << "after Page A";
        demo.load_b_button().send_clicked();
        EXPECT_TRUE(declares_background()) << "after Page B";
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
