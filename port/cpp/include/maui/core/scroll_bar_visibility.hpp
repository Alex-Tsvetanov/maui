#pragma once
// maui::core::scroll_bar_visibility  <=  Microsoft.Maui.ScrollBarVisibility
// When a scroll bar is visible. Ported from src/Core/src/Primitives/ScrollBarVisibility.cs
// (Default = 0 / Always / Never; `default` is a C++ keyword, hence default_ — the return_type.hpp
// convention).

#include <cstdint>

namespace maui::core
{
    enum class scroll_bar_visibility : std::uint8_t
    {
        default_ = 0, // the platform default for the content/orientation
        always,       // always visible
        never         // never visible
    };
} // namespace maui::core
