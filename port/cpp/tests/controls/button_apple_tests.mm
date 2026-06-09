// Apple (AppKit) backend tests for the button seam — the real-native half of the M2 Rosetta Stone,
// run only for MAUI_BACKEND=apple. Drives a genuine NSButton: Text maps to NSButton.title, and a
// native click ([NSButton performClick:] fires the target-action without a run loop) flows back through
// the handler to the control's `clicked` event. Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>

#include <memory>
#include <string>

#include "apple_text_ops.hpp"
#include "maui/controls/button.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/font.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::button;
    using maui::core::button_handler;
    using maui::core::i_element_handler;
    using maui::platform::apple::kerning_of;

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

    // NSButton creation needs the shared application object (no run loop required).
    class apple_button_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_button_seam, attaching_handler_creates_nsbutton_and_maps_text)
    {
        button control;
        control.set_text("Start");
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_EQ(to_std_string(native_button(handler).title), "Start");
    }

    TEST_F(apple_button_seam, setting_text_updates_nsbutton_title)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        control.set_text("Changed");
        EXPECT_EQ(to_std_string(native_button(handler).title), "Changed");
    }

    TEST_F(apple_button_seam, native_click_flows_back_to_clicked_event)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        int clicks = 0;
        control.clicked.connect([&clicks] { ++clicks; });

        [native_button(handler) performClick:nil]; // simulate a real tap

        EXPECT_EQ(clicks, 1);
    }

    TEST_F(apple_button_seam, disabled_button_click_is_suppressed)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        control.set_is_enabled(false);

        int clicks = 0;
        control.clicked.connect([&clicks] { ++clicks; });
        [native_button(handler) performClick:nil];

        EXPECT_EQ(clicks, 0);
    }

    TEST_F(apple_button_seam, clearing_handler_disconnects)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST_F(apple_button_seam, appearance_maps_to_nsbutton)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        NSButton* const view = native_button(handler);

        control.set_font(maui::core::font::of_size("Helvetica", 18));
        EXPECT_EQ(view.font.pointSize, 18.0);

        control.set_stroke_thickness(3.0);
        EXPECT_TRUE(view.wantsLayer);
        EXPECT_EQ(view.layer.borderWidth, 3.0);

        control.set_corner_radius(7);
        EXPECT_EQ(view.layer.cornerRadius, 7.0);
    }

    // Ports ButtonHandlerTests.iOS CharacterSpacingInitializesCorrectly: the kerning on the (now
    // attributed) title equals the cross-platform CharacterSpacing.
    TEST_F(apple_button_seam, character_spacing_kerns_the_attributed_title)
    {
        button control;
        control.set_text("Test");
        control.set_character_spacing(4);
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        NSButton* const view = native_button(handler);
        EXPECT_EQ(kerning_of(view.attributedTitle), 4.0);
        EXPECT_EQ(to_std_string(view.attributedTitle.string), "Test");
    }

    // Ports ButtonHandlerTests.iOS CharacterSpacingAndTextColorInitializesCorrectly: the kerned title
    // carries both the spacing and the explicit text color.
    TEST_F(apple_button_seam, character_spacing_with_text_color_sets_both_on_the_title)
    {
        button control;
        control.set_text("Test");
        control.set_character_spacing(4);
        control.set_text_color(maui::graphics::color(1.0F, 0.0F, 0.5F)); // hot-pink-ish
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        NSButton* const view = native_button(handler);
        EXPECT_EQ(kerning_of(view.attributedTitle), 4.0);
        NSColor* const fg = [view.attributedTitle attribute:NSForegroundColorAttributeName
                                                    atIndex:0
                                             effectiveRange:nullptr];
        ASSERT_NE(fg, nil);
        NSColor* const srgb = [fg colorUsingColorSpace:NSColorSpace.sRGBColorSpace];
        EXPECT_NEAR(srgb.redComponent, 1.0, 0.01);
        EXPECT_NEAR(srgb.blueComponent, 0.5, 0.01);
    }

    // Spacing back to 0 reverts to a plain (un-kerned) title.
    TEST_F(apple_button_seam, clearing_character_spacing_reverts_to_plain_title)
    {
        button control;
        control.set_text("Test");
        control.set_character_spacing(4);
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        EXPECT_EQ(kerning_of(native_button(handler).attributedTitle), 4.0);

        control.set_character_spacing(0);
        EXPECT_EQ(kerning_of(native_button(handler).attributedTitle), 0.0);
    }

    // Ports ButtonHandlerTests Padding: the maui padding reaches the custom cell's content insets and
    // enlarges the cell's measured size by the padding (the custom cell's cellSizeForBounds: override).
    TEST_F(apple_button_seam, padding_maps_to_cell_insets_and_grows_cell_size)
    {
        button control;
        control.set_text("Test");
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        NSButton* const view = native_button(handler);
        const NSSize unpadded = [(NSCell*)view.cell cellSize];

        control.set_padding(maui::core::thickness(5, 10, 15, 20));
        ASSERT_TRUE([view.cell respondsToSelector:@selector(contentInsets)]);
        const NSEdgeInsets insets = [(id)view.cell contentInsets];
        EXPECT_EQ(insets.left, 5.0);
        EXPECT_EQ(insets.top, 10.0);
        EXPECT_EQ(insets.right, 15.0);
        EXPECT_EQ(insets.bottom, 20.0);

        // The cell reserves left+right (20) horizontally and top+bottom (30) vertically.
        const NSSize padded = [(NSCell*)view.cell cellSize];
        EXPECT_NEAR(padded.width - unpadded.width, 20.0, 0.5);
        EXPECT_NEAR(padded.height - unpadded.height, 30.0, 0.5);
    }

    TEST_F(apple_button_seam, handler_resolved_from_default_registry)
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
        auto const button_view = (__bridge NSButton*)resolved->typed_platform_view()->native;
        EXPECT_EQ(to_std_string(button_view.title), "Registered");
    }
} // namespace
