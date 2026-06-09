// Apple (AppKit) backend tests for the navigation_page seam. The container is an NSView holding a CUSTOM
// navigation BAR (always present) above a CONTENT area: after a push, the current page's real native
// NSView is hosted in the content area (and the previously-current page's view is removed); pop/pop_to_root
// swap it back. The bar's NSTextField title tracks the current page and its back NSButton is hidden at the
// root, shown above it. A modal is presented as a full-container overlay. Each page here is a content_page
// whose handler owns a real NSView host (its native_view()), so the container hosts that. Compiled as
// Objective-C++ with ARC for the `apple` backend.
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>

#include <memory>
#include <string>

#include "maui/controls/content_page.hpp"
#include "maui/controls/navigation_page.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/navigation_page_handler.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::content_page;
    using maui::controls::navigation_page;
    using maui::core::content_page_handler;
    using maui::core::navigation_page_handler;

    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    NSView* native_container(const std::shared_ptr<navigation_page_handler>& handler)
    {
        return (__bridge NSView*)handler->typed_platform_view()->native;
    }

    NSView* native_bar(const std::shared_ptr<navigation_page_handler>& handler)
    {
        return (__bridge NSView*)handler->typed_platform_view()->bar;
    }

    NSTextField* native_title(const std::shared_ptr<navigation_page_handler>& handler)
    {
        return (__bridge NSTextField*)handler->typed_platform_view()->title_field;
    }

    NSButton* native_back(const std::shared_ptr<navigation_page_handler>& handler)
    {
        return (__bridge NSButton*)handler->typed_platform_view()->back_button;
    }

    NSView* native_modal_overlay(const std::shared_ptr<navigation_page_handler>& handler)
    {
        return (__bridge NSView*)handler->typed_platform_view()->modal_overlay;
    }

    // The container's CONTENT subviews — everything except the always-present bar (and any modal overlay).
    NSUInteger content_subview_count(const std::shared_ptr<navigation_page_handler>& handler)
    {
        NSView* const bar = native_bar(handler);
        NSView* const overlay = native_modal_overlay(handler);
        NSUInteger count = 0;
        NSArray<NSView*>* const subviews = native_container(handler).subviews;
        for (NSUInteger i = 0; i < subviews.count; ++i)
        {
            if (subviews[i] != bar && subviews[i] != overlay)
            {
                ++count;
            }
        }
        return count;
    }

    // A content_page with its handler attached, so it owns a real native NSView host the container can
    // host. Returns the page's native NSView for superview assertions.
    NSView* attach_page(content_page& page)
    {
        auto page_handler = std::make_shared<content_page_handler>();
        page.set_handler(page_handler);
        return (__bridge NSView*)page_handler->native_view();
    }

    class apple_navigation_page_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_navigation_page_seam, container_hosts_a_bar_and_starts_with_no_content)
    {
        navigation_page nav;
        auto handler = std::make_shared<navigation_page_handler>();
        nav.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_TRUE([native_container(handler) isKindOfClass:[NSView class]]);
        // The bar is always present; there is no content subview until the first push.
        EXPECT_EQ(native_bar(handler).superview, native_container(handler));
        EXPECT_EQ(content_subview_count(handler), 0U);
    }

    TEST_F(apple_navigation_page_seam, push_hosts_the_current_pages_native_view)
    {
        navigation_page nav;
        auto handler = std::make_shared<navigation_page_handler>();
        nav.set_handler(handler);

        content_page root;
        NSView* const root_native = attach_page(root);
        ASSERT_NE(root_native, nil);

        nav.push(root); // -> request_navigation -> the container hosts the root's NSView (below the bar)

        EXPECT_EQ(content_subview_count(handler), 1U);
        EXPECT_EQ(root_native.superview, native_container(handler));
        EXPECT_EQ(handler->typed_platform_view()->hosted_page, &root);
    }

    TEST_F(apple_navigation_page_seam, push_swaps_the_visible_content)
    {
        navigation_page nav;
        auto handler = std::make_shared<navigation_page_handler>();
        nav.set_handler(handler);

        content_page root;
        NSView* const root_native = attach_page(root);
        content_page second;
        NSView* const second_native = attach_page(second);

        nav.push(root);
        EXPECT_EQ(root_native.superview, native_container(handler));

        nav.push(second); // the previous page's view leaves, the new one is hosted
        EXPECT_EQ(content_subview_count(handler), 1U);
        EXPECT_EQ(second_native.superview, native_container(handler));
        EXPECT_EQ(root_native.superview, nil);
    }

    TEST_F(apple_navigation_page_seam, pop_restores_the_revealed_pages_view)
    {
        navigation_page nav;
        auto handler = std::make_shared<navigation_page_handler>();
        nav.set_handler(handler);

        content_page root;
        NSView* const root_native = attach_page(root);
        content_page second;
        NSView* const second_native = attach_page(second);

        nav.push(root);
        nav.push(second);
        EXPECT_EQ(second_native.superview, native_container(handler));

        nav.pop(); // the popped page leaves; the revealed root is hosted again
        EXPECT_EQ(content_subview_count(handler), 1U);
        EXPECT_EQ(root_native.superview, native_container(handler));
        EXPECT_EQ(second_native.superview, nil);
    }

    // ---- the custom bar: title tracks the current page; back is shown only above the root ----

    TEST_F(apple_navigation_page_seam, bar_title_tracks_the_current_page)
    {
        content_page root;
        attach_page(root);
        root.set_title("Root");
        content_page second;
        attach_page(second);
        second.set_title("Second");

        auto handler = std::make_shared<navigation_page_handler>();
        navigation_page nav(root);
        nav.set_handler(handler);
        EXPECT_EQ(to_std_string(native_title(handler).stringValue), "Root");

        nav.push(second);
        EXPECT_EQ(to_std_string(native_title(handler).stringValue), "Second");

        nav.pop();
        EXPECT_EQ(to_std_string(native_title(handler).stringValue), "Root");
    }

    TEST_F(apple_navigation_page_seam, back_button_hidden_at_root_shown_above)
    {
        content_page root;
        attach_page(root);
        content_page second;
        attach_page(second);

        auto handler = std::make_shared<navigation_page_handler>();
        navigation_page nav(root);
        nav.set_handler(handler);
        EXPECT_TRUE(native_back(handler).hidden); // single page -> no back

        nav.push(second);
        EXPECT_FALSE(native_back(handler).hidden); // depth 2 -> back shown

        nav.pop();
        EXPECT_TRUE(native_back(handler).hidden); // back at the root
    }

    TEST_F(apple_navigation_page_seam, back_button_click_pops_the_current_page)
    {
        content_page root;
        attach_page(root);
        content_page second;
        attach_page(second);

        auto handler = std::make_shared<navigation_page_handler>();
        navigation_page nav(root);
        nav.set_handler(handler);
        nav.push(second);
        ASSERT_EQ(nav.current_page(), &second);

        // The back NSButton's target-action routes to navigation_page::send_back_button_pressed() -> pop().
        [native_back(handler) performClick:nil];
        EXPECT_EQ(nav.current_page(), &root);
        EXPECT_EQ(nav.navigation_stack().size(), 1U);
        EXPECT_TRUE(native_back(handler).hidden); // back now hidden at the root
    }

    // ---- the modal overlay: the modal's native view covers the whole container; popping reveals content --

    TEST_F(apple_navigation_page_seam, push_modal_overlays_the_container)
    {
        content_page root;
        NSView* const root_native = attach_page(root);
        content_page modal;
        NSView* const modal_native = attach_page(modal);

        auto handler = std::make_shared<navigation_page_handler>();
        navigation_page nav(root);
        nav.set_handler(handler);
        nav.push(root); // host the root content

        nav.push_modal(modal); // -> request_modal_navigation -> overlay the modal
        NSView* const overlay = native_modal_overlay(handler);
        ASSERT_NE(overlay, nil);
        EXPECT_EQ(overlay.superview, native_container(handler));
        EXPECT_EQ(modal_native.superview, overlay);
        // The underlying content is still hosted (the modal overlays it, it is not removed).
        EXPECT_EQ(root_native.superview, native_container(handler));
        EXPECT_EQ(handler->typed_platform_view()->hosted_modal, &modal);

        nav.pop_modal(); // the overlay is torn down, revealing the content
        EXPECT_EQ(modal_native.superview, nil);
        EXPECT_EQ(native_modal_overlay(handler), nil);
        EXPECT_EQ(root_native.superview, native_container(handler));
    }

    TEST_F(apple_navigation_page_seam, navigating_under_a_modal_keeps_the_overlay)
    {
        content_page root;
        attach_page(root);
        content_page second;
        NSView* const second_native = attach_page(second);
        content_page modal;
        NSView* const modal_native = attach_page(modal);

        auto handler = std::make_shared<navigation_page_handler>();
        navigation_page nav(root);
        nav.set_handler(handler);
        nav.push(root);
        nav.push(second);      // content = second
        nav.push_modal(modal); // overlay the modal on top

        NSView* const overlay = native_modal_overlay(handler);
        ASSERT_NE(overlay, nil);

        // Pop the underlying nav stack WHILE the modal is up: host_current re-swaps the content but must
        // NOT tear down the modal overlay.
        nav.pop();                                         // content swaps back to root under the modal
        EXPECT_EQ(native_modal_overlay(handler), overlay); // the overlay survived the content swap
        EXPECT_EQ(overlay.superview, native_container(handler));
        EXPECT_EQ(modal_native.superview, overlay); // the modal is still inside the overlay
        EXPECT_EQ(second_native.superview, nil);    // the popped page's content left
    }

    TEST_F(apple_navigation_page_seam, arrange_sizes_the_container_bar_and_current_page)
    {
        navigation_page nav;
        auto handler = std::make_shared<navigation_page_handler>();
        nav.set_handler(handler);

        content_page root;
        NSView* const root_native = attach_page(root);
        nav.push(root);

        nav.arrange(maui::graphics::rect(5, 10, 200, 120)); // -> platform_arrange frames container + bar + page

        const NSRect container_frame = native_container(handler).frame;
        EXPECT_EQ(container_frame.origin.x, 5.0);
        EXPECT_EQ(container_frame.origin.y, 10.0);
        EXPECT_EQ(container_frame.size.width, 200.0);
        EXPECT_EQ(container_frame.size.height, 120.0);

        // The bar pins to the top (bottom-left origin: y = height - bar_height = 120 - 44 = 76), full width.
        const NSRect bar_frame = native_bar(handler).frame;
        EXPECT_EQ(bar_frame.origin.x, 0.0);
        EXPECT_EQ(bar_frame.origin.y, 76.0);
        EXPECT_EQ(bar_frame.size.width, 200.0);
        EXPECT_EQ(bar_frame.size.height, 44.0);

        // The current page fills the content area below the bar (origin 0,0; height = 120 - 44 = 76).
        const NSRect page_frame = root_native.frame;
        EXPECT_EQ(page_frame.origin.x, 0.0);
        EXPECT_EQ(page_frame.origin.y, 0.0);
        EXPECT_EQ(page_frame.size.width, 200.0);
        EXPECT_EQ(page_frame.size.height, 76.0);
    }

    // ---- bar styling: BarBackgroundColor paints the bar's layer; BarTextColor colors the title; a TitleView
    // is hosted in the bar instead of the label (C# NavigationPage Bar*Color / TitleView). ----

    TEST_F(apple_navigation_page_seam, bar_background_color_paints_the_bar_layer)
    {
        content_page root;
        attach_page(root);
        auto handler = std::make_shared<navigation_page_handler>();
        navigation_page nav(root);
        nav.set_handler(handler);

        // Unset by default -> the bar keeps its system default (no layer background color).
        NSView* const bar = native_bar(handler);
        EXPECT_EQ(bar.layer.backgroundColor, nullptr);

        nav.set_bar_background_color(maui::graphics::color::from_rgb(10, 20, 30));
        CGColorRef painted_cg = bar.layer.backgroundColor; // capture once (re-reads are nullable)
        ASSERT_NE(painted_cg, nullptr);
        // The painted CGColor matches the requested color. Read it back through an sRGB NSColor (named
        // component accessors, no raw CGColorGetComponents pointer indexing).
        NSColor* const painted =
            [[NSColor colorWithCGColor:painted_cg] colorUsingColorSpace:[NSColorSpace sRGBColorSpace]];
        ASSERT_NE(painted, nil);
        EXPECT_NEAR(painted.redComponent, 10.0 / 255.0, 0.01);
        EXPECT_NEAR(painted.greenComponent, 20.0 / 255.0, 0.01);
        EXPECT_NEAR(painted.blueComponent, 30.0 / 255.0, 0.01);
    }

    TEST_F(apple_navigation_page_seam, bar_text_color_colors_the_title)
    {
        content_page root;
        attach_page(root);
        root.set_title("Root");
        auto handler = std::make_shared<navigation_page_handler>();
        navigation_page nav(root);
        nav.set_handler(handler);

        nav.set_bar_text_color(maui::graphics::color::from_rgb(200, 0, 0));
        NSColor* const text_color = native_title(handler).textColor;
        ASSERT_NE(text_color, nil);
        NSColor* const srgb = [text_color colorUsingColorSpace:[NSColorSpace sRGBColorSpace]];
        EXPECT_NEAR(srgb.redComponent, 200.0 / 255.0, 0.01);
        EXPECT_NEAR(srgb.greenComponent, 0.0, 0.01);
        EXPECT_NEAR(srgb.blueComponent, 0.0, 0.01);
    }

    TEST_F(apple_navigation_page_seam, title_view_is_hosted_in_the_bar_instead_of_the_label)
    {
        content_page root;
        attach_page(root);
        root.set_title("Root");
        content_page title_page; // a stand-in view; its native NSView is hosted in the bar
        NSView* const title_native = attach_page(title_page);
        auto handler = std::make_shared<navigation_page_handler>();
        navigation_page nav(root);
        nav.set_handler(handler);

        NSTextField* const label = native_title(handler);
        NSView* const bar = native_bar(handler);
        EXPECT_FALSE(label.hidden); // the label shows when there is no title view

        nav.set_title_view(&title_page); // host the title view in the bar
        EXPECT_TRUE(label.hidden);       // the label is hidden in favor of the title view
        EXPECT_EQ(title_native.superview, bar);
        EXPECT_EQ(handler->typed_platform_view()->hosted_title_view, &title_page);

        nav.set_title_view(nullptr); // clearing removes the title view + shows the label again
        EXPECT_FALSE(label.hidden);
        EXPECT_EQ(title_native.superview, nil);
        EXPECT_EQ(handler->typed_platform_view()->hosted_title_view, nullptr);
    }
} // namespace
