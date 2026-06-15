#pragma once
// maui::core::keyboard  <=  Microsoft.Maui.Keyboard
//
// The keyboard input type for a text-input view (Default and the specialized chat/email/numeric/
// telephone/text/url/date/time/password keyboards, plus a custom keyboard carrying KeyboardFlags).
// Ported from src/Core/src/Primitives/Keyboard.cs + its sibling subclasses (CustomKeyboard.cs,
// ChatKeyboard.cs, EmailKeyboard.cs, NumericKeyboard.cs, TelephoneKeyboard.cs, TextKeyboard.cs,
// UrlKeyboard.cs, DateTimeKeyboard.cs) + KeyboardFlags.cs.
//
// MODELING DECISION (faithful, not a deviation). C# uses a class hierarchy where each named keyboard is
// a distinct lazily-created SINGLETON and the platform code distinguishes them by REFERENCE identity
// (`keyboard == Keyboard.Email`) or by RTTI (`keyboard is CustomKeyboard`) — see
// src/Core/src/Platform/iOS/KeyboardExtensions.cs. C++23 has no reflection and a value-type port should
// not heap-allocate singletons just to compare pointers, so this collapses the hierarchy into a small
// value type: a `kind` discriminator (one enumerator per named keyboard, with `custom` for
// Keyboard.Create(flags)) plus the KeyboardFlags (meaningful only for the custom kind). Value equality on
// (kind, flags) reproduces C#'s identity comparisons EXACTLY — the named singletons each compare unequal
// to every other, and a CustomKeyboard equals another only when its flags match — so the ported iOS
// KeyboardExtensions switch behaves identically. The named accessors mirror the C# static properties
// (Keyboard.Default, Keyboard.Email, …); create() mirrors Keyboard.Create(KeyboardFlags).
//
// The XAML KeyboardTypeConverter (src/Core/src/Converters/KeyboardTypeConverter.cs) is layer-6 (XAML,
// deferred) and is intentionally NOT ported here.

#include "maui/core/keyboard_flags.hpp"

namespace maui::core
{
    class keyboard
    {
    public:
        // One discriminator per C# named keyboard, plus `custom` for Keyboard.Create(flags). The default
        // member-default is `default_` (Keyboard.Default), matching InputView.KeyboardProperty's default.
        enum class kind
        {
            default_,
            plain,
            chat,
            email,
            numeric,
            telephone,
            text,
            url,
            date,
            time,
            password,
            custom
        };

        // Default-constructs to Keyboard.Default (the InputView default). Aggregate-free so the invariant
        // "flags are only meaningful when kind == custom" stays under the type's control.
        constexpr keyboard() = default;

        [[nodiscard]] constexpr enum kind kind() const
        {
            return kind_;
        }
        // The KeyboardFlags — meaningful only for the custom kind (the named keyboards leave it `none`,
        // mirroring how only CustomKeyboard carries Flags in C#).
        [[nodiscard]] constexpr keyboard_flags flags() const
        {
            return flags_;
        }

        // Value equality reproduces C#'s reference/RTTI identity comparisons (see the header note): two
        // keyboards are equal iff their kind matches, and — for the custom kind — their flags also match.
        [[nodiscard]] constexpr bool operator==(const keyboard& other) const = default;

        // The named keyboards — the C# static properties (Keyboard.Default, .Plain, .Chat, .Email,
        // .Numeric, .Telephone, .Text, .Url, .Date, .Time, .Password). Plain is `Create(KeyboardFlags.None)`
        // in C# (a CustomKeyboard with no flags); it is given its own kind here so it compares distinct from
        // both Default and an empty custom keyboard, matching the singleton identity C# hands out.
        [[nodiscard]] static constexpr keyboard default_keyboard()
        {
            return keyboard{kind::default_, keyboard_flags::none};
        }
        [[nodiscard]] static constexpr keyboard plain()
        {
            return keyboard{kind::plain, keyboard_flags::none};
        }
        [[nodiscard]] static constexpr keyboard chat()
        {
            return keyboard{kind::chat, keyboard_flags::none};
        }
        [[nodiscard]] static constexpr keyboard email()
        {
            return keyboard{kind::email, keyboard_flags::none};
        }
        [[nodiscard]] static constexpr keyboard numeric()
        {
            return keyboard{kind::numeric, keyboard_flags::none};
        }
        [[nodiscard]] static constexpr keyboard telephone()
        {
            return keyboard{kind::telephone, keyboard_flags::none};
        }
        [[nodiscard]] static constexpr keyboard text()
        {
            return keyboard{kind::text, keyboard_flags::none};
        }
        [[nodiscard]] static constexpr keyboard url()
        {
            return keyboard{kind::url, keyboard_flags::none};
        }
        [[nodiscard]] static constexpr keyboard date()
        {
            return keyboard{kind::date, keyboard_flags::none};
        }
        [[nodiscard]] static constexpr keyboard time()
        {
            return keyboard{kind::time, keyboard_flags::none};
        }
        [[nodiscard]] static constexpr keyboard password()
        {
            return keyboard{kind::password, keyboard_flags::none};
        }

        // Keyboard.Create(KeyboardFlags): a custom keyboard carrying the given flags (CustomKeyboard).
        [[nodiscard]] static constexpr keyboard create(keyboard_flags flags)
        {
            return keyboard{kind::custom, flags};
        }

    private:
        constexpr keyboard(enum kind k, keyboard_flags f) : kind_(k), flags_(f)
        {
        }

        enum kind kind_ = kind::default_;
        keyboard_flags flags_ = keyboard_flags::none;
    };
} // namespace maui::core
