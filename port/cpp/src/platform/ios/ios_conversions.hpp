#pragma once
// Shared UIKit conversions from maui value types — the iOS sibling of apple_conversions.hpp.
// Objective-C++ only — include exclusively from .mm files compiled as Objective-C++ (it references
// UIColor/UIFont).

#import <UIKit/UIKit.h>

#include <string>

#include "maui/core/font.hpp"
#include "maui/core/line_break_mode.hpp"
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

    // The default font SIZE a text control (Label/Entry/Editor/Picker/DatePicker/TimePicker) uses when its
    // FontSize is unset. MAUI gives FontSize a default-value-creator (FontElement.FontSizeDefaultValueCreator)
    // returning IFontManager.DefaultFontSize = UIFont.SystemFontSize (14pt), so a control's font.Size is
    // already 14 by the time it reaches the handler — the UIFont.LabelFontSize (17pt) fallback inside MAUI's
    // *Extensions.UpdateFont is never actually reached for a real control. The port has no such creator, so
    // the handler's default IS the effective default and must be SystemFontSize to match MAUI. (Porting the
    // LabelFontSize fallback literally rendered every implicit-size label/entry/… at 17pt instead of 14pt — a
    // ~1.2× over-size the 3-column iOS comparison surfaced; see docs/comparison/EXAMPLES_XAML.md.)
    [[nodiscard]] inline double default_text_font_size()
    {
        return static_cast<double>(UIFont.systemFontSize);
    }

    // maui font → UIFont. `default_size` is the control's system default when the font carries no size
    // (e.g. default_text_font_size() for text controls, UIFont.buttonFontSize for buttons); an unknown family
    // falls back to the system font, like FontManager's font-registrar miss path.
    inline UIFont* to_ui_font(const maui::core::font& value, double default_size)
    {
        const double size = value.size() > 0 ? value.size() : default_size;
        UIFont* base = nil;
        if (!value.family().empty())
        {
            NSString* const name = [NSString stringWithUTF8String:value.family().c_str()];
            base = name != nil ? [UIFont fontWithName:name size:size] : nil;
        }
        if (base == nil)
        {
            base = [UIFont systemFontOfSize:size];
        }
        // Fold the font's weight + slant into the UIFont symbolic traits (Font.ToUIFont's WithAttributes
        // path): Bold → Trait Bold, Italic → Trait Italic. Without this a Label/Button/Entry/etc. with
        // FontAttributes=Bold/Italic rendered REGULAR (the traits were applied only to attributed runs).
        UIFontDescriptorSymbolicTraits traits = 0;
        if (value.weight() == maui::core::font_weight::bold)
        {
            traits |= UIFontDescriptorTraitBold;
        }
        if (value.slant() != maui::core::font_slant::normal)
        {
            traits |= UIFontDescriptorTraitItalic;
        }
        if (traits == 0)
        {
            return base;
        }
        UIFontDescriptor* const descriptor =
            [base.fontDescriptor fontDescriptorWithSymbolicTraits:(base.fontDescriptor.symbolicTraits | traits)];
        return descriptor != nil ? [UIFont fontWithDescriptor:descriptor size:size] : base;
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

    // maui line_break_mode → NSLineBreakMode (the type behind UILabel.lineBreakMode — C#'s
    // UILineBreakMode binding). Ports the LineBreakMode→UILineBreakMode mapping inside
    // TextExtensions.SetLineBreakMode (NoWrap→Clip, WordWrap→WordWrapping, CharacterWrap→CharWrapping,
    // *Truncation→TruncatingHead/Middle/Tail). The numberOfLines half of SetLineBreakMode is the
    // handler's concern (it also needs MaxLines), so this helper is the wrap-mode half only.
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
