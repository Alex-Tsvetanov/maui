#pragma once
// Shared UIKit attributed-string operations — the iOS sibling of apple_text_ops.hpp, shared by the
// button / label / entry recipes. Objective-C++ only — include exclusively from .mm files compiled as
// Objective-C++.
//
// Ports the iOS Core extensions directly (these ARE the UIKit originals the AppKit twins were adapted
// from):
//   - kern_attributed  <-  AttributedStringExtensions.WithCharacterSpacing (+ WithTextColor): builds a
//     fresh NSAttributedString over `text`, applying KerningAdjustment when spacing != 0 and a
//     ForegroundColor when `foreground` is non-nil. Returns nil when NEITHER attribute is needed
//     (empty text, or spacing == 0 with no color) so the caller can fall back to the plain title —
//     matching C#, where WithCharacterSpacing returns null and SetAttributedTitle(null) keeps the
//     plain-title path.
//   - with_character_spacing  <-  AttributedStringExtensions.WithCharacterSpacing, the overload the
//     label/entry recipes use on an EXISTING attributed string (UILabel.AttributedText /
//     UITextField.AttributedText|AttributedPlaceholder): nil for an empty input, nil when un-setting
//     (spacing == 0) with no kerning present (the C# "bail out" branch), otherwise a mutable copy with
//     KerningAdjustment applied over the whole range — preserving every other attribute.
//   - with_decorations  <-  AttributedStringExtensions.WithDecorations (+ UpdateDecoration): a mutable
//     copy with StrikethroughStyle / UnderlineStyle each ADDED (single style) when its flag is set and
//     REMOVED when clear; nil for an empty input.
//   - kerning_of  <-  the device-test helper AssertionExtensions.GetCharacterSpacing: reads the
//     KerningAdjustment double off an attributed string (0 when absent / empty), used by the .mm tests.

#import <UIKit/UIKit.h>

#include <utility>

#include "maui/core/text_decorations.hpp"

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

    // AttributedStringExtensions.WithCharacterSpacing, operating on an existing attributed string (the
    // label/entry path — every other attribute is preserved). Returns nil when there is nothing to do:
    // empty input, or "un-set" (spacing == 0) with no KerningAdjustment present.
    inline NSAttributedString* with_character_spacing(NSAttributedString* attributed, double spacing)
    {
        if (attributed == nil || attributed.length == 0)
        {
            return nil;
        }
        id const existing = [attributed attribute:NSKernAttributeName atIndex:0 effectiveRange:nullptr];
        // "if we are going to un-set, but there is no adjustment, then bail out"
        if (spacing == 0 && existing == nil)
        {
            return nil;
        }
        NSMutableAttributedString* const mutable_copy =
            [[NSMutableAttributedString alloc] initWithAttributedString:attributed];
        [mutable_copy addAttribute:NSKernAttributeName
                             value:[NSNumber numberWithDouble:spacing]
                             range:NSMakeRange(0, mutable_copy.length)];
        return mutable_copy;
    }

    // AttributedStringExtensions.UpdateDecoration: a zero flag REMOVES the style attribute, a set flag
    // ADDS it as NSUnderlineStyleSingle (both strikethrough and underline carry the same single-style
    // value in C#).
    inline void update_decoration(NSMutableAttributedString* attributed, NSAttributedStringKey key, NSRange range,
                                  bool enabled)
    {
        if (!enabled)
        {
            [attributed removeAttribute:key range:range];
        }
        else
        {
            [attributed addAttribute:key value:[NSNumber numberWithInteger:NSUnderlineStyleSingle] range:range];
        }
    }

    // AttributedStringExtensions.WithDecorations: apply/remove the strikethrough + underline styles on a
    // mutable copy (nil for an empty input, matching the C# null return).
    inline NSAttributedString* with_decorations(NSAttributedString* attributed,
                                                maui::core::text_decorations decorations)
    {
        if (attributed == nil || attributed.length == 0)
        {
            return nil;
        }
        NSMutableAttributedString* const mutable_copy =
            [[NSMutableAttributedString alloc] initWithAttributedString:attributed];
        const NSRange range = NSMakeRange(0, mutable_copy.length);
        const auto flags = std::to_underlying(decorations);
        update_decoration(mutable_copy, NSStrikethroughStyleAttributeName, range,
                          (flags & std::to_underlying(maui::core::text_decorations::strikethrough)) != 0);
        update_decoration(mutable_copy, NSUnderlineStyleAttributeName, range,
                          (flags & std::to_underlying(maui::core::text_decorations::underline)) != 0);
        return mutable_copy;
    }
} // namespace maui::platform::ios
