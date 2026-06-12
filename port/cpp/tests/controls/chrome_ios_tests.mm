// iOS (UIKit) backend tests for the W1-11 chrome — run ON the simulator for MAUI_BACKEND=ios:
//   - the navigation bar materializes the page toolbar items as REAL UIButtons on the custom bar
//     (C#'s UINavigationBar rightBarButtonItems path) and a tap routes back to `clicked`;
//   - a context flyout ATTACHES a real UIContextMenuInteraction to the view (menu materialization
//     needs the user's long-press interaction — attach-only by design, documented);
//   - the window menu bar / title bar stay MIRROR-ONLY (stored-inert) and the tooltip is a documented
//     no-op — C# materializes menus/tooltips/title bars on desktop (Windows / Mac Catalyst) only.
// Compiled as Objective-C++ with ARC.
#import <UIKit/UIKit.h>

#include <memory>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/menu_bar_item.hpp"
#include "maui/controls/menu_flyout.hpp"
#include "maui/controls/menu_flyout_item.hpp"
#include "maui/controls/navigation_page.hpp"
#include "maui/controls/title_bar.hpp"
#include "maui/controls/tool_tip_properties.hpp"
#include "maui/controls/toolbar_item.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/i_menu_bar.hpp"
#include "maui/core/navigation_page_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/window_handler.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::button;
    using maui::controls::content_page;
    using maui::controls::menu_bar_item;
    using maui::controls::menu_flyout;
    using maui::controls::menu_flyout_item;
    using maui::controls::navigation_page;
    using maui::controls::title_bar;
    using maui::controls::tool_tip_properties;
    using maui::controls::toolbar_item;
    using maui::controls::window;
    using maui::core::button_handler;
    using maui::core::navigation_page_handler;
    using maui::core::window_handler;

    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    NSArray<UIButton*>* bar_buttons(const std::shared_ptr<navigation_page_handler>& handler)
    {
        return (__bridge NSArray<UIButton*>*)handler->typed_platform_view()->toolbar_buttons;
    }

    UIView* bar_view(const std::shared_ptr<navigation_page_handler>& handler)
    {
        return (__bridge UIView*)handler->typed_platform_view()->bar;
    }

    UIView* native_view_of(const std::shared_ptr<button_handler>& handler)
    {
        return (__bridge UIView*)handler->native_view();
    }

    // Replicates -[UIControl sendActionsForControlEvents:]'s dispatch-table walk for one event — the
    // spawned test process has no UIApplication to deliver the action through (the same helper the
    // button seam tests use; see button_ios_tests.mm).
    void send_control_event(UIControl* control, UIControlEvents event)
    {
        NSArray* const targets = control.allTargets.allObjects;
        for (NSUInteger t = 0; t < targets.count; ++t)
        {
            id const target = targets[t];
            NSArray<NSString*>* const actions = [control actionsForTarget:target forControlEvent:event];
            for (NSUInteger a = 0; a < actions.count; ++a)
            {
                SEL const action = NSSelectorFromString(actions[a]);
                NSMethodSignature* const signature = [target methodSignatureForSelector:action];
                ASSERT_NE(signature, nil);
                NSInvocation* const invocation = [NSInvocation invocationWithMethodSignature:signature];
                invocation.selector = action;
                id sender = control;
                [invocation setArgument:&sender atIndex:2]; // 0 = self, 1 = _cmd, 2 = the sender
                [invocation invokeWithTarget:target];
            }
        }
    }

    // ---- navigation bar: real toolbar buttons ----

    TEST(ios_chrome, toolbar_items_materialize_as_bar_buttons)
    {
        toolbar_item save("Save", "", [] {});
        content_page root;
        navigation_page nav(root);
        nav.toolbar_items().add(save);

        auto handler = std::make_shared<navigation_page_handler>();
        nav.set_handler(handler);

        NSArray<UIButton*>* const buttons = bar_buttons(handler);
        ASSERT_NE(buttons, nil);
        ASSERT_EQ(buttons.count, 1U);
        EXPECT_EQ(to_std_string([buttons[0] titleForState:UIControlStateNormal]), "Save");
        // The button lives ON the custom bar.
        EXPECT_EQ(buttons[0].superview, bar_view(handler));
        // The mirror agrees.
        ASSERT_EQ(handler->typed_platform_view()->toolbar_items.size(), 1U);
    }

    TEST(ios_chrome, bar_button_tap_routes_to_the_control)
    {
        bool fired = false;
        toolbar_item save("Save", "", [&fired] { fired = true; });
        content_page root;
        navigation_page nav(root);
        nav.toolbar_items().add(save);

        auto handler = std::make_shared<navigation_page_handler>();
        nav.set_handler(handler);

        NSArray<UIButton*>* const buttons = bar_buttons(handler);
        ASSERT_EQ(buttons.count, 1U);
        send_control_event(buttons[0], UIControlEventTouchUpInside);

        EXPECT_TRUE(fired);
    }

    TEST(ios_chrome, pushing_a_page_with_items_rebuilds_the_bar_buttons)
    {
        toolbar_item nav_item("NavItem", "", [] {});
        toolbar_item page_item("PageItem", "", [] {});
        content_page root;
        navigation_page nav(root);
        nav.toolbar_items().add(nav_item);

        auto handler = std::make_shared<navigation_page_handler>();
        nav.set_handler(handler);
        ASSERT_EQ(bar_buttons(handler).count, 1U);

        content_page second;
        second.toolbar_items().add(page_item);
        nav.push(second);
        ASSERT_EQ(bar_buttons(handler).count, 2U);

        nav.pop();
        ASSERT_EQ(bar_buttons(handler).count, 1U);
        EXPECT_EQ(to_std_string([bar_buttons(handler)[0] titleForState:UIControlStateNormal]), "NavItem");
    }

    // ---- context flyout: the UIContextMenuInteraction ATTACH (materialization needs interaction) ----

    // The number of UIContextMenuInteractions on the view — counted by TYPE, not by total interaction
    // count: attaching one can make UIKit add unrelated system interactions of its own alongside it.
    NSUInteger context_menu_interaction_count(UIView* view)
    {
        NSUInteger count = 0;
        NSArray<id<UIInteraction>>* const interactions = view.interactions;
        for (NSUInteger i = 0; i < interactions.count; ++i)
        {
            if ([interactions[i] isKindOfClass:[UIContextMenuInteraction class]])
            {
                ++count;
            }
        }
        return count;
    }

    TEST(ios_chrome, context_flyout_attaches_a_context_menu_interaction)
    {
        button host;
        menu_flyout flyout;
        menu_flyout_item copy_item;
        copy_item.set_text("Copy");
        flyout.items().add(copy_item);

        auto handler = std::make_shared<button_handler>();
        host.set_handler(handler);
        UIView* const native = native_view_of(handler);
        const NSUInteger baseline = context_menu_interaction_count(native);

        host.set_context_flyout(&flyout);
        EXPECT_EQ(context_menu_interaction_count(native), baseline + 1);

        // Clearing removes the interaction again.
        host.set_context_flyout(nullptr);
        EXPECT_EQ(context_menu_interaction_count(native), baseline);
    }

    // ---- the documented iOS no-ops: tooltip + window menu bar / title bar stay mirrors ----

    TEST(ios_chrome, tool_tip_is_a_documented_no_op_with_an_observable_mirror)
    {
        button host;
        auto handler = std::make_shared<button_handler>();
        host.set_handler(handler);

        tool_tip_properties::set_text(host, "No hover here");
        auto* base = handler->platform_base();
        ASSERT_NE(base, nullptr);
        // The mirror records it; no native surface on iOS.
        EXPECT_EQ(base->tool_tip, std::optional<std::string>("No hover here"));
    }

    TEST(ios_chrome, window_menu_bar_and_title_bar_stay_stored_inert)
    {
        menu_bar_item file_menu;
        file_menu.set_text("File");
        content_page page;
        page.menu_bar_items().add(file_menu);

        window host(page);
        auto handler = std::make_shared<window_handler>();
        host.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        // The aggregate is stored (observable) but nothing native materializes on plain iOS (C# builds
        // menu bars on desktop/Catalyst only; TitleBar maps on Windows + Mac Catalyst only).
        ASSERT_NE(platform->hosted_menu_bar, nullptr);
        EXPECT_EQ(platform->hosted_menu_bar->item_count(), 1U);

        title_bar bar;
        bar.set_title("Inert");
        host.set_title_bar(&bar);
        EXPECT_EQ(platform->hosted_title_bar, &bar);
    }
} // namespace
