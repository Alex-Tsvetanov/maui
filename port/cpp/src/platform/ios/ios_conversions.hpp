#pragma once
// Shared UIKit conversions from maui value types — the iOS sibling of apple_conversions.hpp.
// Objective-C++ only — include exclusively from .mm files compiled as Objective-C++ (it references
// UIColor/UIFont).

#import <UIKit/UIKit.h>

#include <string>

#include "maui/core/font.hpp"
#include "maui/graphics/color.hpp"

namespace maui::platform::ios
{
    // maui color → UIColor (colorWithRed: is sRGB, matching the cross-platform color space).
    inline UIColor* to_ui_color(const maui::graphics::color& value)
    {
        return [UIColor colorWithRed:value.red green:value.green blue:value.blue alpha:value.alpha];
    }

    // maui font → UIFont. `default_size` is the control's system default when the font carries no size
    // (FontManager falls back per control — e.g. UIFont.buttonFontSize for buttons); an unknown family
    // falls back to the system font, like FontManager's font-registrar miss path.
    inline UIFont* to_ui_font(const maui::core::font& value, double default_size)
    {
        const double size = value.size() > 0 ? value.size() : default_size;
        if (!value.family().empty())
        {
            NSString* const name = [NSString stringWithUTF8String:value.family().c_str()];
            UIFont* const named = name != nil ? [UIFont fontWithName:name size:size] : nil;
            if (named != nil)
            {
                return named;
            }
        }
        return [UIFont systemFontOfSize:size];
    }
} // namespace maui::platform::ios
