#pragma once
// maui::core::clear_button_visibility  <=  Microsoft.Maui.ClearButtonVisibility
// Controls the in-field clear button. Ported from src/Core/src/Primitives/ClearButtonVisibility.cs.

#include <cstdint>

namespace maui::core
{
    enum class clear_button_visibility : std::uint8_t
    {
        never = 0,        // never show a clear button
        while_editing = 1 // show a clear button while the field is focused/being edited
    };
} // namespace maui::core
