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
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/collection_view_handler.hpp"
#include "maui/controls/items/item_collection.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/items/linear_items_layout.hpp"
#include "maui/controls/scroll_to_position.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/thickness.hpp"
#include "tests/support/run_loop_pump.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::carousel_view;
    using maui::controls::collection_view;
    using maui::controls::collection_view_handler;
    using maui::controls::items_layout_orientation;
    using maui::controls::linear_items_layout;
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

    // A simulated user scroll-and-settle writes Position + CurrentItem BACK to the carousel
    // (CarouselViewController2.SetPosition/SetCurrentItem → SetValueFromRenderer, driven from the
    // UIScrollViewDelegate scroll-end callback when the centered item changes). We push the native
    // contentOffset to center a later item, then deliver the scroll-end callback the same way UIKit
    // would after deceleration; the carousel's Position must follow.
    TEST(carousel_view_ios, user_scroll_writes_position_and_current_item_back)
    {
        rig r;
        r.view.set_is_scroll_animated(false);
        UIWindow* const window = r.mount();
        UICollectionView* const collection_view = native_collection_view(r.handler);
        EXPECT_EQ(r.view.position(), 0);

        // Scroll the native carousel so a later item is centered, then end deceleration (the user-gesture
        // settle the delegate listens for). Center item 4 by scrolling it to the horizontal center.
        NSIndexPath* const target = [NSIndexPath indexPathForItem:4 inSection:0];
        [collection_view scrollToItemAtIndexPath:target
                                atScrollPosition:UICollectionViewScrollPositionCenteredHorizontally
                                        animated:NO];
        [collection_view layoutIfNeeded];
        // Deliver the scroll-end callback (the UIScrollViewDelegate hook the controller now implements).
        [(id<UIScrollViewDelegate>)collection_view.delegate scrollViewDidEndDecelerating:collection_view];
        pump_until([&] { return r.view.position() == 4; });

        EXPECT_EQ(r.view.position(), 4) << "a user scroll-and-settle should write Position back";
        EXPECT_EQ(r.view.current_item().text(), "item-4") << "a user scroll-and-settle should write CurrentItem back";
        (void)window;
    }

    // IsSwipeEnabled drives the native UICollectionView.scrollEnabled
    // (CarouselViewHandler2.MapIsSwipeEnabled → CollectionView.ScrollEnabled).
    TEST(carousel_view_ios, is_swipe_enabled_drives_native_scroll_enabled)
    {
        rig r;
        UIWindow* const window = r.mount();
        UICollectionView* const collection_view = native_collection_view(r.handler);

        EXPECT_TRUE(collection_view.scrollEnabled); // default true
        r.view.set_is_swipe_enabled(false);
        EXPECT_FALSE(collection_view.scrollEnabled);
        r.view.set_is_swipe_enabled(true);
        EXPECT_TRUE(collection_view.scrollEnabled);
        (void)window;
    }

    // IsBounceEnabled drives the native UICollectionView.bounces
    // (CarouselViewHandler2.MapIsBounceEnabled → CollectionView.Bounces).
    TEST(carousel_view_ios, is_bounce_enabled_drives_native_bounces)
    {
        rig r;
        UIWindow* const window = r.mount();
        UICollectionView* const collection_view = native_collection_view(r.handler);

        EXPECT_TRUE(collection_view.bounces); // default true
        r.view.set_is_bounce_enabled(false);
        EXPECT_FALSE(collection_view.bounces);
        r.view.set_is_bounce_enabled(true);
        EXPECT_TRUE(collection_view.bounces);
        (void)window;
    }

    // PeekAreaInsets non-zero adjusts the native layout insets so the first cell starts shifted in by
    // the leading peek (CarouselViewHandler2.MapPeekAreaInsets → UpdateLayout → section/content insets).
    // The carousel is horizontal, so the left peek inset shifts the first cell's origin rightward.
    TEST(carousel_view_ios, peek_area_insets_shift_the_first_cell_origin)
    {
        rig r;
        UIWindow* const window = r.mount();
        UICollectionView* const collection_view = native_collection_view(r.handler);

        NSIndexPath* const first = [NSIndexPath indexPathForItem:0 inSection:0];
        UICollectionViewLayoutAttributes* const before = [collection_view layoutAttributesForItemAtIndexPath:first];
        const CGFloat origin_before = before.frame.origin.x;

        r.view.set_peek_area_insets(maui::core::thickness{40, 0, 40, 0}); // left/right peek
        [collection_view layoutIfNeeded];
        pump_run_loop(0.1);
        UICollectionViewLayoutAttributes* const after = [collection_view layoutAttributesForItemAtIndexPath:first];
        const CGFloat origin_after = after.frame.origin.x;

        EXPECT_GT(origin_after, origin_before)
            << "a non-zero left peek inset should shift the first cell's origin rightward";
        (void)window;
    }

    // THE CAROUSEL ITEM-SIZING CONTRACT (LayoutFactory2.CreateCarouselLayout, the gap this slice closes):
    // each item fills the carousel VIEWPORT on the scroll axis — one full-width item per page, swipeable
    // one-at-a-time — NOT its intrinsic label width laid side-by-side. The horizontal carousel's first
    // cell frame width must equal the viewport width (no peek here), proving the full-viewport item.
    TEST(carousel_view_ios, item_fills_the_viewport_width_one_per_page)
    {
        const rig r;
        const CGFloat viewport_width = 300;
        UIWindow* const window = r.mount(viewport_width, 200);
        UICollectionView* const collection_view = native_collection_view(r.handler);

        NSIndexPath* const first = [NSIndexPath indexPathForItem:0 inSection:0];
        UICollectionViewLayoutAttributes* const attrs = [collection_view layoutAttributesForItemAtIndexPath:first];
        ASSERT_NE(attrs, nil);
        EXPECT_NEAR(attrs.frame.size.width, viewport_width, 0.5)
            << "a carousel item must fill the full viewport width (one item per page), not its intrinsic "
               "label width";
    }

    // With a non-zero peek the carousel item fills viewport-MINUS-peek (the adjacent items peek in). The
    // section's leading+trailing peek insets shrink the page, so the full-viewport item narrows to match.
    TEST(carousel_view_ios, item_fills_the_viewport_minus_peek)
    {
        rig r;
        const CGFloat viewport_width = 300;
        const CGFloat peek = 40; // left + right (CarouselViewHandler2.MapPeekAreaInsets)
        UIWindow* const window = r.mount(viewport_width, 200);
        UICollectionView* const collection_view = native_collection_view(r.handler);

        r.view.set_peek_area_insets(maui::core::thickness{peek, 0, peek, 0});
        [collection_view layoutIfNeeded];
        pump_run_loop(0.1);

        NSIndexPath* const first = [NSIndexPath indexPathForItem:0 inSection:0];
        UICollectionViewLayoutAttributes* const attrs = [collection_view layoutAttributesForItemAtIndexPath:first];
        ASSERT_NE(attrs, nil);
        EXPECT_NEAR(attrs.frame.size.width, viewport_width - (peek * 2), 0.5)
            << "a carousel item must fill the viewport minus the leading+trailing peek";
        (void)window;
    }

    // GUARD (no regression): a REGULAR horizontal collection_view must keep INTRINSIC-width items (the
    // CreateListLayout estimated extent), NOT full-viewport — only the carousel snaps one-item-per-page.
    // The default cells mirror short item text, so an intrinsic-width cell is far narrower than the 300pt
    // viewport. Mirrors the carousel rig but over a plain collection_view with a horizontal linear layout.
    TEST(carousel_view_ios, regular_horizontal_collection_keeps_intrinsic_width_items)
    {
        auto items = std::make_shared<string_collection>(make_alphabet(12)); // publisher FIRST (§8)
        collection_view plain;
        auto plain_handler = std::make_shared<collection_view_handler>();
        plain.set_items_layout(std::make_shared<linear_items_layout>(items_layout_orientation::horizontal));
        plain.set_items_source(items);
        plain.set_handler(plain_handler);

        const CGFloat viewport_width = 300;
        UIWindow* const window = make_host_window();
        UICollectionView* const collection_view = (__bridge UICollectionView*)plain_handler->native_view();
        [window addSubview:collection_view];
        [window makeKeyAndVisible];
        plain_handler->native_force_layout(viewport_width, 200);
        pump_until([&] { return plain_handler->native_visible_cell_count() > 0; });

        NSIndexPath* const first = [NSIndexPath indexPathForItem:0 inSection:0];
        UICollectionViewLayoutAttributes* const attrs = [collection_view layoutAttributesForItemAtIndexPath:first];
        ASSERT_NE(attrs, nil);
        EXPECT_LT(attrs.frame.size.width, viewport_width - 1)
            << "a regular horizontal collection_view item must keep its intrinsic width, NOT fill the "
               "viewport like a carousel";
        (void)window;
    }
} // namespace
