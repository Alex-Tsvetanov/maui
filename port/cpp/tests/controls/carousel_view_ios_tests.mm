// iOS (UIKit, on-simulator) backend tests for carousel_view — the carousel reuses the W3-29
// collection_view_handler (the Items2 COMPOSITIONAL path over a REAL UICollectionView), so these
// assert the genuine native stack the carousel rides on:
//   - the carousel's native view is a UICollectionView and realizes cells after a layout pass;
//   - a horizontal carousel lays its items out along the horizontal axis (the snap layout default);
//   - a Position change funnels a CENTERED scroll-to through the handler onto the native content offset
//     (CarouselViewHandler2.MapPosition → ScrollToPosition, the carousel snap alignment).
//
// The run loop is pumped via tests/support/run_loop_pump.hpp so the compositional layout + cell
// realization run deterministically. Compiled as Objective-C++ with ARC for the `ios` backend; run ON
// the booted simulator via tools/ios-sim-run.sh. §8: the items collection (publisher) is declared
// before the view/handler (subscriber).
//
// DOCUMENTED DEVIATION: the carousel reuses the collection_view_handler, whose iOS compositional path
// (W3-29) carries NO phantom-cell loop wrap — Loop is honored as control state + the centered snap
// scroll, but the native UICollectionView shows ItemCount cells (not the CarouselViewController2
// LoopCount = ItemCount + 2 phantom-cell wrap). The looping SNAP/position behavior is exercised; the
// infinite-wrap visual is a documented carry-over of W3-29's collection handler scope.
#import <UIKit/UIKit.h>

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/items/carousel_view.hpp"
#include "maui/controls/items/collection_view_handler.hpp"
#include "maui/controls/items/item_collection.hpp"
#include "maui/controls/scroll_to_position.hpp"
#include "maui/core/observable_collection.hpp"
#include "tests/support/run_loop_pump.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::carousel_view;
    using maui::controls::collection_view_handler;
    using maui::core::observable_collection;
    using maui::tests::pump_run_loop;
    using maui::tests::pump_until;

    using string_collection = observable_collection<std::string>;

    UICollectionView* native_collection_view(const std::shared_ptr<collection_view_handler>& handler)
    {
        return (__bridge UICollectionView*)handler->native_view();
    }

    UIWindow* make_host_window()
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        UIWindow* const window = [[UIWindow alloc] init]; // SDK-deprecated; see window_handler.mm precedent
#pragma clang diagnostic pop
        return window;
    }

    std::vector<std::string> make_alphabet(int count)
    {
        std::vector<std::string> values;
        values.reserve(static_cast<std::size_t>(count));
        for (int i = 0; i < count; ++i)
        {
            values.push_back("item-" + std::to_string(i));
        }
        return values;
    }

    // The carousel rig: 12 items on a horizontal carousel (the default snap layout). The collection
    // (publisher) is declared FIRST (§8).
    struct rig
    {
        std::shared_ptr<string_collection> items; // publisher FIRST (§8)
        carousel_view view;
        std::shared_ptr<collection_view_handler> handler = std::make_shared<collection_view_handler>();

        explicit rig(std::vector<std::string> initial = make_alphabet(12))
            : items(std::make_shared<string_collection>(std::move(initial)))
        {
            view.set_items_source(items);
            view.set_handler(handler);
        }

        UIWindow* mount(double width = 300, double height = 200) const
        {
            UIWindow* const window = make_host_window();
            UICollectionView* const collection_view = native_collection_view(handler);
            [window addSubview:collection_view];
            [window makeKeyAndVisible];
            handler->native_force_layout(width, height);
            pump_until([&] { return handler->native_visible_cell_count() > 0; });
            return window;
        }
    };

    TEST(carousel_view_ios, native_view_is_a_uicollectionview)
    {
        const rig r;
        EXPECT_TRUE([native_collection_view(r.handler) isKindOfClass:[UICollectionView class]]);
    }

    TEST(carousel_view_ios, data_realizes_cells_after_layout)
    {
        const rig r;
        UIWindow* const window = r.mount();
        EXPECT_GT(r.handler->native_visible_cell_count(), 0);
        EXPECT_EQ([native_collection_view(r.handler) numberOfItemsInSection:0], 12);
        (void)window;
    }

    // A Position change drives a CENTERED native scroll-to (the offset moves toward the target item).
    // Scroll animation is disabled so the offset updates synchronously (the W3-29 collection scroll
    // test precedent uses animate=false for determinism; AnimatePositionChanges tracks IsScrollAnimated).
    TEST(carousel_view_ios, position_change_scrolls_the_native_offset)
    {
        rig r;
        r.view.set_is_scroll_animated(false);
        UIWindow* const window = r.mount();
        UICollectionView* const collection_view = native_collection_view(r.handler);
        const CGFloat start_x = collection_view.contentOffset.x;

        r.view.set_position(8); // → scroll_to(8, center, animate=false) → native_scroll_to
        pump_until([&] { return collection_view.contentOffset.x > start_x; });
        [collection_view layoutIfNeeded];

        EXPECT_GT(collection_view.contentOffset.x, start_x)
            << "a Position change should move the carousel's native horizontal offset";
        EXPECT_EQ(r.view.position(), 8);
        (void)window;
    }

    // Loop is honored as control state even though the native compositional path carries no phantom
    // wrap (documented deviation): the carousel still tracks Loop and stays alive through a scroll.
    TEST(carousel_view_ios, loop_state_survives_a_scroll)
    {
        rig r;
        UIWindow* const window = r.mount();
        EXPECT_TRUE(r.view.loop());
        r.view.set_position(5);
        pump_run_loop(0.2);
        EXPECT_TRUE(r.view.loop());
        (void)window;
    }
} // namespace
