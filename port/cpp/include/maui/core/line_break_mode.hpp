#pragma once
// maui::core::line_break_mode  <=  Microsoft.Maui.LineBreakMode
// How a label's text is wrapped or truncated when it overflows. Ported from
// src/Core/src/Primitives/LineBreakMode.cs (the C# enum order is preserved so the underlying values
// match: NoWrap=0, WordWrap=1, CharacterWrap=2, HeadTruncation=3, TailTruncation=4, MiddleTruncation=5).

#include <cstdint>

namespace maui::core
{
    enum class line_break_mode : std::uint8_t
    {
        no_wrap = 0,
        word_wrap = 1,
        character_wrap = 2,
        head_truncation = 3,
        tail_truncation = 4,
        middle_truncation = 5
    };
} // namespace maui::core
