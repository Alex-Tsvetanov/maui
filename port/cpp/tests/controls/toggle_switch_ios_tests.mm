// iOS (UIKit) backend tests for the toggle_switch seam — run only for MAUI_BACKEND=ios (executed ON
// the iOS simulator via tools/ios-sim-run.sh). Drives a genuine UISwitch: IsToggled maps through
// setOn:animated:, the colors through the SwitchExtensions recipe, and the native toggle flows back
// through the handler's ValueChanged proxy to the control's `toggled` event. Compiled as Objective-C++
// with ARC.
//
// NATIVE EVENT INJECTION: as in button_ios_tests.mm, -[UIControl sendActionsForControlEvents:] needs a
// UIApplication this spawned test process cannot create, so send_control_event replicates UIControl's
// documented dispatch walk (allTargets × actionsForTarget:forControlEvent: → invoke) over the REAL
// registration the handler made on the REAL UISwitch.
#import <UIKit/UIKit.h>

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

    UISwitch* native_switch(const std::shared_ptr<switch_handler>& handler)
    {
        return (__bridge UISwitch*)handler->typed_platform_view()->native;
    }

    // Replicates -[UIControl sendActionsForControlEvents:]'s dispatch-table walk for one event (see
    // the header comment): every (target, action) pair registered for `event` is invoked with the
    // control as sender, exactly as UIApplication's sendAction:to:from:forEvent: would.
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

    TEST(ios_switch_seam, attaching_handler_creates_uiswitch_and_maps_state)
    {
        toggle_switch control;
        control.set_is_toggled(true);
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_TRUE(native_switch(handler).on);
    }

    TEST(ios_switch_seam, setting_is_toggled_updates_the_uiswitch)
    {
        toggle_switch control;
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);
        EXPECT_FALSE(native_switch(handler).on);

        control.set_is_toggled(true);
        EXPECT_TRUE(native_switch(handler).on);
    }

    TEST(ios_switch_seam, native_toggle_flows_back_to_the_control)
    {
        toggle_switch control;
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);

        bool reported = false;
        control.toggled.connect([&reported](bool value) { reported = value; });

        // Simulate the user's flip: the programmatic setOn: does not fire ValueChanged (UIKit
        // behavior), so flip the state then drive the registered ValueChanged action.
        UISwitch* const view = native_switch(handler);
        view.on = YES;
        send_control_event(view, UIControlEventValueChanged);

        EXPECT_TRUE(control.is_toggled());
        EXPECT_TRUE(reported);
    }

    TEST(ios_switch_seam, on_color_maps_to_on_tint_color)
    {
        toggle_switch control;
        control.set_is_toggled(true);
        control.set_on_color(maui::graphics::color(0.0F, 1.0F, 0.0F));
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);

        UIColor* const tint = native_switch(handler).onTintColor;
        ASSERT_NE(tint, nil);
        CGFloat red = 0;
        CGFloat green = 0;
        CGFloat blue = 0;
        CGFloat alpha = 0;
        ASSERT_TRUE([tint getRed:&red green:&green blue:&blue alpha:&alpha]);
        EXPECT_NEAR(green, 1.0, 0.01);
        EXPECT_NEAR(red, 0.0, 0.01);
    }

    TEST(ios_switch_seam, thumb_color_maps_to_thumb_tint_color)
    {
        toggle_switch control;
        control.set_thumb_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);

        UIColor* const tint = native_switch(handler).thumbTintColor;
        ASSERT_NE(tint, nil);
        CGFloat red = 0;
        CGFloat green = 0;
        CGFloat blue = 0;
        CGFloat alpha = 0;
        ASSERT_TRUE([tint getRed:&red green:&green blue:&blue alpha:&alpha]);
        EXPECT_NEAR(red, 1.0, 0.01);
    }

    TEST(ios_switch_seam, generic_iview_properties_reach_the_uiswitch)
    {
        toggle_switch control;
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);
        UISwitch* const view = native_switch(handler);

        control.set_is_enabled(false);
        EXPECT_FALSE(view.enabled);

        control.set_visibility(maui::core::visibility::hidden);
        EXPECT_TRUE(view.hidden);

        control.set_opacity(0.5);
        EXPECT_EQ(view.alpha, 0.5);

        control.set_automation_id("dark_mode_switch");
        EXPECT_EQ(to_std_string(view.accessibilityIdentifier), "dark_mode_switch");
    }

    TEST(ios_switch_seam, needs_container_wraps_the_uiswitch)
    {
        // SwitchHandler.NeedsContainer => true: the container_view map wraps the natural-sized UISwitch
        // in a plain UIView container on connect (the >101pt accessibility workaround). The switch
        // becomes a subview of the container, and the handler exposes it as container_view().
        toggle_switch control;
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);

        EXPECT_TRUE(handler->has_container());
        void* const container = handler->container_view();
        ASSERT_NE(container, nullptr);
        UIView* const wrapper = (__bridge UIView*)container;
        UISwitch* const view = native_switch(handler);
        EXPECT_EQ(view.superview, wrapper);
        EXPECT_EQ(wrapper.subviews.count, 1U);
    }

    // W8-56 regression (#7): native_view() is C#'s ToPlatform() = ContainerView ?? PlatformView, so a
    // NeedsContainer switch must hand back its CONTAINER (not the bare UISwitch). Previously native_view()
    // always returned the bare native, so the container never entered the visual tree.
    TEST(ios_switch_seam, native_view_is_the_container_when_wrapped)
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
    TEST(ios_switch_seam, needs_container_switch_inserts_container_into_layout)
    {
        vertical_stack_layout stack;
        auto layout = std::make_shared<maui::core::layout_handler>();
        stack.set_handler(layout);
        UIView* const panel = (__bridge UIView*)layout->typed_platform_view()->native;

        toggle_switch control;
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);
        UIView* const wrapper = (__bridge UIView*)handler->container_view();
        UISwitch* const view = native_switch(handler);
        ASSERT_NE(wrapper, nil);

        stack.add(control);

        ASSERT_EQ(panel.subviews.count, 1U);
        EXPECT_EQ(panel.subviews.firstObject, wrapper); // the container is the panel's subview
        EXPECT_NE(panel.subviews.firstObject, view);    // NOT the bare switch
        EXPECT_EQ(view.superview, wrapper);             // the switch still lives inside the container
    }

    TEST(ios_switch_seam, clearing_handler_disconnects)
    {
        toggle_switch control;
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST(ios_switch_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<toggle_switch>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<switch_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        toggle_switch control;
        control.set_is_toggled(true);
        control.set_handler(handler);
        EXPECT_TRUE(((__bridge UISwitch*)resolved->typed_platform_view()->native).on);
    }
} // namespace
