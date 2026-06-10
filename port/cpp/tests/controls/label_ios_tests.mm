// iOS (UIKit) backend tests for the label seam — properties pushed to a real UILabel (the MauiIosLabel
// subclass), run only for MAUI_BACKEND=ios (executed ON the iOS simulator via tools/ios-sim-run.sh).
// Mirrors the AppKit twin's coverage (label_apple_tests.mm) plus the UIKit-real pieces: text
// decorations on the attributed text and the generic-IView pushes (standing in for view_mapper
// coverage until a view_mapper_ios_tests.mm arrives). Compiled as Objective-C++ with ARC.
#import <UIKit/UIKit.h>

#include <memory>
#include <string>

#include "ios_text_ops.hpp"
#include "maui/controls/label.hpp"
#include "maui/core/font.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/text_decorations.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

// The shape of the handler's MauiIosLabel (declared in src/platform/ios/label_handler.mm): the test
// reads the custom verticalAlignment back through this protocol after a respondsToSelector: probe (the
// AppKit twin reads its custom cell the same way).
@protocol MauiTestLabelVerticalAlignment <NSObject>
@property(nonatomic) UIControlContentVerticalAlignment verticalAlignment;
@end

namespace
{
    using maui::controls::label;
    using maui::core::label_handler;
    using maui::core::text_alignment;
    using maui::core::text_decorations;
    using maui::platform::ios::kerning_of;

    // -[NSString UTF8String] is nullable-annotated; convert through this guard so the std::string
    // construction never receives a null pointer (the values under test are always non-null).
    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    UILabel* native_label(const std::shared_ptr<label_handler>& handler)
    {
        return (__bridge UILabel*)handler->typed_platform_view()->native;
    }

    // Reads a style attribute (underline/strikethrough) off the first character; 0 when absent.
    NSInteger style_attribute_of(NSAttributedString* attributed, NSAttributedStringKey key)
    {
        if (attributed == nil || attributed.length == 0)
        {
            return 0;
        }
        NSNumber* const value = [attributed attribute:key atIndex:0 effectiveRange:nullptr];
        return value != nil ? value.integerValue : 0;
    }

    TEST(ios_label_seam, maps_text_to_uilabel)
    {
        label control;
        control.set_text("Start");
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_EQ(to_std_string(native_label(handler).text), "Start");
    }

    TEST(ios_label_seam, maps_font_color_and_alignment)
    {
        label control;
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        UILabel* const view = native_label(handler);

        control.set_font(maui::core::font::of_size("Helvetica", 18));
        EXPECT_EQ(view.font.pointSize, 18.0);

        control.set_text_color(maui::graphics::color(1.0F, 0.0F, 0.5F));
        CGFloat red = 0;
        CGFloat green = 0;
        CGFloat blue = 0;
        CGFloat alpha = 0;
        ASSERT_TRUE([view.textColor getRed:&red green:&green blue:&blue alpha:&alpha]);
        EXPECT_NEAR(red, 1.0, 0.01);
        EXPECT_NEAR(blue, 0.5, 0.01);

        control.set_horizontal_text_alignment(text_alignment::center);
        EXPECT_EQ(view.textAlignment, NSTextAlignmentCenter);
    }

    // Ports LabelHandlerTests.iOS CharacterSpacingInitializesCorrectly: the kerning on the attributed
    // text equals the cross-platform CharacterSpacing, and the visible text is unchanged.
    TEST(ios_label_seam, character_spacing_kerns_the_attributed_text)
    {
        label control;
        control.set_text("Test");
        control.set_character_spacing(4);
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);

        UILabel* const view = native_label(handler);
        EXPECT_EQ(kerning_of(view.attributedText), 4.0);
        EXPECT_EQ(to_std_string(view.text), "Test");
    }

    // Alignment set first, then character spacing must not drop it (MapFormatting re-asserts the
    // horizontal alignment after the attributed passes).
    TEST(ios_label_seam, character_spacing_preserves_horizontal_alignment)
    {
        label control;
        control.set_text("Test");
        control.set_horizontal_text_alignment(text_alignment::end);
        control.set_character_spacing(4);
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);

        UILabel* const view = native_label(handler);
        EXPECT_EQ(kerning_of(view.attributedText), 4.0);
        EXPECT_EQ(view.textAlignment, NSTextAlignmentRight);
    }

    // Ports LabelHandlerTests.iOS TextDecorations device coverage: the underline/strikethrough single
    // styles land on (and clear from) the attributed text.
    TEST(ios_label_seam, text_decorations_set_and_clear_the_styles)
    {
        label control;
        control.set_text("Decorated");
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        UILabel* const view = native_label(handler);

        control.set_text_decorations(text_decorations::underline);
        EXPECT_EQ(style_attribute_of(view.attributedText, NSUnderlineStyleAttributeName), NSUnderlineStyleSingle);
        EXPECT_EQ(style_attribute_of(view.attributedText, NSStrikethroughStyleAttributeName), 0);

        control.set_text_decorations(text_decorations::strikethrough);
        EXPECT_EQ(style_attribute_of(view.attributedText, NSUnderlineStyleAttributeName), 0);
        EXPECT_EQ(style_attribute_of(view.attributedText, NSStrikethroughStyleAttributeName), NSUnderlineStyleSingle);

        control.set_text_decorations(text_decorations::none);
        EXPECT_EQ(style_attribute_of(view.attributedText, NSUnderlineStyleAttributeName), 0);
        EXPECT_EQ(style_attribute_of(view.attributedText, NSStrikethroughStyleAttributeName), 0);
    }

    // MapText → MapFormatting: replacing the text re-applies the decorations + kerning onto the fresh
    // attributed value ("Any text update requires that we update any attributed string formatting").
    TEST(ios_label_seam, text_change_reapplies_decorations_and_kerning)
    {
        label control;
        control.set_text("First");
        control.set_text_decorations(text_decorations::underline);
        control.set_character_spacing(3);
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);

        control.set_text("Second");
        UILabel* const view = native_label(handler);
        EXPECT_EQ(to_std_string(view.text), "Second");
        EXPECT_EQ(style_attribute_of(view.attributedText, NSUnderlineStyleAttributeName), NSUnderlineStyleSingle);
        EXPECT_EQ(kerning_of(view.attributedText), 3.0);
    }

    // vertical_text_alignment reaches the custom MauiIosLabel (UILabel has no vertical alignment of its
    // own; the subclass offsets the draw rect — Start→Top / Center→Center / End→Bottom).
    TEST(ios_label_seam, vertical_text_alignment_is_stored_on_the_label)
    {
        label control;
        control.set_text("Test");
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        UILabel* const view = native_label(handler);
        ASSERT_TRUE([view respondsToSelector:@selector(verticalAlignment)]);
        id const any_label = view; // ObjC allows id -> id<Protocol> implicitly (no cast machinery needed)
        id<MauiTestLabelVerticalAlignment> const aligned = any_label;

        // The control's default (Start) mapped at connect → Top.
        EXPECT_EQ(aligned.verticalAlignment, UIControlContentVerticalAlignmentTop);

        control.set_vertical_text_alignment(text_alignment::end);
        EXPECT_EQ(aligned.verticalAlignment, UIControlContentVerticalAlignmentBottom);

        control.set_vertical_text_alignment(text_alignment::center);
        EXPECT_EQ(aligned.verticalAlignment, UIControlContentVerticalAlignmentCenter);
    }

    // The generic-IView pushes (the shared view_mapper through label_platform's ios update_*
    // overrides): visibility / opacity / is_enabled / automation_id reach the real UILabel.
    TEST(ios_label_seam, generic_iview_properties_reach_the_uilabel)
    {
        label control;
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        UILabel* const view = native_label(handler);

        control.set_visibility(maui::core::visibility::hidden);
        EXPECT_TRUE(view.hidden);
        control.set_visibility(maui::core::visibility::visible);
        EXPECT_FALSE(view.hidden);

        control.set_opacity(0.5);
        EXPECT_EQ(view.alpha, 0.5);

        // UpdateIsEnabled's non-UIControl branch: the interaction toggle.
        control.set_is_enabled(false);
        EXPECT_FALSE(view.userInteractionEnabled);
        control.set_is_enabled(true);
        EXPECT_TRUE(view.userInteractionEnabled);

        control.set_automation_id("title_label");
        EXPECT_EQ(to_std_string(view.accessibilityIdentifier), "title_label");
    }
} // namespace
