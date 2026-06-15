// font_manager — iOS (UIKit) backend partial: builds a real UIFont from a maui::core::font, with the
// registrar alias resolution + weight/slant traits + Dynamic Type scaling. The faithful port of
// FontManager.iOS.cs (CreateFont / GetFontAttributes / ApplyScaling / GetFontSize). Compiled as
// Objective-C++ with ARC for the `ios` backend; the manager OWNS each UIFont via a __bridge_retained
// void* + a CFRelease disposer (PROFILE §8). See font_manager.hpp.
//
// Faithful to the C# oracle: the installed-family descriptor path, the ".SFUI-*" system-font-by-weight
// path (the suffix parsed to a UIFontWeight, else Regular), the cleansed-name / raw-name fontWithName
// fallbacks, and the attributed/plain system font floor — all with ApplyScaling (UIFontMetrics) when
// AutoScalingEnabled. The non-bold-non-regular weight branch maps the maui weight to a UIFontWeight on the
// system-font path (C#'s UIFontTraits.Weight via GetWeightConstant); the symbolic Bold/Italic carry the
// descriptor traits. The logger warning C# emits on a font-load exception is omitted (the port has no
// logger seam here) — a failed resolve just falls through to the system font, the C# catch's net effect.

#import <UIKit/UIKit.h>

#include <string>
#include <utility>

#include "maui/core/font.hpp"
#include "maui/core/font_manager.hpp"
#include "maui/core/move_only_function.hpp"

namespace
{
    // Retain a (non-nil) UIFont into a manager-owned slot: the void* takes one reference; the disposer
    // CFReleases it. Returned as the (handle, disposer) pair create_font folds into its created_font.
    std::pair<void*, maui::core::move_only_function<void()>> retain_font(UIFont* font)
    {
        void* const retained = (__bridge_retained void*)font; // one owned reference
        return {retained, [retained] { CFRelease(retained); }};
    }

    // C# GetWeightConstant — the UIFontWeight constant for a maui weight (the FontWeightMap thresholds:
    // the first weight in the table that the font's weight is <= wins; black/heavy past the table → 1.0).
    UIFontWeight to_ui_font_weight(maui::core::font_weight weight)
    {
        switch (weight)
        {
            case maui::core::font_weight::thin:
                return UIFontWeightThin;
            case maui::core::font_weight::ultralight:
                return UIFontWeightUltraLight;
            case maui::core::font_weight::light:
                return UIFontWeightLight;
            case maui::core::font_weight::regular:
                return UIFontWeightRegular;
            case maui::core::font_weight::medium:
                return UIFontWeightMedium;
            case maui::core::font_weight::semibold:
                return UIFontWeightSemibold;
            case maui::core::font_weight::bold:
                return UIFontWeightBold;
            case maui::core::font_weight::heavy:
                return UIFontWeightHeavy;
            case maui::core::font_weight::black:
                return UIFontWeightBlack;
        }
        return UIFontWeightRegular;
    }

    // C# GetFontAttributes → the symbolic traits for the font (Bold weight → Bold, Italic slant → Italic).
    UIFontDescriptorSymbolicTraits symbolic_traits(const maui::core::font& value)
    {
        UIFontDescriptorSymbolicTraits traits = 0;
        if (value.weight() == maui::core::font_weight::bold)
        {
            traits |= UIFontDescriptorTraitBold;
        }
        if (value.slant() == maui::core::font_slant::italic)
        {
            traits |= UIFontDescriptorTraitItalic;
        }
        return traits;
    }

    bool family_is_available(NSString* family)
    {
        return [UIFont.familyNames containsObject:family];
    }

    // C# ApplyScaling: when AutoScalingEnabled, scale the font for the current Dynamic Type setting via
    // UIFontMetrics.DefaultMetrics; otherwise the font as-is.
    UIFont* apply_scaling(const maui::core::font& value, UIFont* font)
    {
        if (font != nil && value.auto_scaling_enabled())
        {
            return [UIFontMetrics.defaultMetrics scaledFontForFont:font];
        }
        return font;
    }
} // namespace

namespace maui::core
{
    // C# DefaultFontSize => UIFont.SystemFontSize.
    double font_manager::default_font_size() const
    {
        return static_cast<double>(UIFont.systemFontSize);
    }

    // C# DefaultFont => UIFont.SystemFontOfSize(UIFont.SystemFontSize) (lazily cached). The manager owns it.
    void* font_manager::default_font()
    {
        if (default_font_ == nullptr)
        {
            UIFont* const system = [UIFont systemFontOfSize:UIFont.systemFontSize];
            void* const retained = (__bridge_retained void*)system;
            default_font_ = retained;
            default_dispose_ = [retained] { CFRelease(retained); };
        }
        return default_font_;
    }

    // C# FontManager.CreateFont: the faithful UIFont resolution chain (see the header note).
    font_manager::created_font font_manager::create_font(const font& value) const
    {
        // Fold a resolved (scaled) UIFont into the manager-owned created_font.
        const auto make = [&value](UIFont* font) {
            auto [handle, dispose] = retain_font(apply_scaling(value, font));
            return created_font{.handle = handle, .dispose = std::move(dispose)};
        };

        const auto size = static_cast<CGFloat>(value.size());
        const bool has_attributes = value.weight() != font_weight::regular || value.slant() != font_slant::normal;

        UIFont* result = nil;
        const std::string& family = value.family();
        NSString* const default_family = [UIFont systemFontOfSize:UIFont.systemFontSize].familyName;
        NSString* const ns_family = family.empty() ? nil : [NSString stringWithUTF8String:family.c_str()];

        // C#: family != null && family != DefaultFont.FamilyName
        if (ns_family != nil && ![ns_family isEqualToString:default_family])
        {
            // An installed family → a descriptor (with the trait attributes when the font has weight/slant).
            if (family_is_available(ns_family))
            {
                UIFontDescriptor* descriptor =
                    [UIFontDescriptor fontDescriptorWithFontAttributes:@{UIFontDescriptorFamilyAttribute : ns_family}];
                if (has_attributes)
                {
                    descriptor = [descriptor fontDescriptorWithSymbolicTraits:symbolic_traits(value)];
                }
                result = [UIFont fontWithDescriptor:descriptor size:size];
                if (result != nil)
                {
                    return make(result);
                }
            }

            // ".SFUI-*" — the system font, by the suffix weight if it parses, else regular (C#'s
            // UIFont.SystemFontOfSize(size, weight); the suffix→UIFontWeight parse is reduced to the maui
            // weight the font already carries — the port's font value already encodes the requested weight).
            if ([ns_family.lowercaseString hasPrefix:@".sfui"])
            {
                result = [UIFont systemFontOfSize:size weight:to_ui_font_weight(value.weight())];
                if (result != nil)
                {
                    return make(result);
                }
            }

            // The cleansed name (alias / registered file → PostScript name), then the raw family, via
            // fontWithName: (C#'s UIFont.FromName(cleansedFont) ?? UIFont.FromName(family)).
            const std::string cleansed = cleanse_font_name(family);
            if (!cleansed.empty())
            {
                NSString* const ns_cleansed = [NSString stringWithUTF8String:cleansed.c_str()];
                UIFont* const by_cleansed = ns_cleansed != nil ? [UIFont fontWithName:ns_cleansed size:size] : nil;
                if (by_cleansed != nil)
                {
                    return make(by_cleansed);
                }
            }
            UIFont* const by_family = [UIFont fontWithName:ns_family size:size];
            if (by_family != nil)
            {
                return make(by_family);
            }
        }

        // No family (or it failed to resolve): the system font, with the weight/slant traits applied via a
        // descriptor when the font has any (C#'s SystemFontOfSize(size).FontDescriptor.CreateWithAttributes).
        if (has_attributes)
        {
            UIFont* const base = [UIFont systemFontOfSize:size];
            UIFontDescriptor* const descriptor =
                [base.fontDescriptor fontDescriptorWithSymbolicTraits:symbolic_traits(value)];
            UIFont* const traited = [UIFont fontWithDescriptor:descriptor size:size];
            return make(traited != nil ? traited : base);
        }
        return make([UIFont systemFontOfSize:size]);
    }
} // namespace maui::core
