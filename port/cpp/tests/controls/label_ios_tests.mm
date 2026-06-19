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
#include "maui/core/line_break_mode.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/text_decorations.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"
#include <gtest/gtest.h>

// The shape of the handler's MauiIosLabel (declared in src/platform/ios/label_handler.mm): the test
// reads the custom verticalAlignment + textInsets back through this protocol after a respondsToSelector:
// probe (the AppKit twin reads its custom cell the same way).
@protocol MauiTestLabelVerticalAlignment <NSObject>
@property(nonatomic) UIControlContentVerticalAlignment verticalAlignment;
@property(nonatomic) UIEdgeInsets textInsets;
@end

namespace
{
    using maui::controls::label;
    using maui::core::label_handler;
    using maui::core::line_break_mode;
    using maui::core::text_alignment;
    using maui::core::text_decorations;
    using maui::platform::ios::kerning_of;
    using maui::platform::ios::line_height_multiple_of;

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

    // MapBackground: a SolidPaint background paints the UILabel's backing layer (the AbsoluteLayout
    // "AutoSized" demo is white text on a blue box). Before update_background was wired on iOS the paint
    // fell through to the no-op view_platform_base mirror and the label rendered transparent — invisible on
    // a light page. The CGColor round-trips back through UIColor to read its components.
    TEST(ios_label_seam, solid_background_paints_the_backing_layer)
    {
        label control;
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        UILabel* const view = native_label(handler);

        control.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::color(0.0F, 0.0F, 1.0F)));
        ASSERT_NE(view.layer.backgroundColor, nullptr);
        UIColor* const bg = [UIColor colorWithCGColor:view.layer.backgroundColor];
        CGFloat red = 0;
        CGFloat green = 0;
        CGFloat blue = 0;
        CGFloat alpha = 0;
        ASSERT_TRUE([bg getRed:&red green:&green blue:&blue alpha:&alpha]);
        EXPECT_NEAR(red, 0.0, 0.01);
        EXPECT_NEAR(blue, 1.0, 0.01);
        EXPECT_NEAR(alpha, 1.0, 0.01);
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

    // LabelExtensions.UpdateLineHeight: the line-height multiple lands on the attributed text's paragraph
    // style (default -1 leaves none); kerning/decorations co-exist (MapFormatting order).
    TEST(ios_label_seam, line_height_applies_a_paragraph_style_multiple)
    {
        label control;
        control.set_text("Test");
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        UILabel* const view = native_label(handler);

        // Default -1: no positive multiple (the bail-out branch leaves the attributed text un-styled here).
        EXPECT_LE(line_height_multiple_of(view.attributedText), 0.0);

        control.set_line_height(2.0);
        EXPECT_DOUBLE_EQ(line_height_multiple_of(view.attributedText), 2.0);

        control.set_character_spacing(3.0);
        EXPECT_DOUBLE_EQ(line_height_multiple_of(view.attributedText), 2.0);
        EXPECT_EQ(kerning_of(view.attributedText), 3.0);
        EXPECT_EQ(to_std_string(view.text), "Test");
    }

    // LabelExtensions.UpdatePadding → MauiLabel.TextInsets (Top, Left, Bottom, Right) on the custom label.
    TEST(ios_label_seam, padding_reaches_the_text_insets)
    {
        label control;
        control.set_text("Test");
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        UILabel* const view = native_label(handler);
        ASSERT_TRUE([view respondsToSelector:@selector(textInsets)]);
        id const any_label = view;
        id<MauiTestLabelVerticalAlignment> const aligned = any_label;

        control.set_padding(maui::core::thickness(4, 8, 12, 16)); // left, top, right, bottom
        EXPECT_DOUBLE_EQ(aligned.textInsets.left, 4.0);
        EXPECT_DOUBLE_EQ(aligned.textInsets.top, 8.0);
        EXPECT_DOUBLE_EQ(aligned.textInsets.right, 12.0);
        EXPECT_DOUBLE_EQ(aligned.textInsets.bottom, 16.0);
    }

    // MauiLabel.SizeThatFits reflects the insets: get_desired_size grows by (left+right, top+bottom).
    TEST(ios_label_seam, padding_inflates_desired_size)
    {
        label control;
        control.set_text("Padded");
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);

        const maui::graphics::size bare = handler->get_desired_size(1.0e9, 1.0e9);
        control.set_padding(maui::core::thickness(5, 6, 7, 8));
        const maui::graphics::size padded = handler->get_desired_size(1.0e9, 1.0e9);
        EXPECT_NEAR(padded.width, bare.width + 12.0, 0.5);   // 5 + 7
        EXPECT_NEAR(padded.height, bare.height + 14.0, 0.5); // 6 + 8
    }

    // PreferredMaxLayoutWidth branch (LabelHandler.iOS.GetDesiredSize): an explicit virtual Width wraps the
    // long text onto more than one line, so the measured height exceeds the single-line measure and the
    // width stays within the explicit cap.
    TEST(ios_label_seam, explicit_width_wraps_to_multiple_lines)
    {
        label control;
        control.set_text("The quick brown fox jumps over the lazy dog repeatedly");
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);

        const maui::graphics::size single = handler->get_desired_size(1.0e9, 1.0e9);

        control.set_width_request(80);
        const maui::graphics::size wrapped = handler->get_desired_size(1.0e9, 1.0e9);
        EXPECT_GT(wrapped.height, single.height);
        EXPECT_LE(wrapped.width, 80.0 + 1.0); // within the PreferredMaxLayoutWidth cap (allow rounding)
    }

    // LabelHandler.MapLineBreakMode → SetLineBreakMode's LineBreakMode→UILineBreakMode mapping reaches the
    // real UILabel.lineBreakMode (NoWrap→Clip, WordWrap→WordWrapping, *Truncation→TruncatingHead/Middle/Tail).
    TEST(ios_label_seam, maps_line_break_mode_to_uilabel)
    {
        label control;
        control.set_text("Test");
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        UILabel* const view = native_label(handler);

        // Default WordWrap mapped at connect.
        EXPECT_EQ(view.lineBreakMode, NSLineBreakByWordWrapping);

        control.set_line_break_mode(line_break_mode::no_wrap);
        EXPECT_EQ(view.lineBreakMode, NSLineBreakByClipping);

        control.set_line_break_mode(line_break_mode::character_wrap);
        EXPECT_EQ(view.lineBreakMode, NSLineBreakByCharWrapping);

        control.set_line_break_mode(line_break_mode::head_truncation);
        EXPECT_EQ(view.lineBreakMode, NSLineBreakByTruncatingHead);

        control.set_line_break_mode(line_break_mode::middle_truncation);
        EXPECT_EQ(view.lineBreakMode, NSLineBreakByTruncatingMiddle);

        control.set_line_break_mode(line_break_mode::tail_truncation);
        EXPECT_EQ(view.lineBreakMode, NSLineBreakByTruncatingTail);
    }

    // SetLineBreakMode's numberOfLines half (UILabel.Lines) — MaxLines<0 resolves to 0 for the wrapping
    // modes and 1 for TailTruncation; NoWrap/Head/MiddleTruncation force 1; an explicit MaxLines wins
    // except where a single-line mode overrides it.
    TEST(ios_label_seam, maps_max_lines_to_uilabel)
    {
        label control;
        control.set_text("Test");
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        UILabel* const view = native_label(handler);

        // Default WordWrap + MaxLines -1 → 0 ("as many lines as needed").
        EXPECT_EQ(view.numberOfLines, 0);

        // TailTruncation with MaxLines unset (-1) → 1 (the only mode the pre-switch branch clamps to 1).
        control.set_line_break_mode(line_break_mode::tail_truncation);
        EXPECT_EQ(view.numberOfLines, 1);

        // An explicit MaxLines under a wrapping mode wins.
        control.set_line_break_mode(line_break_mode::word_wrap);
        control.set_max_lines(3);
        EXPECT_EQ(view.numberOfLines, 3);

        // A single-line truncation mode (Head) forces 1 even when MaxLines is 3.
        control.set_line_break_mode(line_break_mode::head_truncation);
        EXPECT_EQ(view.numberOfLines, 1);

        // MiddleTruncation also forces 1.
        control.set_line_break_mode(line_break_mode::middle_truncation);
        EXPECT_EQ(view.numberOfLines, 1);

        // NoWrap forces 1 (the wrap mode is Clip + a single line).
        control.set_line_break_mode(line_break_mode::no_wrap);
        EXPECT_EQ(view.numberOfLines, 1);
        EXPECT_EQ(view.lineBreakMode, NSLineBreakByClipping);

        // Back to WordWrap with the explicit MaxLines=3 still set → 3 (no single-line clamp).
        control.set_line_break_mode(line_break_mode::word_wrap);
        EXPECT_EQ(view.numberOfLines, 3);
    }
} // namespace
