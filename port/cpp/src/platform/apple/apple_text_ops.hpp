#pragma once
// Shared AppKit attributed-string operations for the text controls (button / label / entry) — the
// platform side of character_spacing (NSKernAttributeName) + the attributed-placeholder foreground color.
// Objective-C++ only — include exclusively from .mm files compiled as Objective-C++ (it references
// NSAttributedString / NSColor).
//
// Ports the iOS Core extensions, with AppKit's NSKernAttributeName standing in for UIKit's
// UIStringAttributeKey.KerningAdjustment (the same kerning attribute):
//   - kern_attributed  <-  AttributedStringExtensions.WithCharacterSpacing (+ WithTextColor): builds a
//     fresh NSAttributedString over `text`, applying KerningAdjustment when spacing != 0 and a
//     ForegroundColor when `foreground` is non-nil. Returns nil when NEITHER attribute is needed (empty
//     text, or spacing == 0 with no color) so the caller can fall back to the plain stringValue/title —
//     matching C#, where WithCharacterSpacing returns null and the control keeps its plain text path.
//   - placeholder_attributed  <-  TextFieldExtensions.UpdatePlaceholder: builds the placeholder string
//     with an optional foreground color, then applies WithCharacterSpacing. Returns nil only when `text`
//     is nil (C#: AttributedPlaceholder = null when Placeholder == null); otherwise it always returns an
//     attributed placeholder (the color may be absent — the system default — but the plain-text branch
//     `new NSAttributedString(placeholder)` is still attributed).
//   - kerning_of  <-  the device-test helper AssertionExtensions.GetCharacterSpacing: reads the
//     KerningAdjustment double off an attributed string (0 when absent / empty), used by the .mm tests.

#import <AppKit/AppKit.h>

namespace maui::platform::apple
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
    // foreground color (when non-nil). Returns nil when neither attribute is needed so the caller can use
    // the plain string (matching WithCharacterSpacing returning null on a fresh, un-kerned string).
    inline NSAttributedString* kern_attributed(NSString* text, double spacing, NSColor* foreground)
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

    // Build the attributed placeholder string: optional foreground color + kerning. Returns nil only when
    // `text` is nil (UpdatePlaceholder sets AttributedPlaceholder = null when Placeholder == null).
    inline NSAttributedString* placeholder_attributed(NSString* text, NSColor* foreground, double spacing)
    {
        if (text == nil)
        {
            return nil;
        }
        NSMutableAttributedString* const attributed = [[NSMutableAttributedString alloc] initWithString:text];
        const NSRange range = NSMakeRange(0, attributed.length);
        if (foreground != nil && attributed.length > 0)
        {
            [attributed addAttribute:NSForegroundColorAttributeName value:foreground range:range];
        }
        if (spacing != 0 && attributed.length > 0)
        {
            [attributed addAttribute:NSKernAttributeName value:[NSNumber numberWithDouble:spacing] range:range];
        }
        return attributed;
    }
} // namespace maui::platform::apple
