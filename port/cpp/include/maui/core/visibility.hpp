#pragma once
// maui::core::visibility  <=  Microsoft.Maui.Visibility
// Whether a view participates in the visual tree. Ported from src/Core/src/Primitives/Visibility.cs.

#include <cstdint>

namespace maui::core
{
    enum class visibility : std::uint8_t
    {
        visible = 0,
        hidden = 1,
        collapsed = 2,
    };
} // namespace maui::core
