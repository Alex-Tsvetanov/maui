// Tests for the maui::core::keyboard value type + keyboard_flags. Derived from the C# behavior in
// src/Core/src/Primitives/{Keyboard,KeyboardFlags,CustomKeyboard}.cs and the named-instance identity the
// iOS KeyboardExtensions switch relies on (each named keyboard is distinct; a custom keyboard equals
// another only when its flags match).
#include "maui/core/keyboard.hpp"

#include "maui/core/keyboard_flags.hpp"

#include <gtest/gtest.h>

namespace
{
    using maui::core::has_flag;
    using maui::core::keyboard;
    using maui::core::keyboard_flags;

    // KeyboardFlags bit values match KeyboardFlags.cs exactly (the contract the iOS map reads).
    TEST(keyboard_flags, bit_values_match_oracle)
    {
        EXPECT_EQ(static_cast<int>(keyboard_flags::none), 0);
        EXPECT_EQ(static_cast<int>(keyboard_flags::capitalize_sentence), 1);
        EXPECT_EQ(static_cast<int>(keyboard_flags::spellcheck), 1 << 1);
        EXPECT_EQ(static_cast<int>(keyboard_flags::suggestions), 1 << 2);
        EXPECT_EQ(static_cast<int>(keyboard_flags::capitalize_word), 1 << 3);
        EXPECT_EQ(static_cast<int>(keyboard_flags::capitalize_character), 1 << 4);
        EXPECT_EQ(static_cast<int>(keyboard_flags::capitalize_none), 1 << 5);
        EXPECT_EQ(static_cast<int>(keyboard_flags::all), ~0);
    }

    TEST(keyboard_flags, bitwise_combine_and_has_flag)
    {
        const keyboard_flags combined = keyboard_flags::capitalize_sentence | keyboard_flags::spellcheck;
        EXPECT_TRUE(has_flag(combined, keyboard_flags::capitalize_sentence));
        EXPECT_TRUE(has_flag(combined, keyboard_flags::spellcheck));
        EXPECT_FALSE(has_flag(combined, keyboard_flags::suggestions));
        // All carries every named flag (~0 has all bits set).
        EXPECT_TRUE(has_flag(keyboard_flags::all, keyboard_flags::capitalize_sentence));
        EXPECT_TRUE(has_flag(keyboard_flags::all, keyboard_flags::suggestions));
    }

    TEST(keyboard, default_is_default_kind)
    {
        const keyboard k;
        EXPECT_EQ(k.kind(), keyboard::kind::default_);
        EXPECT_EQ(k, keyboard::default_keyboard());
    }

    // Each named keyboard is a distinct value (C#'s singleton reference identity) — the iOS switch depends
    // on Email != Numeric != Default, etc.
    TEST(keyboard, named_keyboards_are_distinct)
    {
        EXPECT_NE(keyboard::default_keyboard(), keyboard::email());
        EXPECT_NE(keyboard::email(), keyboard::numeric());
        EXPECT_NE(keyboard::numeric(), keyboard::telephone());
        EXPECT_NE(keyboard::telephone(), keyboard::url());
        EXPECT_NE(keyboard::url(), keyboard::text());
        EXPECT_NE(keyboard::text(), keyboard::chat());
        EXPECT_NE(keyboard::chat(), keyboard::plain());
        EXPECT_NE(keyboard::date(), keyboard::time());
        EXPECT_NE(keyboard::password(), keyboard::default_keyboard());
        // Plain is a distinct identity from both Default and an empty custom keyboard (C# hands out a
        // separate CustomKeyboard singleton for Plain).
        EXPECT_NE(keyboard::plain(), keyboard::default_keyboard());
        EXPECT_NE(keyboard::plain(), keyboard::create(keyboard_flags::none));
    }

    // The same named keyboard accessor always compares equal to itself (a stable identity).
    TEST(keyboard, named_keyboard_equals_itself)
    {
        EXPECT_EQ(keyboard::email(), keyboard::email());
        EXPECT_EQ(keyboard::numeric(), keyboard::numeric());
    }

    // Keyboard.Create(flags) carries the flags on a custom kind; two customs are equal iff flags match.
    TEST(keyboard, create_carries_flags_and_compares_by_flags)
    {
        const keyboard a = keyboard::create(keyboard_flags::capitalize_word | keyboard_flags::spellcheck);
        EXPECT_EQ(a.kind(), keyboard::kind::custom);
        EXPECT_TRUE(has_flag(a.flags(), keyboard_flags::capitalize_word));
        EXPECT_TRUE(has_flag(a.flags(), keyboard_flags::spellcheck));

        const keyboard same = keyboard::create(keyboard_flags::capitalize_word | keyboard_flags::spellcheck);
        EXPECT_EQ(a, same);

        const keyboard different = keyboard::create(keyboard_flags::capitalize_character);
        EXPECT_NE(a, different);
    }

    // A custom keyboard is not equal to any named keyboard (C#: `keyboard is CustomKeyboard` vs the named
    // singletons).
    TEST(keyboard, custom_is_distinct_from_named)
    {
        const keyboard custom = keyboard::create(keyboard_flags::all);
        EXPECT_NE(custom, keyboard::default_keyboard());
        EXPECT_NE(custom, keyboard::email());
        EXPECT_NE(custom, keyboard::text());
    }
} // namespace
