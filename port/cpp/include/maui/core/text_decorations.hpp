#pragma once
// maui::core::text_decorations  <=  Microsoft.Maui.TextDecorations
// Underline / strike-through decoration for text (a [Flags] enum in C#). Ported from
// src/Core/src/Primitives/TextDecorations.cs.

#include <cstdint>

namespace maui::core
{
    enum class text_decorations : std::uint8_t
    {
        none = 0,
        underline = 1U << 0U,
        strikethrough = 1U << 1U
    };
} // namespace maui::core
