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
//   - with_decorations  <-  AttributedStringExtensions.WithDecorations (+ UpdateDecoration): a mutable
//     copy with StrikethroughStyle / UnderlineStyle each ADDED (single style) when its flag is set and
//     REMOVED when clear; nil for an empty input (the same attributed keys exist on AppKit).
//   - kerning_of  <-  the device-test helper AssertionExtensions.GetCharacterSpacing: reads the
//     KerningAdjustment double off an attributed string (0 when absent / empty), used by the .mm tests.

#import <AppKit/AppKit.h>

#include <utility>
#include <vector>

#include "apple_conversions.hpp"         // to_ns_color / to_ns_font (the run-builder reuses to_ns_color)
#include "maui/core/bindable_object.hpp" // is_property_set — explicit-vs-unset TextColor discrimination
#include "maui/core/font.hpp"
#include "maui/core/label_run.hpp"
#include "maui/core/text_decorations.hpp"
#include "maui/graphics/color.hpp"

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

    // AttributedStringExtensions.WithLineHeight: set NSMutableParagraphStyle.lineHeightMultiple over the
    // whole range, copying any existing paragraph style first. line_height == -1 with no existing
    // paragraph-style attribute is the C# "un-set but nothing to modify" bail-out (returns nil); the value
    // written is clamped to -1 for any negative input (matching `lineHeight >= 0 ? lineHeight : -1`).
    inline NSAttributedString* with_line_height(NSAttributedString* attributed, double line_height)
    {
        if (attributed == nil || attributed.length == 0)
        {
            return nil;
        }
        NSParagraphStyle* const existing =
            [attributed attribute:NSParagraphStyleAttributeName atIndex:0 effectiveRange:nullptr];
        if (line_height == -1 && existing == nil)
        {
            return nil;
        }
        NSMutableParagraphStyle* const paragraph = [[NSMutableParagraphStyle alloc] init];
        if (existing != nil)
        {
            [paragraph setParagraphStyle:existing];
        }
        paragraph.lineHeightMultiple = static_cast<CGFloat>(line_height >= 0 ? line_height : -1);
        NSMutableAttributedString* const mutable_copy =
            [[NSMutableAttributedString alloc] initWithAttributedString:attributed];
        [mutable_copy addAttribute:NSParagraphStyleAttributeName
                             value:paragraph
                             range:NSMakeRange(0, mutable_copy.length)];
        return mutable_copy;
    }

    // Read the lineHeightMultiple off an attributed string's paragraph style; 0 when absent / empty (the
    // .mm tests assert on this — the AppKit analog of a GetLineHeight test helper).
    inline double line_height_multiple_of(NSAttributedString* attributed)
    {
        if (attributed == nil || attributed.length == 0)
        {
            return 0;
        }
        NSParagraphStyle* const style =
            [attributed attribute:NSParagraphStyleAttributeName atIndex:0 effectiveRange:nullptr];
        return style != nil ? static_cast<double>(style.lineHeightMultiple) : 0;
    }

    // Build an NSFont for a span run, applying the run font's bold (weight == Bold) + italic (slant !=
    // normal) as symbolic traits — the AppKit analog of Font.ToUIFont(fontManager), which the per-span
    // attributed-string build uses (to_ns_font in apple_conversions ignores traits; a run needs them).
    inline NSFont* to_ns_run_font(const maui::core::font& value)
    {
        const double size = value.size() > 0 ? value.size() : static_cast<double>(NSFont.systemFontSize);
        NSFont* base = nil;
        if (!value.family().empty())
        {
            NSString* const name = [NSString stringWithUTF8String:value.family().c_str()];
            base = name != nil ? [NSFont fontWithName:name size:size] : nil;
        }
        if (base == nil)
        {
            base = [NSFont systemFontOfSize:size];
        }
        NSFontTraitMask traits = 0;
        if (value.weight() == maui::core::font_weight::bold)
        {
            traits |= NSBoldFontMask;
        }
        if (value.slant() != maui::core::font_slant::normal)
        {
            traits |= NSItalicFontMask;
        }
        if (traits == 0)
        {
            return base;
        }
        NSFont* const traited = [[NSFontManager sharedFontManager] convertFont:base toHaveTrait:traits];
        return traited != nil ? traited : base;
    }

    // Port of FormattedStringExtensions.ToNSAttributedString: append one attributed substring per run
    // (font / foreground color / background color / underline / strikethrough / kerning / line-height
    // paragraph style), exactly as the C# loop over formattedString.Spans does. Returns an empty
    // NSAttributedString for no runs (the caller falls back to the plain-text path).
    inline NSAttributedString* attributed_from_runs(const std::vector<maui::core::label_run>& runs)
    {
        NSMutableAttributedString* const attributed = [[NSMutableAttributedString alloc] init];
        for (const maui::core::label_run& run : runs)
        {
            NSString* const text = [NSString stringWithUTF8String:run.text.c_str()];
            if (text == nil)
            {
                continue;
            }
            NSMutableDictionary<NSAttributedStringKey, id>* const attrs = [NSMutableDictionary dictionary];
            attrs[NSFontAttributeName] = to_ns_run_font(run.run_font);
            if (run.text_color.has_value())
            {
                attrs[NSForegroundColorAttributeName] = to_ns_color(*run.text_color);
            }
            if (run.background_color.has_value())
            {
                attrs[NSBackgroundColorAttributeName] = to_ns_color(*run.background_color);
            }
            const auto flags = std::to_underlying(run.decorations);
            if ((flags & std::to_underlying(maui::core::text_decorations::underline)) != 0)
            {
                attrs[NSUnderlineStyleAttributeName] = [NSNumber numberWithInteger:NSUnderlineStyleSingle];
            }
            if ((flags & std::to_underlying(maui::core::text_decorations::strikethrough)) != 0)
            {
                attrs[NSStrikethroughStyleAttributeName] = [NSNumber numberWithInteger:NSUnderlineStyleSingle];
            }
            if (run.character_spacing != 0)
            {
                attrs[NSKernAttributeName] = [NSNumber numberWithDouble:run.character_spacing];
            }
            if (run.line_height >= 0)
            {
                NSMutableParagraphStyle* const style = [[NSMutableParagraphStyle alloc] init];
                style.lineHeightMultiple = static_cast<CGFloat>(run.line_height);
                attrs[NSParagraphStyleAttributeName] = style;
            }
            NSAttributedString* const piece = [[NSAttributedString alloc] initWithString:text attributes:attrs];
            [attributed appendAttributedString:piece];
        }
        return attributed;
    }

    // The view's TextColor when it was EXPLICITLY set, nil otherwise — so the caller can fall back to the
    // SYSTEM color, which is what adapts to light/dark.
    //
    // C# ITextElement.TextColor is a `Color?` that defaults to null, and every UpdateTextColor maps null to
    // the platform's dynamic default. The port models TextColor as a non-nullable value type whose
    // default-constructed value is opaque BLACK, so `to_ns_color(view.text_color())` on an untouched view
    // silently paints black — indistinguishable from an explicit TextColor="Black", and INVISIBLE on a dark
    // background. MEASURED: the AppKit gallery rendered app_theme_binding's headings at (0,0,0) on a
    // (36,40,43) page, while MAUI's Catalyst column draws them at (234,234,234).
    //
    // The iOS lane already discriminates this way (ios/label_handler.mm map_text_color — the chat-bubble
    // cell bug), and so does this backend's button_handler; label/entry/editor/search_bar/radio_button did
    // not, which is why this now lives in the shared header instead of being copied a sixth time. Each
    // caller supplies its OWN fallback because AppKit's defaults differ per control class (labelColor,
    // textColor, controlTextColor).
    template <class View> inline NSColor* explicit_text_color_or_nil(const View& view)
    {
        const auto* const bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        return bindable != nullptr && bindable->is_property_set("text_color") ? to_ns_color(view.text_color()) : nil;
    }
} // namespace maui::platform::apple
