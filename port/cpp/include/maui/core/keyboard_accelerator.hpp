#pragma once
// maui::core::keyboard_accelerator  <=  Microsoft.Maui.IKeyboardAccelerator (+ Controls.KeyboardAccelerator)
// maui::core::keyboard_accelerator_modifiers  <=  Microsoft.Maui.KeyboardAcceleratorModifiers
//
// The shortcut-key descriptor a menu_flyout_item carries (MenuFlyoutItem.KeyboardAccelerators). C# splits
// the contract (IKeyboardAccelerator, Core) from the bindable concrete (KeyboardAccelerator, Controls);
// the port collapses both into one small value struct — the two C# properties (Modifiers / Key) carry no
// behavior, so a bindable wrapper would add nothing the tests observe (documented simplification).
// The modifier FLAGS port KeyboardAcceleratorModifiers.cs 1:1 (same values; [Flags] → bitwise operators).

#include <string>

namespace maui::core
{
    enum class keyboard_accelerator_modifiers : unsigned
    {
        none = 0,
        shift = 1U << 0U,   // Shift on both MacCatalyst and Windows
        ctrl = 1U << 1U,    // Control on both
        alt = 1U << 2U,     // Option on MacCatalyst, Menu on Windows
        cmd = 1U << 3U,     // Command, MacCatalyst only
        windows = 1U << 4U, // Windows key, Windows only
    };

    [[nodiscard]] constexpr keyboard_accelerator_modifiers operator|(keyboard_accelerator_modifiers lhs,
                                                                     keyboard_accelerator_modifiers rhs)
    {
        return static_cast<keyboard_accelerator_modifiers>(static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs));
    }

    [[nodiscard]] constexpr bool has_modifier(keyboard_accelerator_modifiers value, keyboard_accelerator_modifiers flag)
    {
        return (static_cast<unsigned>(value) & static_cast<unsigned>(flag)) != 0U;
    }

    struct keyboard_accelerator
    {
        keyboard_accelerator_modifiers modifiers = keyboard_accelerator_modifiers::none;
        std::string key; // the (single-character) key, e.g. "A" (C# Key; null → empty here)
    };
} // namespace maui::core
