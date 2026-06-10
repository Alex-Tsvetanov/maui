// Tests for the window_handler seam over the BACKEND-PORTABLE platform mirror (window_platform's
// title/content_hosted + the handler lifecycle): connect hosts the page + pushes the title, a title
// change re-pushes, the default-registry resolution, disconnect, and the frame_changed re-push guard.
// Split out of lifecycle_tests.cpp so the per-backend suites can swap it as a pair with the window
// partial (the M6 convention): headless + apple compile THIS file (the apple NSWindow partial keeps the
// same mirrors in sync, and its content_page partial provides a real NSView, so content_hosted holds);
// ios swaps it for tests/controls/window_ios_tests.mm, which drives the genuine UIWindow instead (the
// ios scaffold still compiles the HEADLESS content_page partial, whose pages have no native UIView to
// host — the assertions here would test the wrong oracle there).
#include "maui/controls/content_page.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/window_handler.hpp"
#include "maui/graphics/rect.hpp"

#include <memory>

#include <gtest/gtest.h>

namespace
{
    // ---- window_handler seam: connect hosts the page + pushes the title ----

    TEST(window_handler, connect_pushes_title_and_hosts_the_content)
    {
        maui::controls::content_page page;
        // The page needs an attached view-handler so the window host can reach a native view (headless: a
        // mirror flag). A label-backed page content would work too; the page itself is the window content.
        auto page_handler = std::make_shared<maui::core::content_page_handler>();
        page.set_handler(page_handler);

        maui::controls::window win;
        win.set_title("Hello");
        win.set_content(page);

        auto handler = std::make_shared<maui::core::window_handler>();
        win.set_handler(handler); // connect: create the (mirror) window, host the page, run the mapper

        ASSERT_NE(handler->typed_platform_view(), nullptr);
        EXPECT_EQ(handler->typed_platform_view()->title, "Hello");   // MapTitle pushed the title
        EXPECT_TRUE(handler->typed_platform_view()->content_hosted); // MapContent hosted the page
        EXPECT_EQ(win.handler().get(), handler.get());
    }

    TEST(window_handler, title_change_after_connect_repushes)
    {
        maui::controls::window win;
        win.set_title("First");
        auto handler = std::make_shared<maui::core::window_handler>();
        win.set_handler(handler);
        EXPECT_EQ(handler->typed_platform_view()->title, "First");

        win.set_title("Second"); // OnPropertyChanged(Title) -> update_value("title") -> MapTitle
        EXPECT_EQ(handler->typed_platform_view()->title, "Second");
    }

    TEST(window_handler, resolves_from_the_default_registry)
    {
        // window self-registers (MAUI_REGISTER_HANDLER in window.cpp), so hosting can create the handler.
        auto handler = maui::core::default_handler_registry().create_handler<maui::controls::window>();
        ASSERT_NE(handler, nullptr);
        EXPECT_NE(dynamic_cast<maui::core::window_handler*>(handler.get()), nullptr);
    }

    TEST(window_handler, disconnect_clears_the_seam)
    {
        maui::controls::window win;
        auto handler = std::make_shared<maui::core::window_handler>();
        win.set_handler(handler);
        ASSERT_NE(handler->virtual_view(), nullptr);

        win.set_handler(nullptr); // disconnect the previous handler
        EXPECT_EQ(handler->virtual_view(), nullptr);
        EXPECT_EQ(win.handler(), nullptr);
    }

    TEST(window_handler, frame_changed_during_connect_does_not_loop_the_handler)
    {
        // The handler's update_value for x/y/width/height is suppressed during the frame_changed batch
        // (Window._batchFrameUpdate), so a platform frame report does not re-push to the platform window.
        maui::controls::window win;
        auto handler = std::make_shared<maui::core::window_handler>();
        win.set_handler(handler);
        win.frame_changed(maui::graphics::rect(1, 2, 3, 4)); // must not crash / re-enter
        EXPECT_EQ(win.frame(), maui::graphics::rect(1, 2, 3, 4));
    }
} // namespace
