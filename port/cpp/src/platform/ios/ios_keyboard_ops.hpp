#pragma once
// Shared iOS keyboard-trait application — the port of Microsoft.Maui.Platform.KeyboardExtensions
// (src/Core/src/Platform/iOS/KeyboardExtensions.cs ApplyKeyboard) + the Done input-accessory toolbar
// (MauiDoneAccessoryView, via ios_done_accessory.hpp). Objective-C++ only — include exclusively from .mm
// files compiled as Objective-C++.
//
// apply_keyboard maps a maui::core::keyboard onto a control conforming to UITextInputTraits (UITextField,
// UITextView, UISearchBar's text field): it resets the four traits to their no-op defaults, then applies
// the per-keyboard overrides EXACTLY as the C# switch does — Chat/Default/Text force
// Sentences+autocorrect+spellcheck (so they behave identically on iOS), Email/Numeric/Telephone/Url pick
// a UIKeyboardType, Date/Time use NumbersAndPunctuation + the DateTime content type, Password sets
// SecureTextEntry, and a custom keyboard reads its KeyboardFlags for capitalization / spellcheck /
// suggestions. The C# value-identity comparisons (`keyboard == Keyboard.Email`) become value equality on
// the ported keyboard type (its `kind` discriminator), which is behavior-identical (see keyboard.hpp).

#import <UIKit/UIKit.h>

#include "maui/core/keyboard.hpp"
#include "maui/core/keyboard_flags.hpp"

namespace maui::platform::ios
{
    // KeyboardExtensions.ApplyKeyboard(IUITextInputTraits, Keyboard). `traits` is a UIView<UITextInputTraits>
    // (UITextField / UITextView / a UISearchBar's search field). Mutates the trait properties in place.
    inline void apply_keyboard(id<UITextInputTraits> traits, maui::core::keyboard keyboard)
    {
        using maui::core::has_flag;
        using maui::core::keyboard_flags;

        // The four no-op defaults (C# resets them before the per-keyboard overrides).
        traits.autocapitalizationType = UITextAutocapitalizationTypeNone;
        traits.autocorrectionType = UITextAutocorrectionTypeNo;
        traits.spellCheckingType = UITextSpellCheckingTypeNo;
        traits.keyboardType = UIKeyboardTypeDefault;

        switch (keyboard.kind())
        {
            case maui::core::keyboard::kind::chat:
            case maui::core::keyboard::kind::default_:
            case maui::core::keyboard::kind::text:
                // "chat, default, and text keyboards are the same thing on iOS" — force the on-by-default
                // autocorrect/spellcheck and sentence capitalization.
                traits.autocapitalizationType = UITextAutocapitalizationTypeSentences;
                traits.autocorrectionType = UITextAutocorrectionTypeYes;
                traits.spellCheckingType = UITextSpellCheckingTypeYes;
                break;
            case maui::core::keyboard::kind::email:
                traits.keyboardType = UIKeyboardTypeEmailAddress;
                break;
            case maui::core::keyboard::kind::numeric:
                traits.keyboardType = UIKeyboardTypeDecimalPad;
                break;
            case maui::core::keyboard::kind::telephone:
                traits.keyboardType = UIKeyboardTypePhonePad;
                break;
            case maui::core::keyboard::kind::url:
                traits.keyboardType = UIKeyboardTypeURL;
                break;
            case maui::core::keyboard::kind::date:
            case maui::core::keyboard::kind::time:
                // The DateTime content type + the numbers-and-punctuation keyboard (textContentType lives
                // on UITextField/UITextView; set it when supported).
                if ([traits respondsToSelector:@selector(setTextContentType:)])
                {
                    [(id)traits setTextContentType:UITextContentTypeDateTime];
                }
                traits.keyboardType = UIKeyboardTypeNumbersAndPunctuation;
                break;
            case maui::core::keyboard::kind::password:
                traits.keyboardType = UIKeyboardTypeDefault;
                if ([traits respondsToSelector:@selector(setSecureTextEntry:)])
                {
                    [(id)traits setSecureTextEntry:YES];
                }
                break;
            case maui::core::keyboard::kind::plain:
                // Keyboard.Plain is Create(KeyboardFlags.None): the no-op defaults above already match.
                break;
            case maui::core::keyboard::kind::custom: {
                const keyboard_flags flags = keyboard.flags();
                // Sentence first so KeyboardFlags.All stays backwards-compatible (the C# ordering).
                UITextAutocapitalizationType cap = UITextAutocapitalizationTypeNone;
                if (has_flag(flags, keyboard_flags::capitalize_sentence))
                {
                    cap = UITextAutocapitalizationTypeSentences;
                }
                else if (has_flag(flags, keyboard_flags::capitalize_word))
                {
                    cap = UITextAutocapitalizationTypeWords;
                }
                else if (has_flag(flags, keyboard_flags::capitalize_character))
                {
                    cap = UITextAutocapitalizationTypeAllCharacters;
                }
                else if (has_flag(flags, keyboard_flags::none))
                {
                    cap = UITextAutocapitalizationTypeNone;
                }
                traits.autocapitalizationType = cap;
                traits.autocorrectionType = has_flag(flags, keyboard_flags::suggestions) ? UITextAutocorrectionTypeYes
                                                                                         : UITextAutocorrectionTypeNo;
                traits.spellCheckingType = has_flag(flags, keyboard_flags::spellcheck) ? UITextSpellCheckingTypeYes
                                                                                       : UITextSpellCheckingTypeNo;
                break;
            }
        }
    }

    // Whether the keyboard is a custom (flags-carrying) keyboard — used by UpdateKeyboard to decide whether
    // to additionally re-apply the IsTextPredictionEnabled / IsSpellCheckEnabled property pushes (C# does
    // this only for non-custom keyboards, since a custom keyboard's flags already drive those traits).
    [[nodiscard]] inline bool is_custom_keyboard(maui::core::keyboard keyboard)
    {
        return keyboard.kind() == maui::core::keyboard::kind::custom;
    }
} // namespace maui::platform::ios
