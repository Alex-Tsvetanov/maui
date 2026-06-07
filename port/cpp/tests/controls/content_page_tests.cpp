// Tests for the content_page control + its headless handler seam — a page hosting a single content
// child within a padding (+ a title). Two things are verified: (1) the control's measure/arrange port
// LayoutExtensions.MeasureContent/ArrangeContent (the content is sized + placed within the padding), and
// (2) the headless content_page_platform's single-content mirror tracks the control's content as it is
// set/replaced/cleared so the native host stays in sync.
#include "maui/controls/content_page.hpp"

#include <memory>

#include "maui/core/content_page_handler.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_content_view.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_padding.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "tests/layouts/layout_test_helpers.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::content_page;
    using maui::core::content_page_handler;
    using maui::core::i_content_view;
    using maui::core::i_element_handler;
    using maui::core::i_padding;
    using maui::core::thickness;
    using maui::graphics::rect;
    using maui::graphics::size;
    using maui::layouts::testing::mock_view;

    // ---- the control in isolation (no handler) ----

    TEST(content_page, defaults_empty_with_no_content_zero_padding_empty_title)
    {
        content_page page;
        EXPECT_EQ(page.content(), nullptr);
        EXPECT_EQ(page.padding(), thickness());
        EXPECT_EQ(page.title(), "");
    }

    TEST(content_page, content_is_settable_and_clearable)
    {
        content_page page;
        mock_view child;
        page.set_content(child);
        EXPECT_EQ(page.content(), &child);

        page.set_content(nullptr);
        EXPECT_EQ(page.content(), nullptr);
    }

    TEST(content_page, title_and_padding_are_settable)
    {
        content_page page;
        page.set_title("Home");
        EXPECT_EQ(page.title(), "Home");

        page.set_padding(thickness(5));
        EXPECT_EQ(page.padding(), thickness(5));
    }

    TEST(content_page, usable_through_interface_references)
    {
        content_page page;
        mock_view child;
        page.set_content(child);
        page.set_padding(thickness(8));

        i_content_view& as_content = page;
        i_padding& as_padding = page;
        EXPECT_EQ(as_content.content(), &child);
        EXPECT_EQ(as_content.padding(), thickness(8));
        EXPECT_EQ(as_padding.padding(), thickness(8));
    }

    // ---- measure/arrange: MeasureContent / ArrangeContent within the padding ----

    TEST(content_page, measure_sizes_content_plus_padding)
    {
        content_page page;
        page.set_padding(thickness(10));
        mock_view child;
        child.configure({100, 40});
        page.set_content(child);

        // content 100x40 + padding {10} on all sides -> 120x60.
        const size measured = page.measure(1000, 1000);
        EXPECT_EQ(measured.width, 120.0);
        EXPECT_EQ(measured.height, 60.0);

        // The content was measured with the padding subtracted from the constraints.
        EXPECT_EQ(child.last_measure_width, 980.0);  // 1000 - (10 + 10)
        EXPECT_EQ(child.last_measure_height, 980.0); // 1000 - (10 + 10)
    }

    TEST(content_page, measure_with_no_content_is_padding_only)
    {
        content_page page;
        page.set_padding(thickness(10));

        const size measured = page.measure(1000, 1000);
        EXPECT_EQ(measured.width, 20.0);  // padding horizontal only
        EXPECT_EQ(measured.height, 20.0); // padding vertical only
    }

    TEST(content_page, arrange_places_content_within_padding)
    {
        content_page page;
        page.set_padding(thickness(10));
        mock_view child;
        child.configure({100, 40});
        page.set_content(child);

        page.measure(1000, 1000);
        page.arrange(rect(0, 0, 120, 60));

        // Content arranged at (left+padding, top+padding) with bounds shrunk by the padding.
        EXPECT_EQ(child.last_arrange, rect(10, 10, 100, 40)); // 120-20 wide, 60-20 tall
    }

    TEST(content_page, arrange_with_no_content_does_not_crash)
    {
        content_page page;
        page.set_padding(thickness(10));
        const size arranged = page.arrange(rect(0, 0, 50, 50));
        EXPECT_EQ(arranged.width, 50.0);
        EXPECT_EQ(arranged.height, 50.0);
    }

    // ---- the handler seam (control <-> handler <-> headless host): the host mirrors the content ----

    TEST(content_page_seam, attaching_handler_creates_host_and_mirrors_initial_content)
    {
        content_page page;
        mock_view child;
        page.set_content(child); // set before the handler is attached

        auto handler = std::make_shared<content_page_handler>();
        page.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), &page);
        // The full property/command run on connect re-hosts the already-set content.
        EXPECT_EQ(handler->typed_platform_view()->hosted_content, &child);
    }

    TEST(content_page_seam, setting_content_after_attach_rehosts)
    {
        content_page page;
        auto handler = std::make_shared<content_page_handler>();
        page.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->hosted_content, nullptr);

        mock_view first;
        page.set_content(first); // -> handler->invoke("set_content") -> map_set_content -> set_content()
        EXPECT_EQ(platform->hosted_content, &first);

        mock_view second;
        page.set_content(second); // replacing the content re-hosts the new child
        EXPECT_EQ(platform->hosted_content, &second);

        page.set_content(nullptr); // clearing the content empties the host
        EXPECT_EQ(platform->hosted_content, nullptr);
    }

    TEST(content_page_seam, handler_resolved_from_default_registry)
    {
        // content_page -> content_page_handler is self-registered (MAUI_REGISTER_HANDLER).
        const std::shared_ptr<i_element_handler> handler =
            maui::core::default_handler_registry().create_handler<content_page>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<content_page_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        content_page page;
        mock_view child;
        page.set_content(child);
        page.set_handler(handler);
        EXPECT_EQ(resolved->typed_platform_view()->hosted_content, &child);
    }
} // namespace
