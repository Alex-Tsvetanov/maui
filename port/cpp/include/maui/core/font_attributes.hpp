#pragma once
// maui::core::font_attributes  <=  Microsoft.Maui.Controls.FontAttributes
//
// Font style flags (bold / italic). A [Flags] enum in C#; the bitwise operators below give the same
// `&`/`|` surface. Ported from src/Controls/src/Core/FontAttributes.cs.
//
// The port's maui::core::font already collapses these into a font_weight + font_slant; this enum is the
// IFontElement-level raw value that maui::controls::span carries per the C# Span surface (FontElement
// stores FontAttributes; FontExtensions.WithAttributes folds it into Font.Weight/Slant). The
// from_font / to_attributes helpers reproduce FontExtensions.WithAttributes / GetFontAttributes so a
// span can synthesize its effective font.

#include <bit>
#include <cstdint>

#include "maui/core/font.hpp"

namespace maui::core
{
    enum class font_attributes : std::uint8_t
    {
        none = 0,
        bold = 1U << 0U,
        italic = 1U << 1U
    };

    // The flag operators combine through std::bit_cast (the port-wide idiom, cf. buttons_mask): bold|italic
    // (3) is a valid value of the fixed-underlying-type enum but matches no single enumerator, which an
    // integral cast would trip the analyzer's enum-range check on.
    [[nodiscard]] constexpr font_attributes operator|(font_attributes a, font_attributes b)
    {
        return std::bit_cast<font_attributes>(
            static_cast<std::uint8_t>(static_cast<std::uint8_t>(a) | static_cast<std::uint8_t>(b)));
    }
    [[nodiscard]] constexpr font_attributes operator&(font_attributes a, font_attributes b)
    {
        return std::bit_cast<font_attributes>(
            static_cast<std::uint8_t>(static_cast<std::uint8_t>(a) & static_cast<std::uint8_t>(b)));
    }
    constexpr font_attributes& operator|=(font_attributes& a, font_attributes b)
    {
        a = a | b;
        return a;
    }

    // C# FontExtensions.WithAttributes — fold bold/italic onto a font's weight (Bold/Regular) and slant
    // (Italic/Default). Used by span::get_effective_font (the GetEffectiveFont port).
    [[nodiscard]] font with_attributes(const font& base, font_attributes attributes);

    // C# FontExtensions.GetFontAttributes — recover the flags from a font's weight + slant (Weight == Bold
    // => Bold; Slant != normal => Italic). Used to inherit a default font's attributes when a span leaves
    // its FontAttributes unset.
    [[nodiscard]] font_attributes attributes_of(const font& value);
} // namespace maui::core
