// Apple (AppKit) backend tests for the label seam — properties pushed to a real NSTextField (label
// style). Compiled as Objective-C++ with ARC for the `apple` backend.
#import <AppKit/AppKit.h>

#include <memory>
#include <string>

#include "apple_text_ops.hpp"
#include "maui/controls/label.hpp"
#include "maui/core/font.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/text_decorations.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/size.hpp"
#include <gtest/gtest.h>

// The shape of the handler's MauiLabelCell (declared file-local in src/platform/apple/label_handler.mm):
// the test reads the Padding insets back through this protocol after a respondsToSelector: probe (the
// vertical-alignment property is reached the same way).
@protocol MauiTestLabelCell <NSObject>
@property(nonatomic) maui::core::thickness textInsets;
@end

namespace
{
    using maui::controls::label;
    using maui::core::label_handler;
    using maui::core::text_alignment;
    using maui::platform::apple::kerning_of;
    using maui::platform::apple::line_height_multiple_of;

    // -[NSString UTF8String] is nullable-annotated; convert through this guard so the std::string
    // construction never receives a null pointer (the values under test are always non-null).
    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    NSTextField* native_label(const std::shared_ptr<label_handler>& handler)
    {
        return (__bridge NSTextField*)handler->typed_platform_view()->native;
    }

    class apple_label_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_label_seam, maps_text_to_nstextfield)
    {
        label control;
        control.set_text("Start");
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_EQ(to_std_string(native_label(handler).stringValue), "Start");
    }

    TEST_F(apple_label_seam, maps_font_and_alignment)
    {
        label control;
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);

        control.set_font(maui::core::font::of_size("Helvetica", 18));
        EXPECT_EQ(native_label(handler).font.pointSize, 18.0);

        control.set_horizontal_text_alignment(text_alignment::center);
        EXPECT_EQ(native_label(handler).alignment, NSTextAlignmentCenter);
    }

    // Ports LabelHandlerTests CharacterSpacing: the kerning on the attributed string equals the
    // cross-platform CharacterSpacing, and the visible text is unchanged.
    TEST_F(apple_label_seam, character_spacing_kerns_the_attributed_string)
    {
        label control;
        control.set_text("Test");
        control.set_character_spacing(4);
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);

        NSTextField* const view = native_label(handler);
        EXPECT_EQ(kerning_of(view.attributedStringValue), 4.0);
        EXPECT_EQ(to_std_string(view.stringValue), "Test");
    }

    // Setting LineHeight-style alignment survives: alignment set first, then character spacing must not
    // drop it (mirrors LabelHandlerTests CanSetAlignmentAndLineHeight's alignment-survives expectation).
    TEST_F(apple_label_seam, character_spacing_preserves_horizontal_alignment)
    {
        label control;
        control.set_text("Test");
        control.set_horizontal_text_alignment(text_alignment::end);
        control.set_character_spacing(4);
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);

        NSTextField* const view = native_label(handler);
        EXPECT_EQ(kerning_of(view.attributedStringValue), 4.0);
        EXPECT_EQ(view.alignment, NSTextAlignmentRight);
    }

    // vertical_text_alignment reaches the custom label cell (NSTextField has no vertical alignment of its
    // own; the custom cell offsets the text rect — Start/Center/End).
    TEST_F(apple_label_seam, vertical_text_alignment_is_stored_on_the_cell)
    {
        label control;
        control.set_text("Test");
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        NSTextField* const view = native_label(handler);
        ASSERT_TRUE([view.cell respondsToSelector:@selector(verticalAlignment)]);

        control.set_vertical_text_alignment(text_alignment::end);
        EXPECT_EQ((int)[(id)view.cell verticalAlignment], (int)text_alignment::end);
    }

    // text_decorations land on (and clear from) the attributed string — the AppKit face of
    // LabelExtensions.UpdateTextDecorations (M6 fan-out: ported alongside the UIKit twin).
    TEST_F(apple_label_seam, text_decorations_set_and_clear_the_styles)
    {
        label control;
        control.set_text("Decorated");
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        NSTextField* const view = native_label(handler);

        const auto style_attribute = [](NSAttributedString* attributed, NSAttributedStringKey key) -> NSInteger {
            if (attributed == nil || attributed.length == 0)
            {
                return 0;
            }
            NSNumber* const value = [attributed attribute:key atIndex:0 effectiveRange:nullptr];
            return value != nil ? value.integerValue : 0;
        };

        control.set_text_decorations(maui::core::text_decorations::underline);
        EXPECT_EQ(style_attribute(view.attributedStringValue, NSUnderlineStyleAttributeName), NSUnderlineStyleSingle);
        EXPECT_EQ(style_attribute(view.attributedStringValue, NSStrikethroughStyleAttributeName), 0);

        control.set_text_decorations(maui::core::text_decorations::none);
        EXPECT_EQ(style_attribute(view.attributedStringValue, NSUnderlineStyleAttributeName), 0);
    }

    // LabelExtensions.UpdateLineHeight: the line-height multiple lands on the attributed string's
    // paragraph style; the default (-1) leaves the un-multiplied value.
    TEST_F(apple_label_seam, line_height_applies_a_paragraph_style_multiple)
    {
        label control;
        control.set_text("Test");
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        NSTextField* const view = native_label(handler);

        // Default -1: no positive multiple applied (the bail-out branch leaves nothing to read).
        EXPECT_LE(line_height_multiple_of(view.attributedStringValue), 0.0);

        control.set_line_height(2.0);
        EXPECT_DOUBLE_EQ(line_height_multiple_of(view.attributedStringValue), 2.0);

        // The visible text is unchanged and kerning/decorations still co-exist (MapFormatting order).
        control.set_character_spacing(3.0);
        EXPECT_DOUBLE_EQ(line_height_multiple_of(view.attributedStringValue), 2.0);
        EXPECT_EQ(kerning_of(view.attributedStringValue), 3.0);
        EXPECT_EQ(to_std_string(view.stringValue), "Test");
    }

    // LabelExtensions.UpdatePadding(MauiLabel.TextInsets): the Padding reaches the custom cell (which
    // flips left/right under RTL live at draw time, reading the control view's layout direction).
    TEST_F(apple_label_seam, padding_reaches_the_cell)
    {
        label control;
        control.set_text("Test");
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        NSTextField* const view = native_label(handler);
        ASSERT_TRUE([view.cell respondsToSelector:@selector(textInsets)]);
        id const any_cell = view.cell;
        id<MauiTestLabelCell> const cell = any_cell;

        control.set_padding(maui::core::thickness(4, 8, 12, 16));
        EXPECT_DOUBLE_EQ(cell.textInsets.left, 4.0);
        EXPECT_DOUBLE_EQ(cell.textInsets.top, 8.0);
        EXPECT_DOUBLE_EQ(cell.textInsets.right, 12.0);
        EXPECT_DOUBLE_EQ(cell.textInsets.bottom, 16.0);
    }

    // Padding inflates the desired size: the cell inset shrinks the drawing rect, so get_desired_size adds
    // the insets back (MauiLabel.SizeThatFits' AddInsets).
    TEST_F(apple_label_seam, padding_inflates_desired_size)
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

    // PreferredMaxLayoutWidth branch: an explicit virtual Width wraps the long text onto more than one
    // line, so the measured height exceeds a single-line measure and the width collapses near the cap
    // (NSTextField's fittingSize may slightly exceed preferredMaxLayoutWidth for an unbreakable token, so
    // the proof is the dramatic narrowing from the single-line width, not an exact ceiling).
    TEST_F(apple_label_seam, explicit_width_wraps_to_multiple_lines)
    {
        label control;
        control.set_text("The quick brown fox jumps over the lazy dog repeatedly");
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);

        const maui::graphics::size single = handler->get_desired_size(1.0e9, 1.0e9);

        control.set_width_request(80); // a narrow explicit Width forces wrapping
        const maui::graphics::size wrapped = handler->get_desired_size(1.0e9, 1.0e9);
        EXPECT_GT(wrapped.height, single.height);     // grew past one line
        EXPECT_LT(wrapped.width, single.width / 2.0); // collapsed near the 80pt cap, far below single-line
    }
} // namespace
