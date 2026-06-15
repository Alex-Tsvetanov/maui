// Apple (AppKit) backend tests for the toggle_switch seam — run only for MAUI_BACKEND=apple. Drives a
// genuine NSSwitch: IsToggled maps to NSSwitch.state, and a native toggle ([NSSwitch performClick:]
// flips the state and fires the target-action without a run loop) flows back through the handler to
// the control's `toggled` event. Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>

#include <memory>
#include <string>

#include "maui/controls/toggle_switch.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/switch_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::toggle_switch;
    using maui::controls::vertical_stack_layout;
    using maui::core::i_element_handler;
    using maui::core::switch_handler;

    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    NSSwitch* native_switch(const std::shared_ptr<switch_handler>& handler)
    {
        return (__bridge NSSwitch*)handler->typed_platform_view()->native;
    }

    // NSSwitch creation needs the shared application object (no run loop required).
    class apple_switch_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_switch_seam, attaching_handler_creates_nsswitch_and_maps_state)
    {
        toggle_switch control;
        control.set_is_toggled(true);
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_EQ(native_switch(handler).state, NSControlStateValueOn);
    }

    TEST_F(apple_switch_seam, setting_is_toggled_updates_the_nsswitch)
    {
        toggle_switch control;
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);
        EXPECT_EQ(native_switch(handler).state, NSControlStateValueOff);

        control.set_is_toggled(true);
        EXPECT_EQ(native_switch(handler).state, NSControlStateValueOn);
    }

    TEST_F(apple_switch_seam, native_click_toggles_the_control)
    {
        toggle_switch control;
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);

        bool reported = false;
        control.toggled.connect([&reported](bool value) { reported = value; });

        [native_switch(handler) performClick:nil]; // a real native toggle (state flip + action)

        EXPECT_TRUE(control.is_toggled());
        EXPECT_TRUE(reported);

        [native_switch(handler) performClick:nil]; // and back off
        EXPECT_FALSE(control.is_toggled());
        EXPECT_FALSE(reported);
    }

    TEST_F(apple_switch_seam, track_and_thumb_colors_record_the_mirrors)
    {
        // AppKit deviation (documented in switch_handler.mm): NSSwitch exposes no public tint API, so
        // the colors land on the cross-platform mirrors — the observable native-adjacent state.
        toggle_switch control;
        control.set_is_toggled(true);
        control.set_on_color(maui::graphics::color(0.0F, 1.0F, 0.0F));
        control.set_thumb_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);

        EXPECT_EQ(handler->typed_platform_view()->track_color, maui::graphics::color(0.0F, 1.0F, 0.0F));
        EXPECT_EQ(handler->typed_platform_view()->thumb_color, maui::graphics::color(1.0F, 0.0F, 0.0F));

        control.set_is_toggled(false);
        control.set_off_color(maui::graphics::color(0.0F, 0.0F, 1.0F));
        EXPECT_EQ(handler->typed_platform_view()->track_color, maui::graphics::color(0.0F, 0.0F, 1.0F));
    }

    TEST_F(apple_switch_seam, generic_iview_properties_reach_the_nsswitch)
    {
        toggle_switch control;
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);
        NSSwitch* const view = native_switch(handler);

        control.set_is_enabled(false);
        EXPECT_FALSE(view.enabled);

        control.set_visibility(maui::core::visibility::hidden);
        EXPECT_TRUE(view.hidden);

        control.set_opacity(0.5);
        EXPECT_EQ(view.alphaValue, 0.5);

        control.set_automation_id("dark_mode_switch");
        EXPECT_EQ(to_std_string(view.accessibilityIdentifier), "dark_mode_switch");
    }

    TEST_F(apple_switch_seam, needs_container_wraps_the_nsswitch)
    {
        // SwitchHandler.NeedsContainer => true: the container_view map wraps the natural-sized NSSwitch
        // in a plain NSView container on connect (the >101pt accessibility workaround). The switch
        // becomes a subview of the container, and the handler exposes the container as container_view().
        toggle_switch control;
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);

        EXPECT_TRUE(handler->has_container());
        void* const container = handler->container_view();
        ASSERT_NE(container, nullptr);
        NSView* const wrapper = (__bridge NSView*)container;
        NSSwitch* const view = native_switch(handler);
        EXPECT_EQ(view.superview, wrapper);
        EXPECT_EQ(wrapper.subviews.count, 1U);
    }

    // W8-56 regression (#7): native_view() is C#'s ToPlatform() = ContainerView ?? PlatformView, so a
    // NeedsContainer switch must hand back its CONTAINER (not the bare NSSwitch). Previously native_view()
    // always returned the bare native, so the container never entered the visual tree.
    TEST_F(apple_switch_seam, native_view_is_the_container_when_wrapped)
    {
        toggle_switch control;
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);

        ASSERT_TRUE(handler->has_container());
        auto* const view_handler = static_cast<maui::core::i_view_handler*>(handler.get());
        EXPECT_EQ(view_handler->native_view(), handler->container_view());              // ToPlatform → container
        EXPECT_NE(view_handler->native_view(), (__bridge void*)native_switch(handler)); // NOT the bare switch
    }

    // W8-56 regression (#7): a NeedsContainer switch added to a layout → the panel's child subview is the
    // CONTAINER view (which holds the switch), not the bare switch. This is the end-to-end visual-tree path.
    TEST_F(apple_switch_seam, needs_container_switch_inserts_container_into_layout)
    {
        vertical_stack_layout stack;
        auto layout = std::make_shared<maui::core::layout_handler>();
        stack.set_handler(layout);
        NSView* const panel = (__bridge NSView*)layout->typed_platform_view()->native;

        toggle_switch control;
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);
        NSView* const wrapper = (__bridge NSView*)handler->container_view();
        NSSwitch* const view = native_switch(handler);
        ASSERT_NE(wrapper, nil);

        stack.add(control);

        ASSERT_EQ(panel.subviews.count, 1U);
        EXPECT_EQ(panel.subviews.firstObject, wrapper); // the container is the panel's subview
        EXPECT_NE(panel.subviews.firstObject, view);    // NOT the bare switch
        EXPECT_EQ(view.superview, wrapper);             // the switch still lives inside the container
    }

    TEST_F(apple_switch_seam, clearing_handler_disconnects)
    {
        toggle_switch control;
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST_F(apple_switch_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<toggle_switch>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<switch_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        toggle_switch control;
        control.set_is_toggled(true);
        control.set_handler(handler);
        EXPECT_EQ(((__bridge NSSwitch*)resolved->typed_platform_view()->native).state, NSControlStateValueOn);
    }
} // namespace
