// font_manager — APPLE (AppKit / macOS) backend partial: builds a real NSFont from a maui::core::font,
// with the registrar alias resolution + weight/slant traits. The NSFont twin of FontManager.iOS.cs
// (CreateFont): the iOS oracle is UIFont-based; the structure ports 1:1 to AppKit (NSFontManager's
// availableFontFamilies for the family probe, NSFontDescriptor for the trait attributes, NSFont
// systemFontOfSize:weight: for the .SFUI-* / attributed system font). Compiled as Objective-C++ with ARC
// for the `apple` backend; the manager OWNS each NSFont via a __bridge_retained void* + a CFRelease
// disposer (PROFILE §8). See font_manager.hpp.
//
// DEVIATIONS vs C# FontManager.iOS:
//   * NSFont, not UIFont (the AppKit backend). The family-names list is NSFontManager.availableFontFamilies;
//     the system-font-by-weight is NSFont.systemFontOfSize:weight:; the descriptor is NSFontDescriptor.
//   * ApplyScaling (UIFontMetrics.GetScaledFont for Dynamic Type) has no AppKit analog — AppKit has no
//     UIFontMetrics. AutoScalingEnabled is therefore a no-op here (documented), as the rest of the apple
//     text stack already treats it (apple_conversions/apple_text_ops ignore scaling).

#import <AppKit/AppKit.h>

#include <cctype>
#include <string>
#include <utility>

#include "maui/core/font.hpp"
#include "maui/core/font_manager.hpp"
#include "maui/core/move_only_function.hpp"

namespace
{
    // Retain a (non-nil) NSFont into a manager-owned slot: the void* takes one reference; the disposer
    // CFReleases it. Returned as the (handle, disposer) pair create_font folds into its created_font.
    std::pair<void*, maui::core::move_only_function<void()>> retain_font(NSFont* font)
    {
        void* const retained = (__bridge_retained void*)font; // one owned reference
        return {retained, [retained] { CFRelease(retained); }};
    }

    // Map a maui font_weight to NSFont's system-font weight constant (the AppKit analog of UIFontWeight,
    // used by NSFont systemFontOfSize:weight:). The values match Apple's NSFontWeight* constants.
    NSFontWeight to_ns_font_weight(maui::core::font_weight weight)
    {
        switch (weight)
        {
            case maui::core::font_weight::thin:
                return NSFontWeightThin;
            case maui::core::font_weight::ultralight:
                return NSFontWeightUltraLight;
            case maui::core::font_weight::light:
                return NSFontWeightLight;
            case maui::core::font_weight::regular:
                return NSFontWeightRegular;
            case maui::core::font_weight::medium:
                return NSFontWeightMedium;
            case maui::core::font_weight::semibold:
                return NSFontWeightSemibold;
            case maui::core::font_weight::bold:
                return NSFontWeightBold;
            case maui::core::font_weight::heavy:
                return NSFontWeightHeavy;
            case maui::core::font_weight::black:
                return NSFontWeightBlack;
        }
        return NSFontWeightRegular;
    }

    // C# GetFontAttributes → the symbolic traits for the font's weight/slant, as an NSFontDescriptor trait
    // mask (Bold weight → Bold trait; Italic slant → Italic trait). The non-bold non-regular weight branch
    // (C#'s UIFontTraits Weight/Slant) is folded into the system-font weight path in create_font instead;
    // here we only carry the symbolic Bold/Italic flags an NSFontDescriptor understands.
    NSFontDescriptorSymbolicTraits symbolic_traits(const maui::core::font& value)
    {
        NSFontDescriptorSymbolicTraits traits = 0;
        if (value.weight() == maui::core::font_weight::bold)
        {
            traits |= NSFontDescriptorTraitBold;
        }
        if (value.slant() == maui::core::font_slant::italic)
        {
            traits |= NSFontDescriptorTraitItalic;
        }
        return traits;
    }

    // True when GetFontAttributes would set the CONTINUOUS weight trait — i.e. the weight is neither Regular
    // nor Bold (Bold is carried as the symbolic Bold trait, Regular carries no weight trait).
    bool has_continuous_weight(maui::core::font value)
    {
        return value.weight() != maui::core::font_weight::regular && value.weight() != maui::core::font_weight::bold;
    }

    // C# GetFontAttributes' UIFontTraits.Weight/Slant branch for non-bold non-regular weights: add the
    // continuous NSFontWeightTrait — together with any symbolic Italic/oblique trait — to a descriptor in a
    // SINGLE NSFontTraitsAttribute dict (mirroring C#'s one UIFontAttributes that holds both Traits.Weight
    // and Traits.SymbolicTrait). This is what was DROPPED before — the symbolic-only path rendered
    // medium/semibold/light/… as Regular. NOTE: calling fontDescriptorWithSymbolicTraits: FIRST and then
    // adding the weight trait does NOT work (the symbolic-traits call resets the weight resolution) — both
    // must go into one attributes addition, which is also closest to GetFontAttributes' shape.
    NSFontDescriptor* descriptor_with_attributes(NSFontDescriptor* descriptor, maui::core::font value,
                                                 NSFontDescriptorSymbolicTraits symbolic)
    {
        NSMutableDictionary* const trait_attrs = [NSMutableDictionary dictionary];
        trait_attrs[NSFontWeightTrait] = @(to_ns_font_weight(value.weight()));
        if (value.slant() == maui::core::font_slant::oblique)
        {
            trait_attrs[NSFontSlantTrait] = @(30.0); // C#'s GetFontAttributes Slant = 30.0f for oblique
        }
        if (symbolic != 0)
        {
            trait_attrs[NSFontSymbolicTrait] = @(symbolic); // the symbolic Italic flag (Bold is never set here)
        }
        return [descriptor fontDescriptorByAddingAttributes:@{NSFontTraitsAttribute : trait_attrs}];
    }

    // Map a ".SFUI-<Weight>" family's trailing segment to an NSFontWeight, mirroring C#'s
    // family.Split('-').Last() → Enum.TryParse<UIFontWeight>(…, ignoreCase: true) (Regular on no/blank/bad
    // suffix). The enum names match: ultralight/thin/light/regular/medium/semibold/bold/heavy/black.
    NSFontWeight system_weight_from_family_suffix(const std::string& family)
    {
        const auto dash = family.find_last_of('-');
        if (dash == std::string::npos || dash + 1 >= family.size())
        {
            return NSFontWeightRegular;
        }
        std::string suffix = family.substr(dash + 1);
        for (char& c : suffix)
        {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        if (suffix == "ultralight")
        {
            return NSFontWeightUltraLight;
        }
        if (suffix == "thin")
        {
            return NSFontWeightThin;
        }
        if (suffix == "light")
        {
            return NSFontWeightLight;
        }
        if (suffix == "regular")
        {
            return NSFontWeightRegular;
        }
        if (suffix == "medium")
        {
            return NSFontWeightMedium;
        }
        if (suffix == "semibold")
        {
            return NSFontWeightSemibold;
        }
        if (suffix == "bold")
        {
            return NSFontWeightBold;
        }
        if (suffix == "heavy")
        {
            return NSFontWeightHeavy;
        }
        if (suffix == "black")
        {
            return NSFontWeightBlack;
        }
        return NSFontWeightRegular; // Enum.TryParse failure → Regular
    }

    bool family_is_available(NSString* family)
    {
        return [NSFontManager.sharedFontManager.availableFontFamilies containsObject:family];
    }
} // namespace

namespace maui::core
{
    // C# DefaultFontSize => UIFont.SystemFontSize; the AppKit analog is NSFont.systemFontSize.
    double font_manager::default_font_size() const
    {
        return static_cast<double>(NSFont.systemFontSize);
    }

    // C# DefaultFont => UIFont.SystemFontOfSize(UIFont.SystemFontSize) (lazily cached). The manager owns it.
    void* font_manager::default_font()
    {
        if (default_font_ == nullptr)
        {
            NSFont* const system = [NSFont systemFontOfSize:NSFont.systemFontSize];
            void* const retained = (__bridge_retained void*)system;
            default_font_ = retained;
            default_dispose_ = [retained] { CFRelease(retained); };
        }
        return default_font_;
    }

    // C# FontManager.CreateFont (translated to NSFont): try the family (an installed family name → a
    // descriptor with the trait attributes; a ".SFUI-*" system family → the system font by weight; a
    // cleansed/raw name via fontWithName), then fall back to the attributed/plain system font.
    font_manager::created_font font_manager::create_font(const font& value) const
    {
        // Fold a resolved NSFont into the manager-owned created_font (retain + CFRelease disposer).
        const auto make = [](NSFont* font) {
            auto [handle, dispose] = retain_font(font);
            return created_font{.handle = handle, .dispose = std::move(dispose)};
        };

        const auto size = static_cast<CGFloat>(value.size());
        const bool has_attributes = value.weight() != font_weight::regular || value.slant() != font_slant::normal;

        NSFont* result = nil;
        const std::string& family = value.family();
        NSString* const default_family = [NSFont systemFontOfSize:NSFont.systemFontSize].familyName;
        NSString* const ns_family = family.empty() ? nil : [NSString stringWithUTF8String:family.c_str()];

        // C#: family != null && family != DefaultFont.FamilyName
        if (ns_family != nil && ![ns_family isEqualToString:default_family])
        {
            // An installed family → a descriptor (with the trait attributes when the font has weight/slant).
            // C#'s descriptor.CreateWithAttributes(GetFontAttributes(font)): the symbolic Bold/Italic flags
            // PLUS the continuous weight trait for non-bold non-regular weights (which was dropped before —
            // medium/semibold/light rendered as Regular).
            if (family_is_available(ns_family))
            {
                NSFontDescriptor* descriptor =
                    [NSFontDescriptor fontDescriptorWithFontAttributes:@{NSFontFamilyAttribute : ns_family}];
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
                result = [NSFont fontWithDescriptor:descriptor size:size];
                if (result != nil)
                {
                    return make(result);
                }
            }

            // ".SFUI-*" — the system font, by the suffix weight if it parses, else Regular. C# parses
            // family.Split('-').Last() → Enum.TryParse<UIFontWeight> (FontManager.iOS.cs:134-151) and
            // requests SystemFontOfSize(size, parsedWeight) — driven by the family NAME suffix, NOT
            // value.weight(). AppKit's system font is requested by NSFontWeight.
            if ([ns_family.lowercaseString hasPrefix:@".sfui"])
            {
                result = [NSFont systemFontOfSize:size weight:system_weight_from_family_suffix(family)];
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
                NSFont* const by_cleansed = ns_cleansed != nil ? [NSFont fontWithName:ns_cleansed size:size] : nil;
                if (by_cleansed != nil)
                {
                    return make(by_cleansed);
                }
            }
            NSFont* const by_family = [NSFont fontWithName:ns_family size:size];
            if (by_family != nil)
            {
                return make(by_family);
            }
        }

        // No family (or it failed to resolve): the system font, with the weight/slant traits applied via a
        // descriptor when the font has any (C#'s SystemFontOfSize(size).FontDescriptor.CreateWithAttributes
        // (GetFontAttributes(font))). For non-bold non-regular weights GetFontAttributes sets the continuous
        // weight trait — start from the correctly-weighted system font so the weight is never dropped (the
        // symbolic-only path rendered medium/semibold as Regular), then overlay any symbolic Italic.
        if (has_attributes)
        {
            NSFont* const base = has_continuous_weight(value)
                                     ? [NSFont systemFontOfSize:size weight:to_ns_font_weight(value.weight())]
                                     : [NSFont systemFontOfSize:size];
            const NSFontDescriptorSymbolicTraits symbolic = symbolic_traits(value);
            if (symbolic == 0)
            {
                return make(base); // continuous weight already baked into `base`; no Bold/Italic to add
            }
            NSFontDescriptor* const descriptor = [base.fontDescriptor fontDescriptorWithSymbolicTraits:symbolic];
            NSFont* const traited = [NSFont fontWithDescriptor:descriptor size:size];
            return make(traited != nil ? traited : base);
        }
        return make([NSFont systemFontOfSize:size]);
    }
} // namespace maui::core
