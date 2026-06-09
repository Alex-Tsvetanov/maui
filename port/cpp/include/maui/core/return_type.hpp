#pragma once
// maui::core::return_type  <=  Microsoft.Maui.ReturnType
// The style of the keyboard return/submit key. Ported from src/Core/src/Primitives/ReturnType.cs.

#include <cstdint>

namespace maui::core
{
    enum class return_type : std::uint8_t
    {
        default_ = 0, // C# ReturnType.Default ("default" is a C++ keyword)
        done = 1,
        go = 2,
        next = 3,
        search = 4,
        send = 5,
    };
} // namespace maui::core
