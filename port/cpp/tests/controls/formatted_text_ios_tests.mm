// iOS (UIKit) backend tests for label.formatted_text (gap-closure G1) — the resolved per-span runs build
// a real NSAttributedString on the UILabel (font traits / color / background / underline / kerning), and
// clearing reverts to the plain string. Run only for MAUI_BACKEND=ios (executed ON the iOS simulator via
// tools/ios-sim-run.sh). Mirrors the AppKit twin's coverage. Compiled as Objective-C++ with ARC.
#import <UIKit/UIKit.h>

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

    UILabel* native_label(const std::shared_ptr<label_handler>& handler)
    {
        return (__bridge UILabel*)handler->typed_platform_view()->native;
    }

    id attribute_at(NSAttributedString* attributed, NSAttributedStringKey key, NSUInteger index)
    {
        return [attributed attribute:key atIndex:index effectiveRange:nullptr];
    }

    TEST(ios_formatted_text, builds_attributed_string_with_per_span_attributes)
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

        NSAttributedString* const attributed = native_label(handler).attributedText;
        ASSERT_NE(attributed, nil);
        EXPECT_EQ(to_std_string(attributed.string), "AABB");

        auto* const fg = static_cast<UIColor*>(attribute_at(attributed, NSForegroundColorAttributeName, 0));
        ASSERT_NE(fg, nil);
        CGFloat r = 0;
        CGFloat g = 0;
        CGFloat b = 0;
        CGFloat a = 0;
        [fg getRed:&r green:&g blue:&b alpha:&a];
        EXPECT_GT(r, 0.9);
        auto* const kern = static_cast<NSNumber*>(attribute_at(attributed, NSKernAttributeName, 0));
        ASSERT_NE(kern, nil);
        EXPECT_EQ(kern.doubleValue, 5.0);
        EXPECT_NE(attribute_at(attributed, NSUnderlineStyleAttributeName, 0), nil);
        EXPECT_NE(attribute_at(attributed, NSBackgroundColorAttributeName, 0), nil);

        // The un-styled run (index 2) carries no underline / kerning attribute.
        EXPECT_EQ(attribute_at(attributed, NSUnderlineStyleAttributeName, 2), nil);
        EXPECT_EQ(attribute_at(attributed, NSKernAttributeName, 2), nil);
    }

    TEST(ios_formatted_text, bold_italic_span_applies_font_traits)
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

        NSAttributedString* const attributed = native_label(handler).attributedText;
        auto* const run_font = static_cast<UIFont*>(attribute_at(attributed, NSFontAttributeName, 0));
        ASSERT_NE(run_font, nil);
        EXPECT_EQ(run_font.pointSize, 20.0);
        const UIFontDescriptorSymbolicTraits traits = run_font.fontDescriptor.symbolicTraits;
        EXPECT_NE(traits & UIFontDescriptorTraitBold, 0U);
        EXPECT_NE(traits & UIFontDescriptorTraitItalic, 0U);
    }

    TEST(ios_formatted_text, clearing_reverts_to_plain_string)
    {
        label control;
        control.set_formatted_text(formatted_string::from_string("rich"));
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        ASSERT_NE(native_label(handler).attributedText, nil);
        EXPECT_EQ(to_std_string(native_label(handler).attributedText.string), "rich");

        control.set_text("plain");
        EXPECT_EQ(to_std_string(native_label(handler).text), "plain");
    }
} // namespace
