// iOS (UIKit) backend tests for the layout seam — children become real UIView subviews of the layout
// panel after add (and leave on remove/clear), z-ordered by GetLayoutHandlerIndex; ClipsToBounds drives
// the REAL UIView.clipsToBounds property. Each child here is a button (its handler owns a real UIButton
// — label/entry are still headless on ios, so they have no native view to host). ALSO the on-simulator
// proof of the shared ios op headers this unit ships (ios_visual_ops.hpp / ios_semantics_ops.hpp): the
// generic-IView visual + a11y pushes reach the panel through layout_platform's update_* overrides, and
// the gradient/clip/shadow grammar is exercised directly on a raw UIView (the per-control retrofit will
// reuse exactly these helpers). Run only for MAUI_BACKEND=ios (executed ON the iOS simulator via
// tools/ios-sim-run.sh). Compiled as Objective-C++ with ARC.
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>

#include <memory>
#include <string>
#include <vector>

#include "ios_semantics_ops.hpp"
#include "ios_visual_ops.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/semantics.hpp"
#include "maui/core/shadow.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/gradient_stop.hpp"
#include "maui/graphics/linear_gradient_paint.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/radial_gradient_paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/shapes/rectangle.hpp"
#include "maui/graphics/solid_paint.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::button;
    using maui::controls::vertical_stack_layout;
    using maui::core::button_handler;
    using maui::core::layout_handler;
    using maui::graphics::gradient_stop;
    using maui::graphics::linear_gradient_paint;
    using maui::graphics::radial_gradient_paint;
    using maui::platform::ios::apply_background;

    // -[NSString UTF8String] is nullable-annotated; convert through this guard so the std::string
    // construction never receives a null pointer (the values under test are always non-null).
    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    UIView* native_panel(const std::shared_ptr<layout_handler>& handler)
    {
        return (__bridge UIView*)handler->typed_platform_view()->native;
    }

    // A button with its handler attached, so it owns a real native UIButton the panel can host. Returns
    // the child's native UIView for superview assertions.
    UIView* attach_button(button& control)
    {
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        return (__bridge UIView*)handler->native_view();
    }

    // sRGB component readback of a CGColorRef via UIColor's named accessors (no raw
    // CGColorGetComponents pointer indexing).
    void expect_srgb(CGColorRef cg, double red, double green, double blue, double alpha)
    {
        ASSERT_NE(cg, nullptr);
        UIColor* const color = [UIColor colorWithCGColor:cg];
        CGFloat r = 0;
        CGFloat g = 0;
        CGFloat b = 0;
        CGFloat a = 0;
        ASSERT_TRUE([color getRed:&r green:&g blue:&b alpha:&a]);
        EXPECT_NEAR(r, red, 1e-4);
        EXPECT_NEAR(g, green, 1e-4);
        EXPECT_NEAR(b, blue, 1e-4);
        EXPECT_NEAR(a, alpha, 1e-4);
    }

    // Locate the CAGradientLayer apply_background installs as a sublayer (tagged by name).
    CAGradientLayer* find_gradient_layer(UIView* view)
    {
        NSArray<CALayer*>* const sublayers = view.layer.sublayers;
        for (NSUInteger i = 0; i < sublayers.count; i++)
        {
            CALayer* const sub = sublayers[i];
            if ([sub isKindOfClass:[CAGradientLayer class]] && [sub.name isEqualToString:@"maui.background.gradient"])
            {
                return static_cast<CAGradientLayer*>(sub);
            }
        }
        return nil;
    }

    // Count the gradient sublayers apply_background installed (tagged by name) on a view's layer.
    int gradient_layer_count(UIView* view)
    {
        int count = 0;
        NSArray<CALayer*>* const sublayers = view.layer.sublayers;
        for (NSUInteger i = 0; i < sublayers.count; i++)
        {
            if ([sublayers[i].name isEqualToString:@"maui.background.gradient"])
            {
                count++;
            }
        }
        return count;
    }

    // ---- the layout seam (mirrors layout_apple_tests.mm on UIKit) ----

    TEST(ios_layout_seam, panel_is_a_uiview)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_TRUE([native_panel(handler) isKindOfClass:[UIView class]]);
        EXPECT_EQ(native_panel(handler).subviews.count, 0U);
    }

    TEST(ios_layout_seam, added_child_becomes_a_subview)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);

        // A child with its own native view (a button-backed UIButton). native_view() returns the real
        // UIButton the handler's pimpl owns (platform_view() would return the pimpl pointer itself).
        button child;
        UIView* const child_native = attach_button(child);
        ASSERT_NE(child_native, nil);

        stack.add(child); // -> handler->invoke("add", …) -> map_add -> add() -> insertSubview:atIndex:

        EXPECT_EQ(native_panel(handler).subviews.count, 1U);
        EXPECT_EQ(child_native.superview, native_panel(handler));
        EXPECT_EQ(handler->typed_platform_view()->children.size(), 1U);
    }

    TEST(ios_layout_seam, arrange_sizes_the_panel)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);

        stack.arrange(maui::graphics::rect(5, 10, 200, 120)); // -> handler->platform_arrange sizes the panel

        const CGRect frame = native_panel(handler).frame;
        EXPECT_EQ(frame.origin.x, 5.0);
        EXPECT_EQ(frame.origin.y, 10.0);
        EXPECT_EQ(frame.size.width, 200.0);
        EXPECT_EQ(frame.size.height, 120.0);
    }

    TEST(ios_layout_seam, removed_child_leaves_the_panel)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);

        button first;
        attach_button(first);
        button second;
        attach_button(second);

        stack.add(first);
        stack.add(second);
        EXPECT_EQ(native_panel(handler).subviews.count, 2U);

        stack.remove_at(0);
        EXPECT_EQ(native_panel(handler).subviews.count, 1U);

        stack.clear();
        EXPECT_EQ(native_panel(handler).subviews.count, 0U);
    }

    TEST(ios_layout_seam, clips_to_bounds_sets_the_real_uiview_flag)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);

        // LayoutViewExtensions.UpdateClipsToBounds drives UIView.ClipsToBounds — the REAL UIKit property.
        stack.set_clips_to_bounds(true); // -> map_clips_to_bounds -> panel.clipsToBounds = YES
        EXPECT_TRUE(native_panel(handler).clipsToBounds);

        stack.set_clips_to_bounds(false);
        EXPECT_FALSE(native_panel(handler).clipsToBounds);
    }

    TEST(ios_layout_seam, subviews_stack_by_z_index)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);

        button high;
        high.set_z_index(10);
        UIView* const high_native = attach_button(high);

        button low;
        low.set_z_index(0);
        UIView* const low_native = attach_button(low);

        stack.add(high); // higher z added first, but should end up on top (last subview)
        stack.add(low);

        NSArray<UIView*>* const subviews = native_panel(handler).subviews;
        ASSERT_EQ(subviews.count, 2U);
        EXPECT_EQ(subviews[0], low_native);  // lower z at the bottom
        EXPECT_EQ(subviews[1], high_native); // higher z on top

        // A runtime z-index change re-stacks the subview (routes through the parent layout's handler:
        // EnsureZIndexOrder -> remove + insertSubview:atIndex:).
        low.set_z_index(20);
        NSArray<UIView*>* const reordered = native_panel(handler).subviews;
        EXPECT_EQ(reordered[0], high_native);
        EXPECT_EQ(reordered[1], low_native);
    }

    // The generic-IView pushes (the shared view_mapper through layout_platform's ios update_*
    // overrides): visibility / opacity / automation_id reach the real UIView panel. (is_enabled keeps
    // the base mirror — a plain UIView has no enabled state, only UIControl has.)
    TEST(ios_layout_seam, generic_iview_properties_reach_the_panel)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);
        UIView* const view = native_panel(handler);

        stack.set_visibility(maui::core::visibility::hidden);
        EXPECT_TRUE(view.hidden);
        stack.set_visibility(maui::core::visibility::visible);
        EXPECT_FALSE(view.hidden);

        // 0.25 is exactly representable in float AND double: UIKit stores UIView.alpha through a float
        // internally (0.4 would read back as 0.40000000596…), unlike NSView's double alphaValue.
        stack.set_opacity(0.25);
        EXPECT_EQ(view.alpha, 0.25);

        stack.set_automation_id("form_stack");
        EXPECT_EQ(to_std_string(view.accessibilityIdentifier), "form_stack");
    }

    // Semantics + InputTransparent reach the panel through the layout_platform overrides:
    // Description/Hint -> accessibilityLabel/Hint (+ the panel becomes an a11y element), IsHeading ->
    // the Header trait, InputTransparent -> userInteractionEnabled (UIKit's native flag).
    TEST(ios_layout_seam, semantics_and_input_transparent_reach_the_panel)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);
        UIView* const view = native_panel(handler);

        auto sem = std::make_shared<maui::core::semantics>();
        sem->set_description("Form");
        sem->set_hint("Groups the inputs");
        stack.set_semantics(sem);
        EXPECT_EQ(to_std_string(view.accessibilityLabel), "Form");
        EXPECT_EQ(to_std_string(view.accessibilityHint), "Groups the inputs");
        EXPECT_TRUE(view.isAccessibilityElement); // marked an element because a label/hint is present
        EXPECT_EQ(view.accessibilityTraits & UIAccessibilityTraitHeader, 0U);

        stack.set_input_transparent(true); // UserInteractionEnabled = !InputTransparent
        EXPECT_FALSE(view.userInteractionEnabled);
        stack.set_input_transparent(false);
        EXPECT_TRUE(view.userInteractionEnabled);
    }

    // ---- the visual-layer pushes through layout_platform (ios_visual_ops via update_*) ----

    TEST(ios_layout_seam, solid_background_reaches_the_panel_layer)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);
        UIView* const view = native_panel(handler);

        stack.set_background(
            std::make_shared<maui::graphics::solid_paint>(maui::graphics::color{0.25F, 0.5F, 0.75F, 1.0F}));
        expect_srgb(view.layer.backgroundColor, 0.25, 0.5, 0.75, 1.0);

        stack.set_background(nullptr); // a null paint clears the layer color
        EXPECT_EQ(view.layer.backgroundColor, nullptr);
    }

    TEST(ios_layout_seam, gradient_background_installs_axial_layer)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);
        UIView* const view = native_panel(handler);
        [view setFrame:CGRectMake(0, 0, 100, 40)];

        stack.set_background(std::make_shared<linear_gradient_paint>(
            std::vector<gradient_stop>{gradient_stop(0.0F, maui::graphics::color{1.0F, 0.0F, 0.0F, 1.0F}),
                                       gradient_stop(1.0F, maui::graphics::color{0.0F, 0.0F, 1.0F, 1.0F})},
            maui::graphics::point(0, 0), maui::graphics::point(1, 1)));

        CAGradientLayer* const gradient = find_gradient_layer(view);
        ASSERT_NE(gradient, nil);
        EXPECT_TRUE([gradient.type isEqualToString:kCAGradientLayerAxial]);
        EXPECT_DOUBLE_EQ(gradient.startPoint.x, 0.0);
        EXPECT_DOUBLE_EQ(gradient.endPoint.x, 1.0);
        // sized to the backing layer bounds
        EXPECT_DOUBLE_EQ(gradient.frame.size.width, 100.0);
        EXPECT_DOUBLE_EQ(gradient.frame.size.height, 40.0);
        ASSERT_EQ(gradient.colors.count, 2U);
        ASSERT_EQ(gradient.locations.count, 2U);
        EXPECT_FLOAT_EQ(gradient.locations[0].floatValue, 0.0F);
        EXPECT_FLOAT_EQ(gradient.locations[1].floatValue, 1.0F);
        // the base layer color is cleared while a gradient is active
        EXPECT_EQ(view.layer.backgroundColor, nullptr);

        stack.set_background(nullptr); // a null paint removes the gradient sublayer too
        EXPECT_EQ(find_gradient_layer(view), nil);
    }

    // A shadow sets the layer's shadow properties; ShadowRadius is Radius/2 (exactly as the C#
    // extension); a null shadow clears them (ShadowExtensions.ClearShadow).
    TEST(ios_layout_seam, shadow_reaches_the_panel_layer_and_clears)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);
        UIView* const view = native_panel(handler);

        auto sh = std::make_shared<maui::core::shadow>();
        sh->set_radius(10.0);
        sh->set_opacity(0.5);
        sh->set_offset(maui::graphics::point(3, 6));
        stack.set_shadow(sh);
        EXPECT_DOUBLE_EQ(view.layer.shadowRadius, 5.0); // radius / 2
        EXPECT_FLOAT_EQ(view.layer.shadowOpacity, 0.5F);
        EXPECT_DOUBLE_EQ(view.layer.shadowOffset.width, 3.0);
        EXPECT_DOUBLE_EQ(view.layer.shadowOffset.height, 6.0);
        ASSERT_NE(view.layer.shadowColor, nullptr); // black paint default

        stack.set_shadow(nullptr);
        EXPECT_FLOAT_EQ(view.layer.shadowOpacity, 0.0F);
        EXPECT_DOUBLE_EQ(view.layer.shadowRadius, 0.0);
    }

    // A clip shape installs a CAShapeLayer mask whose path bounding box matches the shape for the
    // panel's bounds; a null shape removes the mask (WrapperView.SetClip).
    TEST(ios_layout_seam, clip_installs_and_removes_the_shape_mask)
    {
        vertical_stack_layout stack;
        auto handler = std::make_shared<layout_handler>();
        stack.set_handler(handler);
        UIView* const view = native_panel(handler);
        [view setFrame:CGRectMake(0, 0, 50, 20)];

        stack.set_clip(std::make_shared<maui::graphics::shapes::rectangle>());
        ASSERT_NE(view.layer.mask, nil);
        CAShapeLayer* const mask = static_cast<CAShapeLayer*>(view.layer.mask);
        EXPECT_TRUE([mask isKindOfClass:[CAShapeLayer class]]);
        ASSERT_NE(mask.path, nullptr);
        const CGRect box = CGPathGetBoundingBox(mask.path);
        EXPECT_NEAR(box.size.width, 50.0, 1e-3);
        EXPECT_NEAR(box.size.height, 20.0, 1e-3);

        stack.set_clip(nullptr);
        EXPECT_EQ(view.layer.mask, nil);
    }

    // ---- the shared ios_visual_ops gradient grammar, driven directly on a raw UIView (the same
    // helpers every control's retrofit override will call) ----

    // A radial gradient installs a radial CAGradientLayer with the center as startPoint, the computed
    // endPoint (GetRadialGradientPaintEndPoint, clamped to [0,1]), and cornerRadius == radius.
    TEST(ios_visual_ops, radial_gradient_installs_radial_layer)
    {
        UIView* const view = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 80, 60)];
        const radial_gradient_paint paint{
            std::vector<gradient_stop>{gradient_stop(0.0F, maui::graphics::color{0.0F, 1.0F, 0.0F, 1.0F}),
                                       gradient_stop(1.0F, maui::graphics::color{0.0F, 0.0F, 0.0F, 1.0F})},
            maui::graphics::point(0.5, 0.5), 0.5};
        apply_background((__bridge void*)view, &paint);

        CAGradientLayer* const gradient = find_gradient_layer(view);
        ASSERT_NE(gradient, nil);
        EXPECT_TRUE([gradient.type isEqualToString:kCAGradientLayerRadial]);
        EXPECT_DOUBLE_EQ(gradient.startPoint.x, 0.5);
        EXPECT_DOUBLE_EQ(gradient.startPoint.y, 0.5);
        // GetRadialGradientPaintEndPoint(center 0.5, radius 0.5) -> (1.0, 1.0), clamped to [0,1].
        EXPECT_DOUBLE_EQ(gradient.endPoint.x, 1.0);
        EXPECT_DOUBLE_EQ(gradient.endPoint.y, 1.0);
        EXPECT_DOUBLE_EQ(gradient.cornerRadius, 0.5);
        ASSERT_EQ(gradient.colors.count, 2U);
    }

    // A transparent stop borrows its neighbor's color at alpha 0 (PaintExtensions
    // GetCAGradientLayerColors), so the gradient fades to the adjacent hue rather than to black.
    TEST(ios_visual_ops, transparent_stop_borrows_neighbor_color)
    {
        UIView* const view = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 100, 40)];
        const linear_gradient_paint paint{
            std::vector<gradient_stop>{gradient_stop(0.0F, maui::graphics::color{1.0F, 0.0F, 0.0F, 1.0F}),
                                       gradient_stop(1.0F, maui::graphics::colors::transparent)}};
        apply_background((__bridge void*)view, &paint);

        CAGradientLayer* const gradient = find_gradient_layer(view);
        ASSERT_NE(gradient, nil);
        ASSERT_EQ(gradient.colors.count, 2U);
        // The transparent (index 1) stop borrows the red (index 0) hue at alpha 0.
        expect_srgb((__bridge CGColorRef)gradient.colors[1], 1.0, 0.0, 0.0, 0.0);
    }

    // A single transparent stop must not read a non-existent neighbor (C# would index out of range; the
    // C++ port keeps the stop's own transparent color). Regression guard for the one-stop borrow path.
    TEST(ios_visual_ops, single_transparent_stop_is_safe)
    {
        UIView* const view = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 100, 40)];
        const linear_gradient_paint paint{
            std::vector<gradient_stop>{gradient_stop(0.0F, maui::graphics::colors::transparent)}};
        apply_background((__bridge void*)view, &paint); // must not crash / read OOB

        CAGradientLayer* const gradient = find_gradient_layer(view);
        ASSERT_NE(gradient, nil);
        ASSERT_EQ(gradient.colors.count, 1U);
        UIColor* const only = [UIColor colorWithCGColor:(__bridge CGColorRef)gradient.colors[0]];
        EXPECT_NEAR(CGColorGetAlpha(only.CGColor), 0.0, 1e-4); // the stop's own transparent color
    }

    // With more than one stop where all offsets are zero, locations spread evenly (1/count steps).
    TEST(ios_visual_ops, locations_spread_evenly_when_all_offsets_zero)
    {
        UIView* const view = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 100, 40)];
        const linear_gradient_paint paint{
            std::vector<gradient_stop>{gradient_stop(0.0F, maui::graphics::color{1.0F, 0.0F, 0.0F, 1.0F}),
                                       gradient_stop(0.0F, maui::graphics::color{0.0F, 0.0F, 1.0F, 1.0F})}};
        apply_background((__bridge void*)view, &paint);

        CAGradientLayer* const gradient = find_gradient_layer(view);
        ASSERT_NE(gradient, nil);
        ASSERT_EQ(gradient.locations.count, 2U);
        // step = 1/2 = 0.5; index 0 -> 0.0, index 1 -> 0.5.
        EXPECT_FLOAT_EQ(gradient.locations[0].floatValue, 0.0F);
        EXPECT_FLOAT_EQ(gradient.locations[1].floatValue, 0.5F);
    }

    // Re-applying a gradient replaces the prior one (no stale duplicate sublayers); switching to a solid
    // removes the gradient and sets the base color.
    TEST(ios_visual_ops, gradient_replaced_then_solid_clears_gradient)
    {
        UIView* const view = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 100, 40)];
        const linear_gradient_paint first{maui::graphics::point(0, 0), maui::graphics::point(1, 0)};
        apply_background((__bridge void*)view, &first);
        const radial_gradient_paint second;
        apply_background((__bridge void*)view, &second);

        EXPECT_EQ(gradient_layer_count(view), 1); // replaced, not stacked
        EXPECT_TRUE([find_gradient_layer(view).type isEqualToString:kCAGradientLayerRadial]);

        const maui::graphics::solid_paint solid{maui::graphics::colors::red};
        apply_background((__bridge void*)view, &solid);
        EXPECT_EQ(find_gradient_layer(view), nil);      // gradient sublayer removed
        ASSERT_NE(view.layer.backgroundColor, nullptr); // solid color set on the base layer
    }
} // namespace
