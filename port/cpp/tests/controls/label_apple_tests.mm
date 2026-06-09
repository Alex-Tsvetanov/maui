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
#include <gtest/gtest.h>

namespace
{
    using maui::controls::label;
    using maui::core::label_handler;
    using maui::core::text_alignment;
    using maui::platform::apple::kerning_of;

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
} // namespace
