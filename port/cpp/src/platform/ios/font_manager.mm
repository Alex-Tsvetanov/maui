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

#include <cctype>
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

    // True when GetFontAttributes would set the CONTINUOUS UIFontWeightTrait — i.e. the weight is neither
    // Regular nor Bold (Bold is carried as the symbolic Bold trait, Regular carries no weight trait).
    bool has_continuous_weight(maui::core::font value)
    {
        return value.weight() != maui::core::font_weight::regular && value.weight() != maui::core::font_weight::bold;
    }

    // C# GetFontAttributes' UIFontTraits.Weight/Slant branch for non-bold non-regular weights: add the
    // continuous UIFontWeightTrait — together with any symbolic Italic/oblique trait — to a descriptor in a
    // SINGLE UIFontDescriptorTraitsAttribute dict (mirroring C#'s one UIFontAttributes that holds both
    // Traits.Weight and Traits.SymbolicTrait). This is what was DROPPED before — the symbolic-only path
    // rendered medium/semibold/light/… as Regular. NOTE: calling fontDescriptorWithSymbolicTraits: FIRST and
    // then adding the weight trait does NOT work (the symbolic-traits call resets the weight resolution) —
    // both must go into one attributes addition, which is also closest to GetFontAttributes' shape.
    UIFontDescriptor* descriptor_with_attributes(UIFontDescriptor* descriptor, maui::core::font value,
                                                 UIFontDescriptorSymbolicTraits symbolic)
    {
        NSMutableDictionary* const trait_attrs = [NSMutableDictionary dictionary];
        trait_attrs[UIFontWeightTrait] = @(to_ui_font_weight(value.weight()));
        if (value.slant() == maui::core::font_slant::oblique)
        {
            trait_attrs[UIFontSlantTrait] = @(30.0); // C#'s GetFontAttributes Slant = 30.0f for oblique
        }
        if (symbolic != 0)
        {
            trait_attrs[UIFontSymbolicTrait] = @(symbolic); // the symbolic Italic flag (Bold is never set here)
        }
        return [descriptor fontDescriptorByAddingAttributes:@{UIFontDescriptorTraitsAttribute : trait_attrs}];
    }

    // Map a ".SFUI-<Weight>" family's trailing segment to a UIFontWeight, mirroring C#'s
    // family.Split('-').Last() → Enum.TryParse<UIFontWeight>(…, ignoreCase: true) (Regular on no/blank/bad
    // suffix). The enum names match UIFontWeight: ultralight/thin/light/regular/medium/semibold/bold/heavy/black.
    UIFontWeight system_weight_from_family_suffix(const std::string& family)
    {
        const auto dash = family.find_last_of('-');
        if (dash == std::string::npos || dash + 1 >= family.size())
        {
            return UIFontWeightRegular;
        }
        std::string suffix = family.substr(dash + 1);
        for (char& c : suffix)
        {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        if (suffix == "ultralight")
        {
            return UIFontWeightUltraLight;
        }
        if (suffix == "thin")
        {
            return UIFontWeightThin;
        }
        if (suffix == "light")
        {
            return UIFontWeightLight;
        }
        if (suffix == "regular")
        {
            return UIFontWeightRegular;
        }
        if (suffix == "medium")
        {
            return UIFontWeightMedium;
        }
        if (suffix == "semibold")
        {
            return UIFontWeightSemibold;
        }
        if (suffix == "bold")
        {
            return UIFontWeightBold;
        }
        if (suffix == "heavy")
        {
            return UIFontWeightHeavy;
        }
        if (suffix == "black")
        {
            return UIFontWeightBlack;
        }
        return UIFontWeightRegular; // Enum.TryParse failure → Regular
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
            // C#'s descriptor.CreateWithAttributes(GetFontAttributes(font)): the symbolic Bold/Italic flags
            // PLUS the continuous UIFontWeightTrait for non-bold non-regular weights (which was dropped
            // before — medium/semibold/light rendered as Regular).
            if (family_is_available(ns_family))
            {
                UIFontDescriptor* descriptor =
                    [UIFontDescriptor fontDescriptorWithFontAttributes:@{UIFontDescriptorFamilyAttribute : ns_family}];
                if (has_attributes)
                {
                    if (has_continuous_weight(value))
                    {
                        // Non-bold non-regular: fold the continuous weight + any symbolic Italic into ONE
                        // attributes addition (a separate fontDescriptorWithSymbolicTraits: call would reset
                        // the weight). Bold is never reached here (it's the symbolic-only path below).
                        descriptor = descriptor_with_attributes(descriptor, value, symbolic_traits(value));
                    }
                    else
                    {
                        descriptor = [descriptor fontDescriptorWithSymbolicTraits:symbolic_traits(value)];
                    }
                }
                result = [UIFont fontWithDescriptor:descriptor size:size];
                if (result != nil)
                {
                    return make(result);
                }
            }

            // ".SFUI-*" — the system font, by the suffix weight if it parses, else Regular. C# parses
            // family.Split('-').Last() → Enum.TryParse<UIFontWeight> (FontManager.iOS.cs:134-151) and
            // requests SystemFontOfSize(size, parsedWeight) — driven by the family NAME suffix, NOT
            // value.weight(). (e.g. ".SFUI-Semibold" with weight=Regular → a semibold system font.)
            if ([ns_family.lowercaseString hasPrefix:@".sfui"])
            {
                result = [UIFont systemFontOfSize:size weight:system_weight_from_family_suffix(family)];
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
        // descriptor when the font has any (C#'s SystemFontOfSize(size).FontDescriptor.CreateWithAttributes
        // (GetFontAttributes(font))). For non-bold non-regular weights GetFontAttributes sets the CONTINUOUS
        // UIFontWeightTrait — start from the correctly-weighted system font so the weight is never dropped
        // (the symbolic-only path rendered medium/semibold as Regular), then overlay any symbolic Italic.
        if (has_attributes)
        {
            UIFont* const base = has_continuous_weight(value)
                                     ? [UIFont systemFontOfSize:size weight:to_ui_font_weight(value.weight())]
                                     : [UIFont systemFontOfSize:size];
            const UIFontDescriptorSymbolicTraits symbolic = symbolic_traits(value);
            if (symbolic == 0)
            {
                return make(base); // continuous weight already baked into `base`; no Bold/Italic to add
            }
            UIFontDescriptor* const descriptor = [base.fontDescriptor fontDescriptorWithSymbolicTraits:symbolic];
            UIFont* const traited = [UIFont fontWithDescriptor:descriptor size:size];
            return make(traited != nil ? traited : base);
        }
        return make([UIFont systemFontOfSize:size]);
    }
} // namespace maui::core
