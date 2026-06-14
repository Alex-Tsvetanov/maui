// Tests for the W2-19 items_page demo wiring (backend-agnostic): the page owns the tree, the
// collection_view drives the readout through selection_changed, and the live collection feeds the
// EmptyView toggle.

#include <memory>
#include <string>

#include "maui/controls/items/collection_view_handler.hpp"
#include "maui/controls/items/items_view_source.hpp"
#include "maui/controls/items/selection_mode.hpp"
#include "src/samples/pages/items_page.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::collection_view_handler;
    using maui::controls::index_path;
    using maui::samples::items_page;

    TEST(items_page, builds_the_tree_and_defaults)
    {
        items_page page;
        EXPECT_EQ(page.page().title(), "Items");
        EXPECT_EQ(page.tasks()->size(), 3U);
        EXPECT_EQ(page.list().selection_mode(), maui::controls::selection_mode::single);
        EXPECT_EQ(page.readout().text(), "Pick a task");
        EXPECT_EQ(page.page().content(), &page.stack());
    }

    TEST(items_page, selection_drives_the_readout)
    {
        items_page page;
        auto handler = std::make_shared<collection_view_handler>();
        page.list().set_handler(handler);

        handler->simulate_select({0, 1});
        EXPECT_EQ(page.readout().text(), "Selected: Review the port");
    }

    TEST(items_page, clearing_tasks_shows_the_empty_view)
    {
        items_page page;
        auto handler = std::make_shared<collection_view_handler>();
        page.list().set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_FALSE(platform->empty_view.present);

        page.clear_tasks();
        EXPECT_TRUE(platform->empty_view.present);
        EXPECT_EQ(platform->empty_view.text, "All done!");

        page.add_task("New task");
        EXPECT_FALSE(platform->empty_view.present);
        EXPECT_EQ(platform->realized.front().text, "New task");
    }
} // namespace
