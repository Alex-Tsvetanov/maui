#pragma once
// maui::core::keyboard_flags  <=  Microsoft.Maui.KeyboardFlags
//
// Option flags controlling capitalization, spellcheck, and suggestion behavior for a custom keyboard.
// A [Flags] enum in C#; the bitwise operators below give the same `&`/`|` surface. Ported verbatim from
// src/Core/src/Primitives/KeyboardFlags.cs — the bit positions (and the `all = ~0` / `capitalize_none =
// 1 << 5` values) are part of the contract that maui::core::keyboard::create(flags) and the iOS
// KeyboardExtensions port read, so they must match the oracle exactly.
//
// The underlying type is the (signed) int C# uses, because `All = ~0` is -1; bit_cast keeps the combined
// values (none of which match a single enumerator) clear of the analyzer's enum-range check.

#include <bit>
#include <cstdint>

namespace maui::core
{
    enum class keyboard_flags : std::int32_t
    {
        // Nothing is automatically capitalized.
        none = 0,
        // Capitalize the first letter of the first word of each sentence.
        capitalize_sentence = 1,
        // Perform spellcheck on entered text.
        spellcheck = 1 << 1,
        // Offer suggested word completions on entered text.
        suggestions = 1 << 2,
        // Capitalize the first letter of each word.
        capitalize_word = 1 << 3,
        // Capitalize every character.
        capitalize_character = 1 << 4,
        // Nothing is automatically capitalized (an explicit no-capitalization marker).
        capitalize_none = 1 << 5,
        // Capitalize sentences, spellcheck, and suggest completions.
        all = ~0
    };

    // The flag operators combine through std::bit_cast (the port-wide [Flags] idiom, cf. font_attributes):
    // a combined value like (capitalize_sentence | spellcheck) is a valid value of the fixed-underlying-type
    // enum but matches no single enumerator, which a plain integral cast would trip the analyzer on.
    [[nodiscard]] constexpr keyboard_flags operator|(keyboard_flags a, keyboard_flags b)
    {
        return std::bit_cast<keyboard_flags>(
            static_cast<std::int32_t>(static_cast<std::int32_t>(a) | static_cast<std::int32_t>(b)));
    }
    [[nodiscard]] constexpr keyboard_flags operator&(keyboard_flags a, keyboard_flags b)
    {
        return std::bit_cast<keyboard_flags>(
            static_cast<std::int32_t>(static_cast<std::int32_t>(a) & static_cast<std::int32_t>(b)));
    }
    constexpr keyboard_flags& operator|=(keyboard_flags& a, keyboard_flags b)
    {
        a = a | b;
        return a;
    }

    // Whether `value` carries every bit of `flag` (the C# `(flags & X) == X` test, the idiom every
    // KeyboardExtensions branch uses).
    [[nodiscard]] constexpr bool has_flag(keyboard_flags value, keyboard_flags flag)
    {
        return (value & flag) == flag;
    }
} // namespace maui::core
