// Apple (AppKit) backend tests for label.formatted_text (gap-closure G1) — the resolved per-span runs
// build a real NSAttributedString on the NSTextField (font / color / background / underline / kerning),
// and clearing reverts to the plain string. Compiled as Objective-C++ with ARC for the `apple` backend.
#import <AppKit/AppKit.h>

#include <memory>
#include <string>

#include "maui/controls/formatted_string.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/span.hpp"
#include "maui/core/font.hpp"
#include "maui/core/font_attributes.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/text_decorations.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::formatted_string;
    using maui::controls::label;
    using maui::controls::span;
    using maui::core::font;
    using maui::core::font_attributes;
    using maui::core::label_handler;
    using maui::core::text_decorations;
    using maui::graphics::color;

    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    NSTextField* native_label(const std::shared_ptr<label_handler>& handler)
    {
        return (__bridge NSTextField*)handler->typed_platform_view()->native;
    }

    // Read an attribute off the attributed string at a character index.
    id attribute_at(NSAttributedString* attributed, NSAttributedStringKey key, NSUInteger index)
    {
        return [attributed attribute:key atIndex:index effectiveRange:nullptr];
    }

    class apple_formatted_text : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_formatted_text, builds_attributed_string_with_per_span_attributes)
    {
        label control;
        control.set_font(font::of_size("Helvetica", 14));

        auto fs = std::make_shared<formatted_string>();
        auto red = std::make_shared<span>();
        red->set_text("AA");
        red->set_text_color(color::from_rgb(1.0F, 0.0F, 0.0F));
        red->set_background_color(color::from_rgb(0.0F, 0.0F, 1.0F));
        red->set_character_spacing(5);
        red->set_text_decorations(text_decorations::underline);
        auto plain = std::make_shared<span>();
        plain->set_text("BB");
        fs->add_span(red);
        fs->add_span(plain);
        control.set_formatted_text(fs);

        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);

        NSAttributedString* const attributed = native_label(handler).attributedStringValue;
        EXPECT_EQ(to_std_string(attributed.string), "AABB");

        // Run 0 (index 0): red foreground, blue background, kerning 5, underline.
        auto* const fg = static_cast<NSColor*>(attribute_at(attributed, NSForegroundColorAttributeName, 0));
        ASSERT_NE(fg, nil);
        EXPECT_GT(fg.redComponent, 0.9);
        auto* const bg = static_cast<NSColor*>(attribute_at(attributed, NSBackgroundColorAttributeName, 0));
        ASSERT_NE(bg, nil);
        EXPECT_GT(bg.blueComponent, 0.9);
        auto* const kern = static_cast<NSNumber*>(attribute_at(attributed, NSKernAttributeName, 0));
        ASSERT_NE(kern, nil);
        EXPECT_EQ(kern.doubleValue, 5.0);
        EXPECT_NE(attribute_at(attributed, NSUnderlineStyleAttributeName, 0), nil);

        // Run 1 (index 2, the 'B'): no underline, no kerning attribute on the un-styled span.
        EXPECT_EQ(attribute_at(attributed, NSUnderlineStyleAttributeName, 2), nil);
        EXPECT_EQ(attribute_at(attributed, NSKernAttributeName, 2), nil);
    }

    TEST_F(apple_formatted_text, bold_italic_span_applies_font_traits)
    {
        label control;
        auto fs = std::make_shared<formatted_string>();
        auto s = std::make_shared<span>();
        s->set_text("Bold");
        s->set_font_size(20);
        s->set_font_attributes(font_attributes::bold | font_attributes::italic);
        fs->add_span(s);
        control.set_formatted_text(fs);

        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);

        NSAttributedString* const attributed = native_label(handler).attributedStringValue;
        auto* const run_font = static_cast<NSFont*>(attribute_at(attributed, NSFontAttributeName, 0));
        ASSERT_NE(run_font, nil);
        EXPECT_EQ(run_font.pointSize, 20.0);
        const NSFontTraitMask traits = [[NSFontManager sharedFontManager] traitsOfFont:run_font];
        EXPECT_NE(traits & NSBoldFontMask, 0U);
        EXPECT_NE(traits & NSItalicFontMask, 0U);
    }

    TEST_F(apple_formatted_text, clearing_reverts_to_plain_string)
    {
        label control;
        control.set_formatted_text(formatted_string::from_string("rich"));
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        EXPECT_EQ(to_std_string(native_label(handler).attributedStringValue.string), "rich");

        control.set_text("plain");
        EXPECT_EQ(to_std_string(native_label(handler).stringValue), "plain");
    }
} // namespace
