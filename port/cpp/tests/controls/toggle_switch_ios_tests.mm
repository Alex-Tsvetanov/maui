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
#include "tests/support/run_loop_pump.hpp"
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

    // SwitchExtensions.GetTrackSubview (iOS 13+ branch): the live track view whose backgroundColor the
    // track-color recipe drives — the first subview's first subview. Mirrors switch_handler.mm.
    UIView* track_subview(UISwitch* native)
    {
        return native.subviews.firstObject.subviews.firstObject;
    }

    // The green channel of a UIColor (the color-re-application assertions compare a single component —
    // re-application restores the custom color the test corrupted).
    CGFloat green_component(UIColor* color)
    {
        CGFloat red = 0;
        CGFloat green = 0;
        CGFloat blue = 0;
        CGFloat alpha = 0;
        return [color getRed:&red green:&green blue:&blue alpha:&alpha] ? green : -1;
    }

    CGFloat red_component(UIColor* color)
    {
        CGFloat red = 0;
        CGFloat green = 0;
        CGFloat blue = 0;
        CGFloat alpha = 0;
        return [color getRed:&red green:&green blue:&blue alpha:&alpha] ? red : -1;
    }

    // A key+visible host window so a trait change actually propagates to a hosted view (a detached view
    // does not observe an overrideUserInterfaceStyle flip). [[UIWindow alloc] init] adopts a placeholder
    // scene in the spawned test process — the same initializer window_handler.mm uses; the deprecation
    // pragma matches that established precedent (see collection_view_ios_tests.mm).
    UIWindow* make_host_window()
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        UIWindow* const window = [[UIWindow alloc] init]; // SDK-deprecated; see window_handler.mm precedent
#pragma clang diagnostic pop
        return window;
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

    // platform_arrange must frame the CONTAINER, not the bare switch. Framing only the switch left the
    // wrapper at its setup-time CGRectZero, and because UIView does not clip to bounds the switch still
    // RENDERED correctly — so the board's at-rest frames were pixel-identical to MAUI's while every touch
    // was refused: hitTest: asks pointInside: of the empty wrapper first and nothing is ever inside it.
    // Measured on maccatalyst 2026-08-07: MAUI's switch changed 877 px on the toggle step, the port zero.
    //
    // The assertion is about HIT-TESTABILITY, so it checks what UIKit checks — that the arranged point
    // resolves through the wrapper down to the switch — rather than just comparing frame rectangles.
    TEST(ios_switch_seam, arrange_frames_the_container_so_the_switch_is_hit_testable)
    {
        toggle_switch control;
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);
        UIView* const wrapper = (__bridge UIView*)handler->container_view();
        UISwitch* const view = native_switch(handler);
        ASSERT_NE(wrapper, nil);

        // The wrapper starts at the switch's INTRINSIC size at the ORIGIN — UISwitch sizes itself even
        // when minted with initWithFrame:CGRectZero, so the wrapper is ~51x31 at (0,0) rather than empty.
        // That is precisely why the bug hid: the wrapper is a plausible size, just never in the arranged
        // PLACE, so anything below its ~31pt height falls outside it.
        EXPECT_EQ(wrapper.frame.origin.x, 0);
        EXPECT_EQ(wrapper.frame.origin.y, 0);

        const maui::graphics::rect arranged{12, 117, 51, 31};
        ASSERT_GT(arranged.y, wrapper.frame.size.height)
            << "the arranged row must sit below the un-moved wrapper, or this proves nothing";
        handler->platform_arrange(arranged);

        EXPECT_EQ(wrapper.frame.origin.x, arranged.x);
        EXPECT_EQ(wrapper.frame.origin.y, arranged.y);
        EXPECT_EQ(wrapper.frame.size.width, arranged.width);
        EXPECT_EQ(wrapper.frame.size.height, arranged.height);
        EXPECT_EQ(view.frame.origin.x, 0) << "the switch fills the container, in ITS coordinate space";
        EXPECT_EQ(view.frame.origin.y, 0);

        // The centre of the arranged rect, in the wrapper's own coordinates: UIKit must route it to the
        // switch. With a zero-sized wrapper this returns nil, which is the whole bug.
        const CGPoint centre = CGPointMake(arranged.width / 2, arranged.height / 2);
        EXPECT_TRUE([wrapper pointInside:centre withEvent:nil]);
        // hitTest: returns the DEEPEST view under the point, which for a UISwitch is one of its own
        // private visual subviews — never the UISwitch itself. What matters is that the touch lands
        // somewhere inside the switch, so ask that; isDescendantOfView: is YES for the switch too.
        UIView* const hit = [wrapper hitTest:centre withEvent:nil];
        ASSERT_NE(hit, nil) << "a zero-sized or mispositioned wrapper swallows the touch";
        EXPECT_TRUE([hit isDescendantOfView:view]);
    }

    // U19 — SwitchProxy.WillEnterForeground observer: on app return-from-background UIKit re-applies the
    // default OFF-track styling, so the handler re-applies the custom OFF track color (async, 10ms settle)
    // when the switch is OFF and a custom color is set. The spawned process has no UIApplication, but the
    // handler observes object:nil, so a nil-object post reaches the block exactly like the system's would.
    TEST(ios_switch_seam, foreground_notification_reapplies_off_track_color)
    {
        toggle_switch control;
        control.set_is_toggled(false);                                  // OFF — the re-apply branch
        control.set_off_color(maui::graphics::color(0.0F, 1.0F, 0.0F)); // custom green OFF track
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);

        UISwitch* const view = native_switch(handler);
        UIView* const track = track_subview(view);
        ASSERT_NE(track, nil);
        EXPECT_NEAR(green_component(track.backgroundColor), 1.0, 0.01); // mapped on connect

        // Corrupt the live track color the way a UIKit lifecycle reset would, then post the notification.
        track.backgroundColor = UIColor.redColor;
        [[NSNotificationCenter defaultCenter] postNotificationName:UIApplicationWillEnterForegroundNotification
                                                            object:nil];

        // The re-apply is dispatched async with a 10ms settle — pump the main loop until it lands.
        const bool restored =
            maui::tests::pump_until([&] { return green_component(track_subview(view).backgroundColor) > 0.99; });
        EXPECT_TRUE(restored);
    }

    // U19 — SwitchProxy iOS-26 trait-change registration: a light/dark change resets thumbTintColor, so
    // the handler re-applies the custom ThumbColor (async, 10ms settle). The switch is hosted in a
    // key+visible window and the WINDOW's overrideUserInterfaceStyle is flipped, which propagates a
    // UITraitUserInterfaceStyle change down to the hosted switch — firing the registration the handler made.
    TEST(ios_switch_seam, trait_change_reapplies_thumb_color)
    {
        toggle_switch control;
        control.set_thumb_color(maui::graphics::color(0.0F, 1.0F, 0.0F)); // custom green thumb
        auto handler = std::make_shared<switch_handler>();
        control.set_handler(handler);

        UISwitch* const view = native_switch(handler);
        EXPECT_NEAR(green_component(view.thumbTintColor), 1.0, 0.01); // mapped on connect

        // Host the switch in a key+visible window (a detached view does not observe a trait flip).
        UIWindow* const window = make_host_window();
        window.overrideUserInterfaceStyle = UIUserInterfaceStyleLight; // start light
        [window addSubview:view];
        [window makeKeyAndVisible];

        // Drain the connect-time immediate re-apply (also async, 10ms) so it cannot confound the trait
        // path: after this settle the ONLY thing that can restore green is the trait-change registration.
        maui::tests::pump_run_loop(0.05);

        // Corrupt the thumb tint the way a UIKit theme reset would, then drive the light→dark change.
        view.thumbTintColor = UIColor.redColor;
        EXPECT_NEAR(red_component(view.thumbTintColor), 1.0, 0.01);
        window.overrideUserInterfaceStyle = UIUserInterfaceStyleDark; // propagates the trait change down

        const bool restored = maui::tests::pump_until([&] { return green_component(view.thumbTintColor) > 0.99; });
        EXPECT_TRUE(restored);
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
