// Apple (AppKit) backend tests for the shared ViewMapper — the generic IView properties (Visibility /
// Opacity / IsEnabled / AutomationId) pushed to a real NSView via view_platform_base overrides. Run only
// for MAUI_BACKEND=apple. Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>

#include <memory>
#include <string>

#include "../../src/platform/apple/apple_view_ops.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/entry_handler.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/image_handler.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
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
    using maui::platform::apple::apply_flow_direction;
    using maui::platform::apple::apply_transform;

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

    struct apple_view_mapper : ::testing::Test
    {
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
        EXPECT_EQ(std::string(view.accessibilityIdentifier.UTF8String), "submit_button");
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
        EXPECT_EQ(std::string(view.accessibilityIdentifier.UTF8String), "caption");
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
        EXPECT_EQ(std::string(view.accessibilityIdentifier.UTF8String), "email_entry");
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
        EXPECT_EQ(std::string(view.accessibilityIdentifier.UTF8String), "avatar");
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
        EXPECT_EQ(std::string(view.accessibilityIdentifier.UTF8String), "form_stack");
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
} // namespace
