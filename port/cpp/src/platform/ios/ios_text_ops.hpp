#pragma once
// Shared UIKit attributed-string operations — the iOS sibling of apple_text_ops.hpp, scoped to what
// the M6 scaffold's button needs (the label/entry placeholder helpers arrive with their fan-out
// units). Objective-C++ only — include exclusively from .mm files compiled as Objective-C++.
//
// Ports the iOS Core extensions directly (these ARE the UIKit originals the AppKit twins were adapted
// from):
//   - kern_attributed  <-  AttributedStringExtensions.WithCharacterSpacing (+ WithTextColor): builds a
//     fresh NSAttributedString over `text`, applying KerningAdjustment when spacing != 0 and a
//     ForegroundColor when `foreground` is non-nil. Returns nil when NEITHER attribute is needed
//     (empty text, or spacing == 0 with no color) so the caller can fall back to the plain title —
//     matching C#, where WithCharacterSpacing returns null and SetAttributedTitle(null) keeps the
//     plain-title path.
//   - kerning_of  <-  the device-test helper AssertionExtensions.GetCharacterSpacing: reads the
//     KerningAdjustment double off an attributed string (0 when absent / empty), used by the .mm tests.

#import <UIKit/UIKit.h>

namespace maui::platform::ios
{
    // Read the KerningAdjustment (character spacing) off an attributed string; 0 when absent or empty.
    // Mirrors AssertionExtensions.GetCharacterSpacing (iOS device tests).
    inline double kerning_of(NSAttributedString* attributed)
    {
        if (attributed == nil || attributed.length == 0)
        {
            return 0;
        }
        id const value = [attributed attribute:NSKernAttributeName atIndex:0 effectiveRange:nullptr];
        if (![value isKindOfClass:[NSNumber class]])
        {
            return 0;
        }
        return ((NSNumber*)value).doubleValue;
    }

    // Build a fresh attributed string for the visible text, applying kerning (when spacing != 0) and a
    // foreground color (when non-nil). Returns nil when neither attribute is needed so the caller can
    // use the plain string (matching WithCharacterSpacing returning null on a fresh, un-kerned string).
    inline NSAttributedString* kern_attributed(NSString* text, double spacing, UIColor* foreground)
    {
        if (text == nil || text.length == 0)
        {
            return nil;
        }
        if (spacing == 0 && foreground == nil)
        {
            return nil;
        }
        NSMutableAttributedString* const attributed = [[NSMutableAttributedString alloc] initWithString:text];
        const NSRange range = NSMakeRange(0, attributed.length);
        if (spacing != 0)
        {
            [attributed addAttribute:NSKernAttributeName value:[NSNumber numberWithDouble:spacing] range:range];
        }
        if (foreground != nil)
        {
            [attributed addAttribute:NSForegroundColorAttributeName value:foreground range:range];
        }
        return attributed;
    }
} // namespace maui::platform::ios
