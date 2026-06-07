// Apple (AppKit) backend tests for the shared ViewMapper — the generic IView properties (Visibility /
// Opacity / IsEnabled / AutomationId) pushed to a real NSView via view_platform_base overrides. Run only
// for MAUI_BACKEND=apple. Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>

#include <memory>
#include <string>

#include "../../src/platform/apple/apple_view_ops.hpp"
#include "../../src/platform/apple/apple_visual_ops.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/entry_handler.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/i_shadow.hpp"
#include "maui/core/image_handler.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/shadow.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/shapes/ellipse.hpp"
#include "maui/graphics/shapes/rectangle.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "maui/graphics/solid_paint.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::button;
    using maui::controls::entry;
    using maui::controls::image;
    using maui::controls::label;
    using maui::controls::vertical_stack_layout;
    using maui::core::button_handler;
    using maui::core::entry_handler;
    using maui::core::flow_direction;
    using maui::core::image_handler;
    using maui::core::label_handler;
    using maui::core::layout_handler;
    using maui::core::transform_spec;
    using maui::core::visibility;
    using maui::platform::apple::apply_background;
    using maui::platform::apple::apply_clip;
    using maui::platform::apple::apply_flow_direction;
    using maui::platform::apple::apply_shadow;
    using maui::platform::apple::apply_transform;

    // -[NSString UTF8String] is nullable-annotated; convert through this guard so the std::string
    // construction never receives a null pointer (the values under test are always non-null).
    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    NSButton* native_button(const std::shared_ptr<button_handler>& handler)
    {
        return (__bridge NSButton*)handler->typed_platform_view()->native;
    }

    NSTextField* native_label(const std::shared_ptr<label_handler>& handler)
    {
        return (__bridge NSTextField*)handler->typed_platform_view()->native;
    }

    NSTextField* native_entry(const std::shared_ptr<entry_handler>& handler)
    {
        return (__bridge NSTextField*)handler->typed_platform_view()->native;
    }

    NSImageView* native_image(const std::shared_ptr<image_handler>& handler)
    {
        return (__bridge NSImageView*)handler->typed_platform_view()->native;
    }

    NSView* native_panel(const std::shared_ptr<layout_handler>& handler)
    {
        return (__bridge NSView*)handler->typed_platform_view()->native;
    }

    class apple_view_mapper : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_view_mapper, button_generic_properties_push_to_nsbutton)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        NSButton* const view = native_button(handler);

        control.set_visibility(visibility::hidden);
        EXPECT_TRUE(view.hidden);
        control.set_visibility(visibility::visible);
        EXPECT_FALSE(view.hidden);

        control.set_opacity(0.5);
        EXPECT_EQ(view.alphaValue, 0.5);

        control.set_is_enabled(false);
        EXPECT_FALSE(view.enabled);
        control.set_is_enabled(true);
        EXPECT_TRUE(view.enabled);

        control.set_automation_id("submit_button");
        EXPECT_EQ(to_std_string(view.accessibilityIdentifier), "submit_button");
    }

    // End-to-end: a render-transform change on the control reaches the NSButton's backing layer through
    // the per-control update_transform override (the M4c retrofit wiring), not just the helper in isolation.
    TEST_F(apple_view_mapper, control_transform_reaches_the_layer)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        NSButton* const view = native_button(handler);
        control.set_scale(2.0);
        EXPECT_TRUE(view.wantsLayer);
        EXPECT_DOUBLE_EQ(view.layer.transform.m11, 2.0); // uniform scale lands on the layer
    }

    TEST_F(apple_view_mapper, label_generic_properties_push_to_nstextfield)
    {
        label control;
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        NSTextField* const view = native_label(handler);

        control.set_visibility(visibility::collapsed);
        EXPECT_TRUE(view.hidden);

        control.set_opacity(0.25);
        EXPECT_EQ(view.alphaValue, 0.25);

        control.set_is_enabled(false);
        EXPECT_FALSE(view.enabled);

        control.set_automation_id("caption");
        EXPECT_EQ(to_std_string(view.accessibilityIdentifier), "caption");
    }

    TEST_F(apple_view_mapper, entry_generic_properties_push_to_nstextfield)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        NSTextField* const view = native_entry(handler);

        control.set_visibility(visibility::hidden);
        EXPECT_TRUE(view.hidden);

        control.set_opacity(0.6);
        EXPECT_EQ(view.alphaValue, 0.6);

        control.set_is_enabled(false);
        EXPECT_FALSE(view.enabled);

        control.set_automation_id("email_entry");
        EXPECT_EQ(to_std_string(view.accessibilityIdentifier), "email_entry");
    }

    TEST_F(apple_view_mapper, image_generic_properties_push_to_nsimageview)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        NSImageView* const view = native_image(handler);

        control.set_visibility(visibility::collapsed);
        EXPECT_TRUE(view.hidden);

        control.set_opacity(0.3);
        EXPECT_EQ(view.alphaValue, 0.3);

        control.set_is_enabled(false);
        EXPECT_FALSE(view.enabled);

        control.set_automation_id("avatar");
        EXPECT_EQ(to_std_string(view.accessibilityIdentifier), "avatar");
    }

    // The layout panel is a plain NSView — it has no native enabled state, so is_enabled is not asserted
    // here (it keeps the headless base mirror, covered by the headless view_mapper_layout test).
    TEST_F(apple_view_mapper, layout_generic_properties_push_to_nsview)
    {
        vertical_stack_layout control;
        auto handler = std::make_shared<layout_handler>();
        control.set_handler(handler);
        NSView* const view = native_panel(handler);

        control.set_visibility(visibility::hidden);
        EXPECT_TRUE(view.hidden);

        control.set_opacity(0.4);
        EXPECT_EQ(view.alphaValue, 0.4);

        control.set_automation_id("form_stack");
        EXPECT_EQ(to_std_string(view.accessibilityIdentifier), "form_stack");
    }

    // ---- the shared apple_view_ops helpers (the coordinator's per-control retrofit calls these) ----

    // A pure uniform/per-axis scale (identity anchor, no translation/rotation) lands on the layer's
    // transform diagonal: m11 = scale_x * scale, m22 = scale_y * scale, m33 = scale.
    TEST_F(apple_view_mapper, apply_transform_scale_lands_on_layer)
    {
        NSView* const view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 100, 40)];
        view.wantsLayer = YES;
        void* const native = (__bridge void*)view;

        apply_transform(native, transform_spec{.scale = 2.0, .scale_x = 3.0, .scale_y = 4.0});

        const CATransform3D transform = view.layer.transform;
        EXPECT_DOUBLE_EQ(transform.m11, 6.0); // scale_x * scale
        EXPECT_DOUBLE_EQ(transform.m22, 8.0); // scale_y * scale
        EXPECT_DOUBLE_EQ(transform.m33, 2.0); // z = scale
        EXPECT_DOUBLE_EQ(view.layer.anchorPoint.x, 0.5);
        EXPECT_DOUBLE_EQ(view.layer.anchorPoint.y, 0.5);
    }

    // The anchor point is always pushed to the layer (even at the default 0.5/0.5).
    TEST_F(apple_view_mapper, apply_transform_sets_anchor_point)
    {
        NSView* const view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 100, 40)];
        view.wantsLayer = YES;
        void* const native = (__bridge void*)view;

        apply_transform(native, transform_spec{.anchor_x = 0.0, .anchor_y = 1.0});

        EXPECT_DOUBLE_EQ(view.layer.anchorPoint.x, 0.0);
        EXPECT_DOUBLE_EQ(view.layer.anchorPoint.y, 1.0);
    }

    // An out-of-plane rotation engages the perspective term m34; a plain in-plane (z) rotation does not.
    // (Like the C# original, m34 is set before the rotation is composed, so the final value is the
    // -1/400 perspective folded through the rotation — what matters is that it is engaged vs. absent.)
    TEST_F(apple_view_mapper, apply_transform_perspective_only_for_out_of_plane_rotation)
    {
        NSView* const flat = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 100, 40)];
        flat.wantsLayer = YES;
        apply_transform((__bridge void*)flat, transform_spec{.rotation = 30.0});
        EXPECT_DOUBLE_EQ(flat.layer.transform.m34, 0.0);

        NSView* const tilted = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 100, 40)];
        tilted.wantsLayer = YES;
        apply_transform((__bridge void*)tilted, transform_spec{.rotation_x = 30.0});
        EXPECT_NE(tilted.layer.transform.m34, 0.0);
    }

    // The identity spec leaves the layer transform at identity.
    TEST_F(apple_view_mapper, apply_transform_identity_is_identity)
    {
        NSView* const view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 100, 40)];
        view.wantsLayer = YES;
        apply_transform((__bridge void*)view, transform_spec{});
        EXPECT_TRUE(CATransform3DIsIdentity(view.layer.transform));
    }

    TEST_F(apple_view_mapper, apply_flow_direction_sets_layout_direction)
    {
        NSView* const view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 100, 40)];
        void* const native = (__bridge void*)view;

        apply_flow_direction(native, flow_direction::right_to_left);
        EXPECT_EQ(view.userInterfaceLayoutDirection, NSUserInterfaceLayoutDirectionRightToLeft);

        apply_flow_direction(native, flow_direction::left_to_right);
        EXPECT_EQ(view.userInterfaceLayoutDirection, NSUserInterfaceLayoutDirectionLeftToRight);
    }

    // ---- the visual-layer apple helpers (apply_background / apply_shadow / apply_clip) ----

    // A solid-paint background lands on the layer's backgroundColor with the paint's sRGB components.
    TEST_F(apple_view_mapper, apply_background_solid_paint_sets_layer_color)
    {
        NSView* const view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 100, 40)];
        view.wantsLayer = YES;
        const maui::graphics::solid_paint paint{maui::graphics::color{0.25F, 0.5F, 0.75F, 1.0F}};
        apply_background((__bridge void*)view, &paint);

        CGColorRef bg = view.layer.backgroundColor;
        ASSERT_NE(bg, nullptr);
        NSColor* const c = [NSColor colorWithCGColor:bg];
        ASSERT_NE(c, nil);
        NSColor* const srgb = [c colorUsingColorSpace:NSColorSpace.sRGBColorSpace];
        ASSERT_NE(srgb, nil);
        EXPECT_NEAR(srgb.redComponent, 0.25, 1e-4);
        EXPECT_NEAR(srgb.greenComponent, 0.5, 1e-4);
        EXPECT_NEAR(srgb.blueComponent, 0.75, 1e-4);
    }

    // A null paint clears the layer's backgroundColor.
    TEST_F(apple_view_mapper, apply_background_null_clears_layer_color)
    {
        NSView* const view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 100, 40)];
        view.wantsLayer = YES;
        const maui::graphics::solid_paint paint{maui::graphics::colors::red};
        apply_background((__bridge void*)view, &paint);
        ASSERT_NE(view.layer.backgroundColor, nullptr);

        apply_background((__bridge void*)view, nullptr);
        EXPECT_EQ(view.layer.backgroundColor, nullptr);
    }

    // A shadow sets the layer's shadow properties; ShadowRadius is Radius/2 (exactly as the C# extension).
    TEST_F(apple_view_mapper, apply_shadow_sets_layer_shadow)
    {
        NSView* const view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 100, 40)];
        view.wantsLayer = YES;
        maui::core::shadow sh;
        sh.set_radius(10.0);
        sh.set_opacity(0.5);
        sh.set_offset(maui::graphics::point(3, 6));
        apply_shadow((__bridge void*)view, &sh);

        EXPECT_DOUBLE_EQ(view.layer.shadowRadius, 5.0); // radius / 2
        EXPECT_FLOAT_EQ(view.layer.shadowOpacity, 0.5F);
        EXPECT_DOUBLE_EQ(view.layer.shadowOffset.width, 3.0);
        EXPECT_DOUBLE_EQ(view.layer.shadowOffset.height, 6.0);
        ASSERT_NE(view.layer.shadowColor, nullptr); // black paint default
    }

    // A null shadow clears the layer shadow (radius / opacity / offset back to zero).
    TEST_F(apple_view_mapper, apply_shadow_null_clears_layer_shadow)
    {
        NSView* const view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 100, 40)];
        view.wantsLayer = YES;
        maui::core::shadow sh;
        apply_shadow((__bridge void*)view, &sh);
        EXPECT_GT(view.layer.shadowOpacity, 0.0F);

        apply_shadow((__bridge void*)view, nullptr);
        EXPECT_FLOAT_EQ(view.layer.shadowOpacity, 0.0F);
        EXPECT_DOUBLE_EQ(view.layer.shadowRadius, 0.0);
    }

    // A clip shape installs a CAShapeLayer mask whose path bounding box matches the shape for the bounds.
    TEST_F(apple_view_mapper, apply_clip_rectangle_sets_shape_mask)
    {
        NSView* const view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 50, 20)];
        view.wantsLayer = YES;
        const maui::graphics::shapes::rectangle shape;
        apply_clip((__bridge void*)view, &shape, maui::graphics::rect(0, 0, 50, 20));

        ASSERT_NE(view.layer.mask, nil);
        CAShapeLayer* const mask = static_cast<CAShapeLayer*>(view.layer.mask);
        EXPECT_TRUE([mask isKindOfClass:[CAShapeLayer class]]);
        ASSERT_NE(mask.path, nullptr);
        const CGRect box = CGPathGetBoundingBox(mask.path);
        EXPECT_NEAR(box.size.width, 50.0, 1e-3);
        EXPECT_NEAR(box.size.height, 20.0, 1e-3);
    }

    // An ellipse clip likewise installs a mask; its path bounding box matches the bounds (curves are exact
    // to the control points for an ellipse — the box equals the rect).
    TEST_F(apple_view_mapper, apply_clip_ellipse_sets_shape_mask)
    {
        NSView* const view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 30, 40)];
        view.wantsLayer = YES;
        const maui::graphics::shapes::ellipse shape;
        apply_clip((__bridge void*)view, &shape, maui::graphics::rect(0, 0, 30, 40));

        ASSERT_NE(view.layer.mask, nil);
        CAShapeLayer* const mask = static_cast<CAShapeLayer*>(view.layer.mask);
        ASSERT_NE(mask.path, nullptr);
        const CGRect box = CGPathGetBoundingBox(mask.path);
        EXPECT_NEAR(box.size.width, 30.0, 0.5);
        EXPECT_NEAR(box.size.height, 40.0, 0.5);
    }

    // A round-rectangle clip installs a mask spanning the bounds.
    TEST_F(apple_view_mapper, apply_clip_round_rectangle_sets_shape_mask)
    {
        NSView* const view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 60, 40)];
        view.wantsLayer = YES;
        const maui::graphics::shapes::round_rectangle shape{8.0};
        apply_clip((__bridge void*)view, &shape, maui::graphics::rect(0, 0, 60, 40));

        ASSERT_NE(view.layer.mask, nil);
        CAShapeLayer* const mask = static_cast<CAShapeLayer*>(view.layer.mask);
        ASSERT_NE(mask.path, nullptr);
        const CGRect box = CGPathGetBoundingBox(mask.path);
        EXPECT_NEAR(box.size.width, 60.0, 0.5);
        EXPECT_NEAR(box.size.height, 40.0, 0.5);
    }

    // A null shape removes the mask (WrapperView.SetClip with clip == null).
    TEST_F(apple_view_mapper, apply_clip_null_removes_mask)
    {
        NSView* const view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 50, 20)];
        view.wantsLayer = YES;
        const maui::graphics::shapes::rectangle shape;
        apply_clip((__bridge void*)view, &shape, maui::graphics::rect(0, 0, 50, 20));
        ASSERT_NE(view.layer.mask, nil);

        apply_clip((__bridge void*)view, nullptr, maui::graphics::rect(0, 0, 50, 20));
        EXPECT_EQ(view.layer.mask, nil);
    }
} // namespace
