// iOS (UIKit) backend tests for the slider seam — run only for MAUI_BACKEND=ios (executed ON the iOS
// simulator via tools/ios-sim-run.sh). Drives a genuine UISlider: Min/Max/Value map to the float
// MinValue/MaxValue/Value, the colors to the three tint properties, and the native control events
// (ValueChanged / TouchDown / TouchUpInside) flow back through the handler's proxy to set_value and
// the drag events. Compiled as Objective-C++ with ARC.
//
// NATIVE EVENT INJECTION: as in button_ios_tests.mm, -[UIControl sendActionsForControlEvents:] needs a
// UIApplication this spawned test process cannot create, so send_control_event replicates UIControl's
// documented dispatch walk over the REAL registrations the handler made on the REAL UISlider.
#import <UIKit/UIKit.h>

#include <cstring>
#include <memory>
#include <string>

#include "maui/controls/file_image_source.hpp" // image_source::from_stream
#include "maui/controls/platform_configuration/ios_specific/slider.hpp"
#include "maui/controls/slider.hpp"
#include "maui/core/cancellation_token.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_stream_image_source.hpp" // image_bytes
#include "maui/core/slider_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::image_source;
    using maui::controls::slider;
    using maui::core::cancellation_token;
    using maui::core::i_element_handler;
    using maui::core::image_bytes;
    using maui::core::slider_handler;

    image_bytes to_image_bytes(NSData* data)
    {
        if (data == nil || data.length == 0)
        {
            return {};
        }
        image_bytes bytes(static_cast<std::size_t>(data.length));
        std::memcpy(bytes.data(), data.bytes, static_cast<std::size_t>(data.length));
        return bytes;
    }

    // A real 2x2 PNG so the loader decodes a genuine UIImage (the knob image SetThumbImage applies).
    image_bytes make_png_bytes()
    {
        UIGraphicsImageRendererFormat* const format = [[UIGraphicsImageRendererFormat alloc] init];
        format.scale = 1;
        UIGraphicsImageRenderer* const renderer = [[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(2, 2)
                                                                                         format:format];
        UIImage* const image = [renderer imageWithActions:^(UIGraphicsImageRendererContext* context) {
          [[UIColor redColor] setFill];
          [context fillRect:CGRectMake(0, 0, 2, 2)];
        }];
        return to_image_bytes(image != nil ? UIImagePNGRepresentation(image) : nil);
    }

    // A stream source yielding a real PNG (the loader decodes it inline — no dispatcher set on iOS).
    std::shared_ptr<maui::core::i_image_source> make_png_stream_source()
    {
        return image_source::from_stream([](const cancellation_token&) { return make_png_bytes(); });
    }

    UISlider* native_slider(const std::shared_ptr<slider_handler>& handler)
    {
        return (__bridge UISlider*)handler->typed_platform_view()->native;
    }

    // Replicates -[UIControl sendActionsForControlEvents:]'s dispatch-table walk for one event (see
    // the header comment).
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

    TEST(ios_slider_seam, attaching_handler_creates_uislider_and_maps_range)
    {
        slider control(20, 200, 50);
        auto handler = std::make_shared<slider_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        UISlider* const view = native_slider(handler);
        EXPECT_EQ(view.minimumValue, 20.0F);
        EXPECT_EQ(view.maximumValue, 200.0F);
        EXPECT_EQ(view.value, 50.0F);
        EXPECT_TRUE(view.continuous);
    }

    TEST(ios_slider_seam, setting_value_updates_the_uislider)
    {
        slider control(0, 100, 0);
        auto handler = std::make_shared<slider_handler>();
        control.set_handler(handler);

        control.set_value(42);
        EXPECT_EQ(native_slider(handler).value, 42.0F);
    }

    TEST(ios_slider_seam, native_value_change_flows_back)
    {
        slider control(0, 100, 0);
        auto handler = std::make_shared<slider_handler>();
        control.set_handler(handler);

        double reported_new = -1;
        control.value_changed.connect([&reported_new](double, double new_value) { reported_new = new_value; });

        // Simulate the user's drag: the native value moves, then ValueChanged fires.
        UISlider* const view = native_slider(handler);
        view.value = 33;
        send_control_event(view, UIControlEventValueChanged);

        EXPECT_EQ(control.value(), 33);
        EXPECT_EQ(reported_new, 33);
    }

    TEST(ios_slider_seam, native_touch_events_drive_the_drag_channel)
    {
        slider control;
        auto handler = std::make_shared<slider_handler>();
        control.set_handler(handler);

        bool started = false;
        bool completed = false;
        control.drag_started.connect([&started] { started = true; });
        control.drag_completed.connect([&completed] { completed = true; });

        UISlider* const view = native_slider(handler);
        send_control_event(view, UIControlEventTouchDown); // finger down on the thumb
        EXPECT_TRUE(started);
        EXPECT_FALSE(completed);
        send_control_event(view, UIControlEventTouchUpInside); // released
        EXPECT_TRUE(completed);
    }

    TEST(ios_slider_seam, colors_map_to_the_tint_properties)
    {
        slider control;
        control.set_minimum_track_color(maui::graphics::color(0.0F, 1.0F, 0.0F));
        control.set_maximum_track_color(maui::graphics::color(0.0F, 0.0F, 1.0F));
        control.set_thumb_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        auto handler = std::make_shared<slider_handler>();
        control.set_handler(handler);
        UISlider* const view = native_slider(handler);

        CGFloat red = 0;
        CGFloat green = 0;
        CGFloat blue = 0;
        CGFloat alpha = 0;
        ASSERT_TRUE([view.minimumTrackTintColor getRed:&red green:&green blue:&blue alpha:&alpha]);
        EXPECT_NEAR(green, 1.0, 0.01);
        ASSERT_TRUE([view.maximumTrackTintColor getRed:&red green:&green blue:&blue alpha:&alpha]);
        EXPECT_NEAR(blue, 1.0, 0.01);
        ASSERT_TRUE([view.thumbTintColor getRed:&red green:&green blue:&blue alpha:&alpha]);
        EXPECT_NEAR(red, 1.0, 0.01);
    }

    TEST(ios_slider_seam, thumb_image_source_sets_the_native_thumb_image)
    {
        // SliderHandler.MapThumbImageSource → SetThumbImage: a decoded image lands on the UISlider's
        // Normal-state thumb image, and the thumb tint is cleared while it is shown.
        slider control;
        control.set_thumb_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        auto handler = std::make_shared<slider_handler>();
        control.set_handler(handler);

        control.set_thumb_image_source(make_png_stream_source());
        UISlider* const view = native_slider(handler);
        EXPECT_NE([view thumbImageForState:UIControlStateNormal], nil);
        EXPECT_EQ(view.thumbTintColor, nil); // cleared while an image is set
        EXPECT_TRUE(handler->typed_platform_view()->thumb_image_set);

        control.set_thumb_image_source(nullptr); // clearing drops the image + re-applies the thumb color
        EXPECT_EQ([view thumbImageForState:UIControlStateNormal], nil);
        EXPECT_FALSE(handler->typed_platform_view()->thumb_image_set);
        EXPECT_NE(view.thumbTintColor, nil);
    }

    // Count the tap recognizers the HANDLER added (UISlider carries its own built-in recognizers on
    // recent iOS, so the count is measured as a delta against that baseline).
    NSUInteger our_tap_count(UISlider* view)
    {
        NSUInteger count = 0;
        for (UIGestureRecognizer* gr in view.gestureRecognizers)
        {
            if ([gr isKindOfClass:[UITapGestureRecognizer class]])
            {
                ++count;
            }
        }
        return count;
    }

    TEST(ios_slider_seam, update_on_tap_installs_and_removes_a_tap_recognizer)
    {
        // Slider.MapUpdateOnTap: a UITapGestureRecognizer is attached when the knob is true, removed when
        // false. UISlider has built-in recognizers, so the install/remove is asserted as a +1/-0 delta.
        namespace ios_slider = maui::controls::platform_configuration::ios_specific::slider;
        slider control;
        auto handler = std::make_shared<slider_handler>();
        control.set_handler(handler);
        UISlider* const view = native_slider(handler);
        const NSUInteger baseline = our_tap_count(view);

        ios_slider::set_update_on_tap(control, true);
        EXPECT_EQ(our_tap_count(view), baseline + 1); // our tap recognizer was added
        EXPECT_TRUE(handler->typed_platform_view()->update_on_tap);

        ios_slider::set_update_on_tap(control, false);
        EXPECT_EQ(our_tap_count(view), baseline); // and removed
        EXPECT_FALSE(handler->typed_platform_view()->update_on_tap);
    }

    TEST(ios_slider_seam, update_on_tap_recognizer_is_idempotent_on_repeated_enable)
    {
        // Re-running the map with UpdateOnTap still true must not stack recognizers (the C# null-guard:
        // a recognizer is created only when none exists).
        namespace ios_slider = maui::controls::platform_configuration::ios_specific::slider;
        slider control;
        auto handler = std::make_shared<slider_handler>();
        control.set_handler(handler);
        UISlider* const view = native_slider(handler);
        const NSUInteger baseline = our_tap_count(view);

        ios_slider::set_update_on_tap(control, true);
        handler->update_value("ios.Slider.UpdateOnTap"); // re-run the mapper with the same value
        EXPECT_EQ(our_tap_count(view), baseline + 1);    // still exactly one added (not stacked)
    }

    TEST(ios_slider_seam, clearing_handler_disconnects)
    {
        slider control;
        auto handler = std::make_shared<slider_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST(ios_slider_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<slider>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<slider_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        slider control(0, 10, 7);
        control.set_handler(handler);
        EXPECT_EQ(((__bridge UISlider*)resolved->typed_platform_view()->native).value, 7.0F);
    }
} // namespace
