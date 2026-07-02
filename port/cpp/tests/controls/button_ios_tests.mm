// iOS (UIKit) backend tests for the button seam — the real-native half of the M6 Rosetta Stone, run
// only for MAUI_BACKEND=ios (executed ON the iOS simulator via tools/ios-sim-run.sh, the preset's
// CMAKE_CROSSCOMPILING_EMULATOR). Drives a genuine UIButton: Text maps to the Normal-state title, and
// the native touch events flow back through the handler's target-action proxy to the control's
// clicked/pressed/released events. Compiled as Objective-C++ with ARC.
//
// NATIVE EVENT INJECTION (send_control_event below): -[UIControl sendActionsForControlEvents:] hands
// each registered (target, action) pair to +[UIApplication sharedApplication] for delivery — and a
// plain spawned simulator process has NO UIApplication (UIKit 26 throws NSInternalInconsistency from
// [[UIApplication alloc] init] outside UIApplicationMain, and UIApplicationMain itself traps without
// an app bundle; both verified on-simulator). The helper therefore replicates the documented dispatch
// walk UIControl performs (allTargets × actionsForTarget:forControlEvent: → invoke), exercising the
// REAL registration the handler made on the REAL UIButton; the only hop skipped is UIApplication's
// final [target performSelector:action] relay. The apple twin keeps NSButton's full -performClick:
// because AppKit's NSApplication is creatable in any process — an AppKit/UIKit platform difference,
// not a port one.
#import <UIKit/UIKit.h>

#include <limits>
#include <memory>
#include <string>

#include "ios_text_ops.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/button_content_layout.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/font.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::button;
    using maui::controls::button_content_layout;
    using maui::controls::image_source;
    using maui::core::button_handler;
    using maui::core::i_element_handler;

    // -[NSString UTF8String] is nullable-annotated; convert through this guard so the std::string
    // construction never receives a null pointer (the values under test are always non-null).
    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    UIButton* native_button(const std::shared_ptr<button_handler>& handler)
    {
        return (__bridge UIButton*)handler->typed_platform_view()->native;
    }

    // Replicates -[UIControl sendActionsForControlEvents:]'s dispatch-table walk for one event (see
    // the header comment): every (target, action) pair registered for `event` is invoked with the
    // control as sender, exactly as UIApplication's sendAction:to:from:forEvent: would (NSInvocation
    // spells the dynamic send without the objc_msgSend function-pointer cast).
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
    // NSTemporaryDirectory(). The tiny-image tests pass 2; the oversize measure test passes 256 to
    // reproduce a settings-icon-sized source.
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
        NSString* const name = [NSString stringWithFormat:@"maui_button_image_test_%@.png", [[NSUUID UUID] UUIDString]];
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

    TEST(ios_button_seam, attaching_handler_creates_uibutton_and_maps_text)
    {
        button control;
        control.set_text("Start");
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_EQ(to_std_string(native_button(handler).currentTitle), "Start");
    }

    TEST(ios_button_seam, setting_text_updates_uibutton_title)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        control.set_text("Changed");
        EXPECT_EQ(to_std_string(native_button(handler).currentTitle), "Changed");
    }

    TEST(ios_button_seam, native_touch_up_inside_flows_back_to_clicked_event)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        int clicks = 0;
        int releases = 0;
        control.clicked.connect([&clicks] { ++clicks; });
        control.released.connect([&releases] { ++releases; });

        send_control_event(native_button(handler), UIControlEventTouchUpInside); // simulate a real tap

        // ButtonEventProxy.OnButtonTouchUpInside: Released, then Clicked.
        EXPECT_EQ(clicks, 1);
        EXPECT_EQ(releases, 1);
    }

    TEST(ios_button_seam, touch_down_presses_and_touch_cancel_releases)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        int pressed = 0;
        int released = 0;
        int clicks = 0;
        control.pressed.connect([&pressed] { ++pressed; });
        control.released.connect([&released] { ++released; });
        control.clicked.connect([&clicks] { ++clicks; });
        UIButton* const view = native_button(handler);

        send_control_event(view, UIControlEventTouchDown); // finger down
        EXPECT_EQ(pressed, 1);
        EXPECT_TRUE(control.is_pressed());

        send_control_event(view, UIControlEventTouchCancel); // gesture cancelled
        EXPECT_EQ(released, 1);
        EXPECT_FALSE(control.is_pressed());
        EXPECT_EQ(clicks, 0); // a cancelled touch never clicks
    }

    TEST(ios_button_seam, touch_up_outside_releases_without_click)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        int released = 0;
        int clicks = 0;
        control.released.connect([&released] { ++released; });
        control.clicked.connect([&clicks] { ++clicks; });

        send_control_event(native_button(handler), UIControlEventTouchUpOutside); // slid off, let go

        EXPECT_EQ(released, 1);
        EXPECT_EQ(clicks, 0);
    }

    TEST(ios_button_seam, disabled_button_click_is_suppressed)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        control.set_is_enabled(false);

        // The disabled state reaches the native control (a real disabled UIButton receives no
        // touches at all)...
        EXPECT_FALSE(native_button(handler).enabled);

        // ...and the control's own IsEnabled gate suppresses Clicked even if an event does arrive.
        int clicks = 0;
        control.clicked.connect([&clicks] { ++clicks; });
        send_control_event(native_button(handler), UIControlEventTouchUpInside);
        EXPECT_EQ(clicks, 0);
    }

    TEST(ios_button_seam, clearing_handler_disconnects)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST(ios_button_seam, appearance_maps_to_uibutton)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        UIButton* const view = native_button(handler);

        control.set_font(maui::core::font::of_size("Helvetica", 18));
        EXPECT_EQ(view.titleLabel.font.pointSize, 18.0);

        control.set_text_color(maui::graphics::color(1.0F, 0.0F, 0.5F));
        UIColor* const title_color = [view titleColorForState:UIControlStateNormal];
        CGFloat red = 0;
        CGFloat green = 0;
        CGFloat blue = 0;
        CGFloat alpha = 0;
        ASSERT_TRUE([title_color getRed:&red green:&green blue:&blue alpha:&alpha]);
        EXPECT_NEAR(red, 1.0, 0.01);
        EXPECT_NEAR(blue, 0.5, 0.01);

        control.set_stroke_thickness(3.0);
        EXPECT_EQ(view.layer.borderWidth, 3.0);

        control.set_corner_radius(7);
        EXPECT_EQ(view.layer.cornerRadius, 7.0);
    }

    // An unset FontSize must resolve to UIFont.SystemFontSize (14pt on iOS), NOT UIFont.ButtonFontSize
    // (18pt): in MAUI, Button.FontSizeDefaultValueCreator() (Button.cs:391) returns this.GetDefaultFontSize()
    // == IFontManager.DefaultFontSize == UIFont.SystemFontSize, so the BindableProperty pre-fills FontSize
    // to 14 before the mapper runs — the ButtonFontSize fallback inside ButtonExtensions.UpdateFont is dead
    // code for a real Button. The port has no default-value-creator, so map_font must supply SystemFontSize
    // as its own default (identical to Label). Regression guard for the "button text renders larger than
    // MAUI" fix (was buttonFontSize == 18pt → ~1.29× too large, 18/14).
    TEST(ios_button_seam, unset_font_size_uses_system_font_size)
    {
        button control;
        control.set_text("Change Formatted String");
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        UIButton* const view = native_button(handler);
        EXPECT_EQ(view.titleLabel.font.pointSize, UIFont.systemFontSize);
        // And specifically NOT the UIButton-native ButtonFontSize (the pre-fix value).
        EXPECT_NE(view.titleLabel.font.pointSize, UIFont.buttonFontSize);
    }

    // MAUI's iOS Button does NOT visibly apply CharacterSpacing (a runtime mapper-order quirk — see
    // refresh_button_title_formatting). Per the 2026-06-23 user ruling the maui-compare RUNTIME is ground
    // truth (it renders "Button", never "B u t t o n"), so the port treats button CharacterSpacing as a
    // visual no-op: NO kerned attributed title is installed; the plain Normal-state title carries the text.
    TEST(ios_button_seam, character_spacing_does_not_kern_the_button_title)
    {
        button control;
        control.set_text("Test");
        control.set_character_spacing(4);
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        UIButton* const native = native_button(handler);
        EXPECT_EQ([native attributedTitleForState:UIControlStateNormal], nil);         // no kerned attributed title
        EXPECT_EQ(to_std_string([native titleForState:UIControlStateNormal]), "Test"); // plain title carries the text
    }

    // With CharacterSpacing a no-op, the text color still applies via UpdateTextColor (setTitleColor), NOT
    // via an attributed title — the title stays plain.
    TEST(ios_button_seam, character_spacing_with_text_color_keeps_plain_title_and_sets_title_color)
    {
        button control;
        control.set_text("Test");
        control.set_character_spacing(4);
        control.set_text_color(maui::graphics::color(1.0F, 0.0F, 0.5F)); // hot-pink-ish
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        UIButton* const native = native_button(handler);
        EXPECT_EQ([native attributedTitleForState:UIControlStateNormal], nil); // still no attributed title
        UIColor* const fg = [native titleColorForState:UIControlStateNormal];
        ASSERT_NE(fg, nil);
        CGFloat red = 0;
        CGFloat green = 0;
        CGFloat blue = 0;
        CGFloat alpha = 0;
        ASSERT_TRUE([fg getRed:&red green:&green blue:&blue alpha:&alpha]);
        EXPECT_NEAR(red, 1.0, 0.01);
        EXPECT_NEAR(blue, 0.5, 0.01);
    }

    // Any CharacterSpacing value (the no-op) leaves the plain, setTitleColor:-colored title in place — no
    // attributed title, whether spacing is non-zero or back at 0.
    TEST(ios_button_seam, character_spacing_changes_keep_the_plain_title)
    {
        button control;
        control.set_text("Test");
        control.set_character_spacing(4);
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        EXPECT_EQ([native_button(handler) attributedTitleForState:UIControlStateNormal], nil);

        control.set_character_spacing(0);
        EXPECT_EQ([native_button(handler) attributedTitleForState:UIControlStateNormal], nil);
        EXPECT_EQ(to_std_string(native_button(handler).currentTitle), "Test");
    }

    // Ports the UpdatePadding recipe: the cross-platform Padding lands on ContentEdgeInsets (top/bottom
    // of exactly 0 become AlmostZero; the border width is folded into every side at map time).
    TEST(ios_button_seam, padding_maps_to_content_edge_insets)
    {
        button control;
        control.set_text("Test");
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        UIButton* const view = native_button(handler);

        // ContentEdgeInsets is deprecated on iOS 15+ but functional (and what C# itself drives);
        // reading it back here mirrors the C# device test under the same suppression.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        // The connect-time map of the default padding. Button.PaddingDefaultValueCreator is
        // new Thickness(double.NaN), so UpdatePadding substitutes DefaultPadding(12,7) — the
        // "Default content insets" a native UIButton has. (A zero default here would collapse the
        // button to bare glyph width: the `clipping` page's crammed digit-row regression.) 7/12 are
        // non-zero, so the AlmostZero floor does not apply.
        const UIEdgeInsets defaults = view.contentEdgeInsets;
        EXPECT_EQ(defaults.top, 7.0);
        EXPECT_EQ(defaults.bottom, 7.0);
        EXPECT_EQ(defaults.left, 12.0);
        EXPECT_EQ(defaults.right, 12.0);

        control.set_padding(maui::core::thickness(5, 10, 15, 20)); // left, top, right, bottom
        const UIEdgeInsets insets = view.contentEdgeInsets;
        EXPECT_EQ(insets.top, 10.0);
        EXPECT_EQ(insets.left, 5.0);
        EXPECT_EQ(insets.bottom, 20.0);
        EXPECT_EQ(insets.right, 15.0);

        // The border width is added to every side when padding is (re)mapped, int-truncated as in C#.
        // (The padding value must actually CHANGE — an equal set is a property-system no-op, exactly
        // like C#'s SetValue, so it would not re-run the mapper.)
        control.set_stroke_thickness(2.0);
        control.set_padding(maui::core::thickness(6, 11, 16, 21));
        const UIEdgeInsets bordered = view.contentEdgeInsets;
        EXPECT_EQ(bordered.top, 13.0);
        EXPECT_EQ(bordered.left, 8.0);
        EXPECT_EQ(bordered.bottom, 23.0);
        EXPECT_EQ(bordered.right, 18.0);
#pragma clang diagnostic pop
    }

    // The generic-IView pushes (the shared view_mapper through button_platform's ios update_*
    // overrides): visibility / opacity / automation_id reach the real UIButton.
    TEST(ios_button_seam, generic_iview_properties_reach_the_uibutton)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        UIButton* const view = native_button(handler);

        control.set_visibility(maui::core::visibility::hidden);
        EXPECT_TRUE(view.hidden);
        control.set_visibility(maui::core::visibility::visible);
        EXPECT_FALSE(view.hidden);

        control.set_opacity(0.5);
        EXPECT_EQ(view.alpha, 0.5);

        control.set_automation_id("submit_button");
        EXPECT_EQ(to_std_string(view.accessibilityIdentifier), "submit_button");
    }

    TEST(ios_button_seam, handler_resolved_from_default_registry)
    {
        // button -> button_handler is self-registered in button.cpp (MAUI_REGISTER_HANDLER).
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<button>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<button_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        button control;
        control.set_text("Registered");
        control.set_handler(handler);
        auto const button_view = (__bridge UIButton*)resolved->typed_platform_view()->native;
        EXPECT_EQ(to_std_string(button_view.currentTitle), "Registered");
    }

    // ---- the image surface (ButtonHandler.MapImageSource → SetImage(Normal) + LayoutIfNeeded) ----

    TEST(ios_button_seam, file_image_source_loads_into_the_normal_state_image)
    {
        const std::string path = write_temp_png();
        ASSERT_FALSE(path.empty());

        button control;
        control.set_image_source(image_source::from_file(path));
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        UIButton* const view = native_button(handler);
        // ButtonImageSourcePartSetter.SetImageSource: SetImage(..., Normal).
        EXPECT_NE([view imageForState:UIControlStateNormal], nil);
        remove_file(path);
    }

    TEST(ios_button_seam, set_image_applies_always_original_rendering_mode)
    {
        const std::string path = write_temp_png();
        ASSERT_FALSE(path.empty());

        button control;
        control.set_image_source(image_source::from_file(path));
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        UIImage* const set = [native_button(handler) imageForState:UIControlStateNormal];
        ASSERT_NE(set, nil);
        // ImageWithRenderingMode(AlwaysOriginal): the iOS-only step the AppKit twin omits.
        EXPECT_EQ(set.renderingMode, UIImageRenderingModeAlwaysOriginal);
        remove_file(path);
    }

    TEST(ios_button_seam, set_image_forces_layout_so_size_that_fits_grows)
    {
        // LayoutIfNeeded after SetImage is required: UIButton applies the image only at render, so
        // SizeThatFits would not reflect it without the forced layout (ButtonHandler.iOS.cs:225-229).
        const std::string path = write_temp_png();
        ASSERT_FALSE(path.empty());

        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        UIButton* const view = native_button(handler);
        const CGSize before = [view sizeThatFits:CGSizeMake(CGFLOAT_MAX, CGFLOAT_MAX)];

        control.set_image_source(image_source::from_file(path));
        const CGSize after = [view sizeThatFits:CGSizeMake(CGFLOAT_MAX, CGFLOAT_MAX)];
        // The 2x2 image adds to the empty button's fitted size once the forced layout takes effect.
        EXPECT_GT(after.width, before.width);
        remove_file(path);
    }

    // Regression (the reported bug): a Button with a LARGE image whose natural width exceeds the WIDTH
    // constraint — the exact StackLayout case: a VerticalStackLayout measures children with a finite width
    // and INFINITE height (see vertical_stack_layout_manager). get_desired_size ports Button.iOS.cs
    // CrossPlatformMeasure, which resizes the image DOWN by the aspect-preserving factor = min(availW/imgW,
    // availH/imgH); a narrow width therefore tames the HEIGHT too, so the reported height follows the
    // shrunk image, NOT the raw pixels. Before the fix a raw -[UIButton sizeThatFits:] returned the full
    // ~256px height, which content_is_minimum_size()==true floored — shoving the following controls
    // off-screen.
    TEST(ios_button_seam, large_image_button_width_constrained_tames_the_height)
    {
        const std::string path = write_temp_png_sized(256); // a square icon far larger than a button
        ASSERT_FALSE(path.empty());

        button control;
        control.set_image_source(image_source::from_file(path));
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        // A 100pt-wide stack cell with infinite height (the real StackLayout measure). The 256px image is
        // wider than the ~76pt available content width, so it aspect-shrinks — and the square image's height
        // shrinks with it, well under the raw 256px source.
        const maui::graphics::size measured = handler->get_desired_size(100.0, std::numeric_limits<double>::infinity());
        EXPECT_LT(measured.height, 130.0);      // tamed, NOT ~256+px (the bug)
        EXPECT_LE(measured.width, 100.0 + 0.5); // clamped to the width constraint (+ceil tolerance)
        EXPECT_GT(measured.height, 0.0);
        remove_file(path);
    }

    // A big image with a bounded HEIGHT constraint narrower than the image also tames it (the height axis of
    // the same min-factor resize). Guards the height-driven branch of CrossPlatformMeasure.
    TEST(ios_button_seam, large_image_button_height_constrained_tames_the_image)
    {
        const std::string path = write_temp_png_sized(256);
        ASSERT_FALSE(path.empty());

        button control;
        control.set_image_source(image_source::from_file(path));
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        // Plenty of width, but a 60pt height budget: the image must shrink to fit the height.
        const maui::graphics::size measured = handler->get_desired_size(400.0, 60.0);
        EXPECT_LE(measured.height, 60.0 + 0.5); // clamped to the height constraint
        EXPECT_GT(measured.height, 0.0);
        remove_file(path);
    }

    // A big image with NO width/height constraint (infinite) keeps roughly its natural size + insets —
    // matching MAUI when there's infinite space (the resize is a no-op on an unconstrained axis). This
    // guards against over-shrinking an unconstrained image button.
    TEST(ios_button_seam, large_image_button_unconstrained_keeps_natural_image_size)
    {
        const std::string path = write_temp_png_sized(64);
        ASSERT_FALSE(path.empty());

        button control;
        control.set_image_source(image_source::from_file(path));
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        const maui::graphics::size measured =
            handler->get_desired_size(std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
        // 64px image + default padding (7 top/bottom) → about 78pt tall, at least the image height.
        EXPECT_GE(measured.height, 64.0);
        EXPECT_LT(measured.height, 200.0);
        remove_file(path);
    }

    // The gallery's settings buttons use ContentLayout Top (image stacked ABOVE the title). get_desired_size
    // reads i_text_button::content_layout_spec() and composes image + title on the HEIGHT axis for Top/Bottom
    // (C#'s layout.Position branch), so a Top button is TALLER than the same button with the image beside the
    // title (Left) — image_h + title_h + spacing vs max(image_h, title_h). This is the exact case the report
    // bug hit (a settings-sized icon above "settings"): it must still be a sane, bounded height, taller than
    // the Left composition but nowhere near the raw stacked pixel blow-up the old raw-sizeThatFits produced.
    TEST(ios_button_seam, top_content_layout_stacks_image_and_title_on_the_height_axis)
    {
        const std::string path = write_temp_png_sized(48); // a modest icon that fits the constraint unshrunk
        ASSERT_FALSE(path.empty());

        button top_control;
        top_control.set_text("settings");
        top_control.set_image_source(image_source::from_file(path));
        top_control.set_content_layout(button_content_layout{button_content_layout::image_position::top, 10.0});
        auto top_handler = std::make_shared<button_handler>();
        top_control.set_handler(top_handler);

        button left_control;
        left_control.set_text("settings");
        left_control.set_image_source(image_source::from_file(path));
        left_control.set_content_layout(button_content_layout{button_content_layout::image_position::left, 10.0});
        auto left_handler = std::make_shared<button_handler>();
        left_control.set_handler(left_handler);

        const maui::graphics::size top = top_handler->get_desired_size(320.0, 600.0);
        const maui::graphics::size left = left_handler->get_desired_size(320.0, 600.0);

        // Top stacks (image_h + title_h + spacing); Left is max(image_h, title_h). So Top is strictly taller.
        EXPECT_GT(top.height, left.height);
        // Still a sane, bounded height — the 48pt image + a line of text + spacing + insets, well under the
        // runaway height the raw sizeThatFits floor produced for the reported bug.
        EXPECT_LT(top.height, 160.0);
        EXPECT_GE(top.height, 48.0); // at least the (unshrunk) image height
        remove_file(path);
    }

    // Text-only buttons keep the native SizeThatFits path (the fix does NOT change them): a plain titled
    // button still measures to its intrinsic title + insets, unaffected by the image branch.
    TEST(ios_button_seam, text_only_button_measure_is_unchanged)
    {
        button control;
        control.set_text("Hello");
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        const maui::graphics::size measured = handler->get_desired_size(CGFLOAT_MAX, CGFLOAT_MAX);
        UIButton* const view = native_button(handler);
        const CGSize native = [view sizeThatFits:CGSizeMake(CGFLOAT_MAX, CGFLOAT_MAX)];
        EXPECT_NEAR(measured.width, native.width, 0.5);
        EXPECT_NEAR(measured.height, native.height, 0.5);
    }

    TEST(ios_button_seam, clearing_image_source_removes_the_normal_state_image)
    {
        const std::string path = write_temp_png();
        ASSERT_FALSE(path.empty());

        button control;
        control.set_image_source(image_source::from_file(path));
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        UIButton* const view = native_button(handler);
        ASSERT_NE([view imageForState:UIControlStateNormal], nil);

        control.set_image_source(nullptr);
        EXPECT_EQ([view imageForState:UIControlStateNormal], nil);
        remove_file(path);
    }

    TEST(ios_button_seam, content_layout_is_stored_and_pushes_without_crashing)
    {
        // ContentLayout is stored + pushed (the text+image composition is deferred — no container infra).
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        control.set_content_layout(button_content_layout{button_content_layout::image_position::right, 6.0});
        EXPECT_EQ(control.content_layout().position, button_content_layout::image_position::right);
        EXPECT_EQ(control.content_layout().spacing, 6.0);
    }
} // namespace
