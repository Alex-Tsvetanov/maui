#pragma once
// Shared AppKit conversions from maui value types. Objective-C++ only — include exclusively from .mm
// files compiled as Objective-C++ (it references NSColor/NSFont).

#import <AppKit/AppKit.h>

#include <string>

#include "maui/core/font.hpp"
#include "maui/core/line_break_mode.hpp"
#include "maui/graphics/color.hpp"

namespace maui::platform::apple
{
    inline NSColor* to_ns_color(const maui::graphics::color& value)
    {
        return [NSColor colorWithSRGBRed:value.red green:value.green blue:value.blue alpha:value.alpha];
    }

    inline NSFont* to_ns_font(const maui::core::font& value)
    {
        const double size = value.size() > 0 ? value.size() : static_cast<double>(NSFont.systemFontSize);
        if (!value.family().empty())
        {
            NSString* const name = [NSString stringWithUTF8String:value.family().c_str()];
            NSFont* const named = name != nil ? [NSFont fontWithName:name size:size] : nil;
            if (named != nil)
            {
                return named;
            }
        }
        return [NSFont systemFontOfSize:size];
    }

    // maui line_break_mode → NSLineBreakMode (the type behind NSTextFieldCell.lineBreakMode). The AppKit
    // analog of the LineBreakMode→UILineBreakMode mapping inside TextExtensions.SetLineBreakMode: NoWrap→
    // Clipping, WordWrap→WordWrapping, CharacterWrap→CharWrapping, *Truncation→TruncatingHead/Middle/Tail.
    // The numberOfLines half (which also needs MaxLines) is the handler's concern, like the iOS twin.
    inline NSLineBreakMode to_ns_line_break_mode(maui::core::line_break_mode value)
    {
        switch (value)
        {
            case maui::core::line_break_mode::no_wrap:
                return NSLineBreakByClipping;
            case maui::core::line_break_mode::word_wrap:
                return NSLineBreakByWordWrapping;
            case maui::core::line_break_mode::character_wrap:
                return NSLineBreakByCharWrapping;
            case maui::core::line_break_mode::head_truncation:
                return NSLineBreakByTruncatingHead;
            case maui::core::line_break_mode::middle_truncation:
                return NSLineBreakByTruncatingMiddle;
            case maui::core::line_break_mode::tail_truncation:
                return NSLineBreakByTruncatingTail;
        }
        return NSLineBreakByWordWrapping; // unreachable for in-range values (enum is closed)
    }
} // namespace maui::platform::apple
