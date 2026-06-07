#pragma once
// Shared AppKit conversions from maui value types. Objective-C++ only — include exclusively from .mm
// files compiled as Objective-C++ (it references NSColor/NSFont).

#import <AppKit/AppKit.h>

#include <string>

#include "maui/core/font.hpp"
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
} // namespace maui::platform::apple
