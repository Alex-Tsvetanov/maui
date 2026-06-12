#pragma once
// maui::core::path_aspect  <=  Microsoft.Maui.PathAspect
// How a shape's path is stretched to fill the view's layout space. Ported from
// src/Core/src/PathAspect.cs.

#include <cstdint>

namespace maui::core
{
    enum class path_aspect : std::uint8_t
    {
        none = 0,
        center,
        stretch,
        aspect_fit,
        aspect_fill,
    };
} // namespace maui::core
