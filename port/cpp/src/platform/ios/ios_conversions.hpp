#pragma once
// Shared UIKit conversions from maui value types — the iOS sibling of apple_conversions.hpp.
// Objective-C++ only — include exclusively from .mm files compiled as Objective-C++ (it references
// UIColor/UIFont).

#import <UIKit/UIKit.h>

#include <string>

#include "maui/core/font.hpp"
#include "maui/core/return_type.hpp"
#include "maui/core/text_alignment.hpp"
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

    // maui text_alignment → NSTextAlignment (the type behind UIKit's textAlignment — C#'s
    // UITextAlignment binding). Ports TextAlignmentExtensions.ToPlatformHorizontal (the
    // layout-direction-free overload; the RTL Left/Right flip rides with FlowDirection, which is the
    // shared ViewMapper's concern, not the per-control text recipe).
    inline NSTextAlignment to_ns_text_alignment(maui::core::text_alignment value)
    {
        switch (value)
        {
            case maui::core::text_alignment::center:
                return NSTextAlignmentCenter;
            case maui::core::text_alignment::justify:
                return NSTextAlignmentJustified;
            case maui::core::text_alignment::end:
                return NSTextAlignmentRight;
            case maui::core::text_alignment::start:
                return NSTextAlignmentLeft;
        }
        return NSTextAlignmentLeft;
    }

    // maui text_alignment → UIControlContentVerticalAlignment. Ports TextAlignmentExtensions
    // .ToPlatformVertical (Center→Center, End→Bottom, Start→Top, anything else→Top — so justify,
    // which has no vertical analog, falls to Top exactly like the C# `_` arm).
    inline UIControlContentVerticalAlignment to_ui_control_content_vertical_alignment(maui::core::text_alignment value)
    {
        switch (value)
        {
            case maui::core::text_alignment::center:
                return UIControlContentVerticalAlignmentCenter;
            case maui::core::text_alignment::end:
                return UIControlContentVerticalAlignmentBottom;
            case maui::core::text_alignment::start:
            case maui::core::text_alignment::justify:
                return UIControlContentVerticalAlignmentTop;
        }
        return UIControlContentVerticalAlignmentTop;
    }

    // maui return_type → UIReturnKeyType. Ports ReturnTypeExtensions.ToPlatform (REAL on iOS — the
    // software keyboard styles its return key; AppKit had no analog and recorded a mirror).
    inline UIReturnKeyType to_ui_return_key_type(maui::core::return_type value)
    {
        switch (value)
        {
            case maui::core::return_type::go:
                return UIReturnKeyGo;
            case maui::core::return_type::next:
                return UIReturnKeyNext;
            case maui::core::return_type::send:
                return UIReturnKeySend;
            case maui::core::return_type::search:
                return UIReturnKeySearch;
            case maui::core::return_type::done:
                return UIReturnKeyDone;
            case maui::core::return_type::default_:
                return UIReturnKeyDefault;
        }
        return UIReturnKeyDefault; // unreachable for in-range values (C# throws on unknown; enum is closed)
    }
} // namespace maui::platform::ios
