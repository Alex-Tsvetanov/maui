// Tests for the swipe_refresh_page demo (examples/gallery/pages/swipe_refresh_page.hpp) — backend-agnostic:
// the page is pure cross-platform control wiring, so this suite compiles in every preset and proves the
// demo's structure (content_page → refresh_view → swipe_view → row) and interactions (the swipe item's
// Invoked → readout, SwipeEnded → readout, the refresh Command → counter) without a hosting main.
#include "examples/gallery/pages/swipe_refresh_page.hpp"

#include "maui/core/swipe_mode.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::samples::swipe_refresh_page;

    TEST(swipe_refresh_page, builds_the_tree)
    {
        swipe_refresh_page demo;
        EXPECT_EQ(demo.page().content(), &demo.refresh());
        // The refresh_view now hosts a column holding BOTH the swipe row and the readout (so the readout
        // stays visible), instead of the bare swipe_view.
        EXPECT_EQ(demo.refresh().content(), &demo.column());
        EXPECT_EQ(demo.column().count(), 2);
        EXPECT_EQ(&demo.column().at(0), &demo.swipe());
        EXPECT_EQ(&demo.column().at(1), &demo.readout());
        EXPECT_EQ(demo.swipe().content(), &demo.row());
        EXPECT_EQ(demo.swipe().right_items_collection().count(), 1U);
        EXPECT_EQ(demo.swipe().right_items_collection().mode(), maui::core::swipe_mode::execute);
    }

    TEST(swipe_refresh_page, invoking_the_swipe_item_updates_the_readout)
    {
        swipe_refresh_page demo;
        EXPECT_EQ(demo.readout().text(), "Ready");
        demo.delete_item().on_invoked();
        EXPECT_EQ(demo.readout().text(), "Deleted");
    }

    TEST(swipe_refresh_page, refreshing_bumps_the_counter)
    {
        swipe_refresh_page demo;
        EXPECT_EQ(demo.refresh_count(), 0);
        demo.refresh().set_is_refreshing(true); // raises Refreshing + runs the command
        EXPECT_EQ(demo.refresh_count(), 1);
        EXPECT_FALSE(demo.refresh().is_refreshing()); // the command ended the spinner
    }
} // namespace
