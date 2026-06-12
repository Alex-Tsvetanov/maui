// Apple (AppKit) backend tests for the W1-11 window/view chrome — the real-native half, run only for
// MAUI_BACKEND=apple:
//   - the menu bar materializes as a REAL NSMenu (top-level titles, submenu items, the separator, the
//     enabled state, the keyEquivalent, and the click routing back to `clicked`);
//   - the window toolbar materializes as a REAL NSToolbar whose delegate serves the primary items (+
//     the secondary overflow NSMenuToolbarItem) — asserted through the delegate (NSToolbar realizes
//     items lazily on display) — and an item "click" routes to `clicked`;
//   - a context flyout becomes the view's NSView.menu (assert the NSMenu contents); a tooltip becomes
//     NSView.toolTip;
//   - a title bar becomes an NSTitlebarAccessoryViewController (replace + clear included).
// Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>

#include <memory>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/menu_bar_item.hpp"
#include "maui/controls/menu_flyout.hpp"
#include "maui/controls/menu_flyout_item.hpp"
#include "maui/controls/menu_flyout_separator.hpp"
#include "maui/controls/menu_flyout_sub_item.hpp"
#include "maui/controls/navigation_page.hpp"
#include "maui/controls/title_bar.hpp"
#include "maui/controls/tool_tip_properties.hpp"
#include "maui/controls/toolbar_item.hpp"
#include "maui/controls/toolbar_item_order.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/keyboard_accelerator.hpp"
#include "maui/core/window_handler.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::button;
    using maui::controls::content_page;
    using maui::controls::menu_bar_item;
    using maui::controls::menu_flyout;
    using maui::controls::menu_flyout_item;
    using maui::controls::menu_flyout_separator;
    using maui::controls::menu_flyout_sub_item;
    using maui::controls::navigation_page;
    using maui::controls::title_bar;
    using maui::controls::tool_tip_properties;
    using maui::controls::toolbar_item;
    using maui::controls::toolbar_item_order;
    using maui::controls::window;
    using maui::core::button_handler;
    using maui::core::keyboard_accelerator;
    using maui::core::keyboard_accelerator_modifiers;
    using maui::core::window_handler;

    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    NSWindow* native_window(const std::shared_ptr<window_handler>& handler)
    {
        return (__bridge NSWindow*)handler->typed_platform_view()->native;
    }

    NSMenu* built_main_menu(const std::shared_ptr<window_handler>& handler)
    {
        return (__bridge NSMenu*)handler->typed_platform_view()->chrome_main_menu;
    }

    // NSWindow creation needs the shared application object (no run loop required).
    class apple_chrome : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    // ---- menu bar → a real NSMenu ----

    TEST_F(apple_chrome, menu_bar_materializes_as_nsmenu_with_items_separator_and_submenu)
    {
        menu_bar_item file_menu;
        file_menu.set_text("File");
        menu_flyout_item open_item;
        open_item.set_text("Open");
        open_item.accelerators().push_back(
            keyboard_accelerator{.modifiers = keyboard_accelerator_modifiers::cmd, .key = "O"});
        menu_flyout_item disabled_item;
        disabled_item.set_text("Disabled");
        disabled_item.set_is_enabled(false);
        menu_flyout_separator separator;
        menu_flyout_sub_item recent;
        recent.set_text("Recent");
        menu_flyout_item first;
        first.set_text("First");
        recent.items().add(first);
        file_menu.items().add(open_item);
        file_menu.items().add(disabled_item);
        file_menu.items().add(separator);
        file_menu.items().add(recent);

        content_page page;
        page.menu_bar_items().add(file_menu);
        window host(page);
        auto handler = std::make_shared<window_handler>();
        host.set_handler(handler);

        NSMenu* const main = built_main_menu(handler);
        ASSERT_NE(main, nil);
        ASSERT_EQ(main.numberOfItems, 1);
        NSMenuItem* const top = [main itemAtIndex:0];
        EXPECT_EQ(to_std_string(top.title), "File");
        NSMenu* const drop_down = top.submenu;
        ASSERT_NE(drop_down, nil);
        ASSERT_EQ(drop_down.numberOfItems, 4);
        EXPECT_EQ(to_std_string([drop_down itemAtIndex:0].title), "Open");
        EXPECT_EQ(to_std_string([drop_down itemAtIndex:0].keyEquivalent), "o");
        EXPECT_EQ([drop_down itemAtIndex:0].keyEquivalentModifierMask & NSEventModifierFlagCommand,
                  NSEventModifierFlagCommand);
        EXPECT_FALSE([drop_down itemAtIndex:1].enabled);
        EXPECT_TRUE([drop_down itemAtIndex:2].separatorItem);
        NSMenuItem* const sub = [drop_down itemAtIndex:3];
        ASSERT_NE(sub.submenu, nil);
        ASSERT_EQ(sub.submenu.numberOfItems, 1);
        EXPECT_EQ(to_std_string([sub.submenu itemAtIndex:0].title), "First");
    }

    TEST_F(apple_chrome, nsmenu_item_click_routes_to_the_control)
    {
        menu_bar_item file_menu;
        file_menu.set_text("File");
        menu_flyout_item open_item;
        open_item.set_text("Open");
        file_menu.items().add(open_item);
        bool clicked = false;
        open_item.clicked.connect([&clicked] { clicked = true; });

        content_page page;
        page.menu_bar_items().add(file_menu);
        window host(page);
        auto handler = std::make_shared<window_handler>();
        host.set_handler(handler);

        NSMenu* const drop_down = [built_main_menu(handler) itemAtIndex:0].submenu;
        // Drive the target-action exactly as NSMenu would on selection.
        [drop_down performActionForItemAtIndex:0];

        EXPECT_TRUE(clicked);
    }

    // ---- window toolbar → a real NSToolbar (items served by its delegate) ----

    TEST_F(apple_chrome, toolbar_materializes_with_primary_items_and_secondary_overflow)
    {
        toolbar_item save("Save", "", [] {}, toolbar_item_order::primary, 0);
        toolbar_item about("About", "", [] {}, toolbar_item_order::secondary, 1);

        content_page root;
        navigation_page nav(root);
        nav.toolbar_items().add(save);
        nav.toolbar_items().add(about);

        window host;
        host.set_content(nav);
        auto handler = std::make_shared<window_handler>();
        host.set_handler(handler);

        NSToolbar* const toolbar = native_window(handler).toolbar;
        ASSERT_NE(toolbar, nil);
        id<NSToolbarDelegate> const delegate = toolbar.delegate;
        ASSERT_NE(delegate, nil);
        NSArray<NSToolbarItemIdentifier>* const identifiers = [delegate toolbarDefaultItemIdentifiers:toolbar];
        ASSERT_EQ(identifiers.count, 2U); // Save + the overflow item
        NSToolbarItem* const first = [delegate toolbar:toolbar
                                 itemForItemIdentifier:identifiers[0]
                             willBeInsertedIntoToolbar:YES];
        ASSERT_NE(first, nil);
        EXPECT_EQ(to_std_string(first.label), "Save");
        NSToolbarItem* const overflow = [delegate toolbar:toolbar
                                    itemForItemIdentifier:identifiers[1]
                                willBeInsertedIntoToolbar:YES];
        ASSERT_TRUE([overflow isKindOfClass:[NSMenuToolbarItem class]]);
        NSMenu* const overflow_menu = ((NSMenuToolbarItem*)overflow).menu;
        ASSERT_EQ(overflow_menu.numberOfItems, 1);
        EXPECT_EQ(to_std_string([overflow_menu itemAtIndex:0].title), "About");
    }

    TEST_F(apple_chrome, toolbar_item_click_routes_to_the_control)
    {
        bool fired = false;
        toolbar_item save("Save", "", [&fired] { fired = true; });

        content_page root;
        navigation_page nav(root);
        nav.toolbar_items().add(save);
        window host;
        host.set_content(nav);
        auto handler = std::make_shared<window_handler>();
        host.set_handler(handler);

        NSToolbar* const toolbar = native_window(handler).toolbar;
        id<NSToolbarDelegate> const delegate = toolbar.delegate;
        NSArray<NSToolbarItemIdentifier>* const identifiers = [delegate toolbarDefaultItemIdentifiers:toolbar];
        ASSERT_EQ(identifiers.count, 1U);
        NSToolbarItem* const item = [delegate toolbar:toolbar
                                itemForItemIdentifier:identifiers[0]
                            willBeInsertedIntoToolbar:YES];
        // Drive the target-action exactly as NSToolbar would on a click.
        [NSApp sendAction:item.action to:item.target from:item];

        EXPECT_TRUE(fired);
    }

    // ---- context flyout → NSView.menu; tooltip → NSView.toolTip ----

    TEST_F(apple_chrome, context_flyout_materializes_as_the_views_nsmenu)
    {
        button host;
        menu_flyout flyout;
        menu_flyout_item copy_item;
        copy_item.set_text("Copy");
        menu_flyout_separator separator;
        menu_flyout_item paste_item;
        paste_item.set_text("Paste");
        flyout.items().add(copy_item);
        flyout.items().add(separator);
        flyout.items().add(paste_item);

        auto handler = std::make_shared<button_handler>();
        host.set_handler(handler);
        host.set_context_flyout(&flyout);

        NSView* const native = (__bridge NSView*)handler->native_view();
        ASSERT_NE(native.menu, nil);
        ASSERT_EQ(native.menu.numberOfItems, 3);
        EXPECT_EQ(to_std_string([native.menu itemAtIndex:0].title), "Copy");
        EXPECT_TRUE([native.menu itemAtIndex:1].separatorItem);
        EXPECT_EQ(to_std_string([native.menu itemAtIndex:2].title), "Paste");

        bool copied = false;
        copy_item.clicked.connect([&copied] { copied = true; });
        [native.menu performActionForItemAtIndex:0]; // drive the action exactly as NSMenu would
        EXPECT_TRUE(copied);

        host.set_context_flyout(nullptr);
        EXPECT_EQ(native.menu, nil);
    }

    TEST_F(apple_chrome, tool_tip_materializes_as_the_views_tooltip)
    {
        button host;
        auto handler = std::make_shared<button_handler>();
        host.set_handler(handler);

        NSView* const native = (__bridge NSView*)handler->native_view();
        EXPECT_EQ(native.toolTip, nil);

        tool_tip_properties::set_text(host, "Click me");
        ASSERT_NE(native.toolTip, nil);
        EXPECT_EQ(to_std_string(native.toolTip), "Click me");
    }

    // ---- title bar → an NSTitlebarAccessoryViewController ----

    TEST_F(apple_chrome, title_bar_materializes_as_a_titlebar_accessory_and_replaces_and_clears)
    {
        content_page page;
        window host(page);
        auto handler = std::make_shared<window_handler>();
        host.set_handler(handler);

        NSWindow* const native = native_window(handler);
        const NSUInteger baseline = native.titlebarAccessoryViewControllers.count;

        title_bar first;
        first.set_title("My App");
        first.set_subtitle("Pro");
        host.set_title_bar(&first);
        ASSERT_EQ(native.titlebarAccessoryViewControllers.count, baseline + 1);
        NSTitlebarAccessoryViewController* const accessory = native.titlebarAccessoryViewControllers.lastObject;
        ASSERT_TRUE([accessory.view isKindOfClass:[NSTextField class]]);
        EXPECT_EQ(to_std_string(((NSTextField*)accessory.view).stringValue), "My App — Pro");

        title_bar second;
        second.set_title("Second");
        host.set_title_bar(&second);
        ASSERT_EQ(native.titlebarAccessoryViewControllers.count, baseline + 1); // replaced, not stacked
        NSTitlebarAccessoryViewController* const replaced = native.titlebarAccessoryViewControllers.lastObject;
        EXPECT_EQ(to_std_string(((NSTextField*)replaced.view).stringValue), "Second");

        host.set_title_bar(nullptr);
        EXPECT_EQ(native.titlebarAccessoryViewControllers.count, baseline);
    }
} // namespace
