// iOS (UIKit) backend tests for the image_button seam — a FILE source loads synchronously into a real
// UIButton's Normal-state image (first-frame + AlwaysOriginal + Fill alignments, the
// ImageButtonImageSourcePartSetter recipe), the aspect rides the button's imageView contentMode, the
// stroke/corner ride the layer (the ButtonExtensions recipe), padding maps to contentEdgeInsets, and
// the native touch events flow back through the handler's ImageButtonProxy twin (TouchDown → pressed;
// TouchUpInside → released + clicked; TouchUpOutside → released). Run only for MAUI_BACKEND=ios
// (executed ON the iOS simulator via tools/ios-sim-run.sh). Compiled as Objective-C++ with ARC.
//
// NATIVE EVENT INJECTION: same constraint as button_ios_tests.mm — a spawned simulator process has no
// UIApplication, so send_control_event replicates -[UIControl sendActionsForControlEvents:]'s
// dispatch-table walk over the REAL registrations the handler made on the REAL UIButton.
#import <UIKit/UIKit.h>

#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "maui/controls/file_image_source.hpp"
#include "maui/controls/image_button.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/image_button_handler.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::image_button;
    using maui::controls::image_source;
    using maui::core::aspect;
    using maui::core::i_element_handler;
    using maui::core::image_button_handler;

    // -[NSString UTF8String] is nullable-annotated; convert through this guard so the std::string
    // construction never receives a null pointer (the values under test are always non-null).
    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    UIButton* native_button(const std::shared_ptr<image_button_handler>& handler)
    {
        return (__bridge UIButton*)handler->typed_platform_view()->native;
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

    // Writes a square PNG of the given point-size (scale 1, so .Size == pixels) to a unique path under
    // NSTemporaryDirectory(). The seam tests pass 2; the oversize measure test passes 256.
    std::string write_temp_png_sized(CGFloat side)
    {
        UIGraphicsImageRendererFormat* const format = [[UIGraphicsImageRendererFormat alloc] init];
        format.opaque = NO;
        format.scale = 1;
        UIGraphicsImageRenderer* const renderer = [[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(side, side)
                                                                                         format:format];
        UIImage* const image = [renderer imageWithActions:^(UIGraphicsImageRendererContext* context) {
          [[UIColor colorWithRed:0 green:0 blue:1 alpha:1] setFill];
          [context fillRect:CGRectMake(0, 0, side, side)];
        }];
        NSData* const png = image != nil ? UIImagePNGRepresentation(image) : nil;
        if (png == nil)
        {
            return {};
        }
        NSString* const name = [NSString stringWithFormat:@"maui_image_button_test_%@.png", [[NSUUID UUID] UUIDString]];
        NSString* const path = [NSTemporaryDirectory() stringByAppendingPathComponent:name];
        if (![png writeToFile:path atomically:YES])
        {
            return {};
        }
        return to_std_string(path);
    }

    // Writes a tiny 2x2 PNG to a unique path under NSTemporaryDirectory() (the image test convention).
    std::string write_temp_png()
    {
        return write_temp_png_sized(2);
    }

    void remove_file(const std::string& path)
    {
        if (!path.empty())
        {
            [[NSFileManager defaultManager] removeItemAtPath:@(path.c_str()) error:nil];
        }
    }

    TEST(ios_image_button_seam, creates_a_system_button_that_clips_to_bounds)
    {
        image_button control;
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);
        UIButton* const button = native_button(handler);
        EXPECT_TRUE([button isKindOfClass:[UIButton class]]);
        EXPECT_TRUE(button.clipsToBounds); // CreatePlatformView: ClipsToBounds = true
    }

    TEST(ios_image_button_seam, file_source_loads_into_the_normal_state_image)
    {
        const std::string path = write_temp_png();
        ASSERT_FALSE(path.empty());

        image_button control;
        control.set_source(image_source::from_file(path));
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);

        UIButton* const button = native_button(handler);
        EXPECT_NE([button imageForState:UIControlStateNormal], nil);
        // ImageButtonImageSourcePartSetter: Fill content alignments so the image stretches.
        EXPECT_EQ(button.contentHorizontalAlignment, UIControlContentHorizontalAlignmentFill);
        EXPECT_EQ(button.contentVerticalAlignment, UIControlContentVerticalAlignmentFill);

        control.set_source(nullptr);
        EXPECT_EQ([button imageForState:UIControlStateNormal], nil);
        remove_file(path);
    }

    TEST(ios_image_button_seam, aspect_maps_to_the_image_view_content_mode)
    {
        image_button control;
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);
        EXPECT_EQ(native_button(handler).imageView.contentMode, UIViewContentModeScaleAspectFit);

        control.set_aspect(aspect::aspect_fill);
        EXPECT_EQ(native_button(handler).imageView.contentMode, UIViewContentModeScaleAspectFill);

        control.set_aspect(aspect::fill);
        EXPECT_EQ(native_button(handler).imageView.contentMode, UIViewContentModeScaleToFill);

        control.set_aspect(aspect::center);
        EXPECT_EQ(native_button(handler).imageView.contentMode, UIViewContentModeCenter);
    }

    TEST(ios_image_button_seam, stroke_and_corner_ride_the_layer)
    {
        image_button control;
        control.set_stroke_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        control.set_stroke_thickness(3.0);
        control.set_corner_radius(7);
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);

        UIButton* const button = native_button(handler);
        EXPECT_EQ(button.layer.borderWidth, 3.0);
        EXPECT_EQ(button.layer.cornerRadius, 7.0);
    }

    TEST(ios_image_button_seam, padding_maps_to_content_edge_insets)
    {
        image_button control;
        control.set_padding(maui::core::thickness{4, 8, 12, 16});
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        const UIEdgeInsets insets = native_button(handler).contentEdgeInsets;
#pragma clang diagnostic pop
        EXPECT_EQ(insets.left, 4.0);
        EXPECT_EQ(insets.top, 8.0);
        EXPECT_EQ(insets.right, 12.0);
        EXPECT_EQ(insets.bottom, 16.0);
    }

    TEST(ios_image_button_seam, native_touches_flow_back_in_the_proxy_order)
    {
        image_button control;
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);

        std::vector<std::string> order;
        control.pressed.connect([&order] { order.emplace_back("pressed"); });
        control.released.connect([&order] { order.emplace_back("released"); });
        control.clicked.connect([&order] { order.emplace_back("clicked"); });

        UIButton* const button = native_button(handler);
        send_control_event(button, UIControlEventTouchDown); // ImageButtonProxy.OnButtonTouchDown
        EXPECT_TRUE(control.is_pressed());
        send_control_event(button, UIControlEventTouchUpInside); // Released, then Clicked
        ASSERT_EQ(order.size(), 3U);
        EXPECT_EQ(order[0], "pressed");
        EXPECT_EQ(order[1], "released");
        EXPECT_EQ(order[2], "clicked");
        EXPECT_FALSE(control.is_pressed());
    }

    TEST(ios_image_button_seam, touch_up_outside_releases_without_click)
    {
        image_button control;
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);

        std::vector<std::string> order;
        control.released.connect([&order] { order.emplace_back("released"); });
        control.clicked.connect([&order] { order.emplace_back("clicked"); });

        UIButton* const button = native_button(handler);
        send_control_event(button, UIControlEventTouchDown);
        send_control_event(button, UIControlEventTouchUpOutside); // ImageButtonProxy.OnButtonTouchUpOutside
        ASSERT_EQ(order.size(), 1U);
        EXPECT_EQ(order[0], "released");
    }

    TEST(ios_image_button_seam, generic_iview_properties_reach_the_uibutton)
    {
        image_button control;
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);
        UIButton* const button = native_button(handler);

        control.set_is_enabled(false);
        EXPECT_FALSE(button.enabled);
        control.set_opacity(0.5);
        EXPECT_EQ(button.alpha, 0.5);
        control.set_visibility(maui::core::visibility::hidden);
        EXPECT_TRUE(button.hidden);
        control.set_automation_id("image-button-id");
        EXPECT_EQ(to_std_string(button.accessibilityIdentifier), "image-button-id");
    }

    TEST(ios_image_button_seam, clearing_handler_disconnects)
    {
        image_button control;
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);
        UIButton* const button = native_button(handler);
        EXPECT_GT(button.allTargets.count, 0U); // the proxy registration

        control.set_handler(nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    // Regression: a large image in an aspect-fit image_button must measure to fit its WIDTH constraint, not
    // the raw image pixel size. get_desired_size ports ImageButton.iOS.cs CrossPlatformMeasure, which uses
    // ImageView.SizeThatFitsImage (aspect-aware, respects constraints) — the shared size_that_fits_image.
    // Before the fix, a raw -[UIButton sizeThatFits:] returned the full 256px source, which
    // content_is_minimum_size-style flooring in view::measure would have blown up the layout.
    TEST(ios_image_button_seam, large_image_measures_to_fit_the_width_constraint)
    {
        const std::string path = write_temp_png_sized(256);
        ASSERT_FALSE(path.empty());

        image_button control;
        control.set_aspect(aspect::aspect_fit); // the default; aspect-fit drives the aspect-aware fit
        control.set_source(image_source::from_file(path));
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);

        // A 100pt-wide cell: the 256px square must aspect-fit down to ~100x100, not report ~256px.
        const maui::graphics::size measured = handler->get_desired_size(100.0, 600.0);
        EXPECT_LE(measured.width, 100.5);
        EXPECT_LE(measured.height, 100.5); // square image → height tracks the shrunk width
        EXPECT_GT(measured.width, 0.0);
        remove_file(path);
    }

    // No image → the source-less button keeps the native -[UIButton sizeThatFits:] fallback (C#'s else
    // branch). This guards that the image branch does not swallow the empty case.
    TEST(ios_image_button_seam, no_image_uses_the_native_size_that_fits_fallback)
    {
        image_button control;
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);

        UIButton* const button = native_button(handler);
        const CGSize native = [button sizeThatFits:CGSizeMake(CGFLOAT_MAX, CGFLOAT_MAX)];
        const maui::graphics::size measured =
            handler->get_desired_size(std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
        EXPECT_NEAR(measured.width, native.width, 0.5);
        EXPECT_NEAR(measured.height, native.height, 0.5);
    }

    TEST(ios_image_button_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<image_button>();
        ASSERT_NE(handler, nullptr);
        EXPECT_NE(dynamic_cast<image_button_handler*>(handler.get()), nullptr);
    }
} // namespace
