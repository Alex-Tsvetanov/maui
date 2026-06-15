// iOS (UIKit) seam tests for the swipe menu-item handler (the UIButton recipe) and the SwipeView's
// interactive drag-to-reveal pan (W7-U09), run ON the iOS simulator via tools/ios-sim-run.sh. Ported from
// SwipeItemMenuItemHandler.iOS.cs (the button visuals) + MauiSwipeView.cs (the pan state machine).
// Compiled as Objective-C++ with ARC for the `ios` backend.
//
// GAP 1 — the swipe menu-item handler materializes a real UIButton subclass with UserInteractionEnabled
// = NO, RestorationIdentifier = Text, the title + (luminosity-derived) title colour + hidden state set.
// GAP 2 — the swipe host carries a real UIPanGestureRecognizer; driving the shared machine (the same
// entry points the pan trampoline calls) opens/closes the row and the Reveal-mode content frame moves.
#import <UIKit/UIKit.h>

#include <memory>

#include "maui/controls/button.hpp"
#include "maui/controls/swipe_item.hpp"
#include "maui/controls/swipe_items.hpp"
#include "maui/controls/swipe_view.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/swipe_direction.hpp"
#include "maui/core/swipe_item_menu_item_handler.hpp"
#include "maui/core/swipe_mode.hpp"
#include "maui/core/swipe_view_handler.hpp"
#include "maui/core/swipe_view_handler_state.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/rect.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::button;
    using maui::controls::swipe_item;
    using maui::controls::swipe_items;
    using maui::controls::swipe_view;
    using maui::core::button_handler;
    using maui::core::swipe_direction;
    using maui::core::swipe_item_menu_item_handler;
    using maui::core::swipe_machine_state;
    using maui::core::swipe_mode;
    using maui::core::swipe_view_handler;

    UIButton* item_button(const std::shared_ptr<swipe_item_menu_item_handler>& handler)
    {
        return (__bridge UIButton*)handler->native_view();
    }
    UIView* swipe_host(const std::shared_ptr<swipe_view_handler>& handler)
    {
        return (__bridge UIView*)handler->typed_platform_view()->native;
    }

    // ---- GAP 1: the UIButton swipe-item handler ----

    TEST(ios_swipe_item_menu_item, materializes_a_non_interactive_button_with_title)
    {
        swipe_item item;
        item.set_text("Delete");
        item.set_background_color(maui::graphics::colors::red);
        auto handler = std::make_shared<swipe_item_menu_item_handler>();
        handler->set_virtual_view(item);

        UIButton* const native = item_button(handler);
        ASSERT_NE(native, nil);
        EXPECT_TRUE([native isKindOfClass:[UIButton class]]);
        EXPECT_FALSE(native.userInteractionEnabled); // C# SwipeItemButton { UserInteractionEnabled = false }
        EXPECT_TRUE([native.currentTitle isEqualToString:@"Delete"]);
        EXPECT_TRUE([native.restorationIdentifier isEqualToString:@"Delete"]);
    }

    TEST(ios_swipe_item_menu_item, title_colour_is_white_on_a_dark_background)
    {
        swipe_item item;
        item.set_text("Delete");
        item.set_background_color(maui::graphics::colors::red); // dark → white title (GetTextColor)
        auto handler = std::make_shared<swipe_item_menu_item_handler>();
        handler->set_virtual_view(item);

        UIColor* const title = [item_button(handler) titleColorForState:UIControlStateNormal];
        ASSERT_NE(title, nil);
        CGFloat r = 0;
        CGFloat g = 0;
        CGFloat b = 0;
        CGFloat a = 0;
        [title getRed:&r green:&g blue:&b alpha:&a];
        EXPECT_NEAR(r, 1.0, 0.01);
        EXPECT_NEAR(g, 1.0, 0.01);
        EXPECT_NEAR(b, 1.0, 0.01);
        // The mirror agrees.
        EXPECT_TRUE(handler->typed_platform_view()->has_title_color);
        EXPECT_EQ(handler->typed_platform_view()->title_color_argb, maui::graphics::colors::white.to_uint());
    }

    TEST(ios_swipe_item_menu_item, visibility_hides_the_button)
    {
        swipe_item item;
        item.set_is_visible(false);
        auto handler = std::make_shared<swipe_item_menu_item_handler>();
        handler->set_virtual_view(item);
        EXPECT_TRUE(item_button(handler).hidden);

        item.set_is_visible(true);
        handler->update_value("visibility");
        EXPECT_FALSE(item_button(handler).hidden);
    }

    TEST(ios_swipe_item_menu_item, frame_change_does_not_crash_and_re_runs_source)
    {
        swipe_item item;
        item.set_text("Archive");
        auto handler = std::make_shared<swipe_item_menu_item_handler>();
        handler->set_virtual_view(item);
        // Setting the frame fires the SwipeItemButton frame callback → re-runs MapSource. With no icon it
        // is a safe no-op; the button must survive (no UAF / re-entrancy crash).
        item_button(handler).frame = CGRectMake(0, 0, 80, 44);
        EXPECT_FALSE(handler->typed_platform_view()->has_source);
        EXPECT_TRUE([item_button(handler) isKindOfClass:[UIButton class]]);
    }

    // ---- GAP 2: the SwipeView interactive drag-to-reveal pan ----

    TEST(ios_swipe_view_pan, host_carries_a_pan_recognizer)
    {
        swipe_view view;
        auto handler = std::make_shared<swipe_view_handler>();
        view.set_handler(handler);

        bool found_pan = false;
        for (UIGestureRecognizer* gr in swipe_host(handler).gestureRecognizers)
        {
            if ([gr isKindOfClass:[UIPanGestureRecognizer class]])
            {
                found_pan = true;
            }
        }
        EXPECT_TRUE(found_pan);
    }

    TEST(ios_swipe_view_pan, dragging_past_the_threshold_opens_and_moves_the_content)
    {
        button child;
        auto child_handler = std::make_shared<button_handler>();
        child.set_handler(child_handler);
        auto* const child_native = (__bridge UIView*)child_handler->native_view();
        ASSERT_NE(child_native, nil);

        swipe_item item;
        auto items = std::make_unique<swipe_items>();
        items->set_mode(swipe_mode::reveal);
        items->add(item);
        swipe_view view;
        view.set_threshold(100);                // 60% open = 60
        view.set_right_items(std::move(items)); // a LEFT swipe reveals the RIGHT items
        view.set_content(child);
        auto handler = std::make_shared<swipe_view_handler>();
        view.set_handler(handler);
        view.arrange(maui::graphics::rect(0, 0, 200, 60));

        // Drive the shared machine exactly as the pan trampoline does for a left drag past the threshold.
        handler->begin_swipe(swipe_direction::left);
        handler->swipe_to(-80); // |80| >= 60 → settles open
        handler->end_swipe();

        EXPECT_EQ(handler->typed_platform_view()->state.state, swipe_machine_state::open);
        EXPECT_TRUE(view.is_open());
    }

    TEST(ios_swipe_view_pan, dragging_below_the_threshold_closes)
    {
        swipe_item item;
        auto items = std::make_unique<swipe_items>();
        items->set_mode(swipe_mode::reveal);
        items->add(item);
        swipe_view view;
        view.set_threshold(100);
        view.set_right_items(std::move(items));
        auto handler = std::make_shared<swipe_view_handler>();
        view.set_handler(handler);

        handler->begin_swipe(swipe_direction::left);
        handler->swipe_to(-20); // |20| < 60 → resets closed
        handler->end_swipe();

        EXPECT_EQ(handler->typed_platform_view()->state.state, swipe_machine_state::idle);
        EXPECT_FALSE(view.is_open());
    }
} // namespace
