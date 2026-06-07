// Apple (AppKit) backend tests for the label seam — properties pushed to a real NSTextField (label
// style). Compiled as Objective-C++ with ARC for the `apple` backend.
#import <AppKit/AppKit.h>

#include <memory>
#include <string>

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

    NSTextField* native_label(const std::shared_ptr<label_handler>& handler)
    {
        return (__bridge NSTextField*)handler->typed_platform_view()->native;
    }

    struct apple_label_seam : ::testing::Test
    {
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
        EXPECT_EQ(std::string(native_label(handler).stringValue.UTF8String), "Start");
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
} // namespace
