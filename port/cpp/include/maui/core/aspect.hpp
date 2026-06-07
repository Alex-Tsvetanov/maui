#pragma once
// maui::core::aspect  <=  Microsoft.Maui.Aspect
// How an image is scaled to fit its view. Ported from src/Core/src/Primitives/Aspect.cs — member set and
// order match the C# enum (AspectFit, AspectFill, Fill, Center).

#include <cstdint>

namespace maui::core
{
    enum class aspect : std::uint8_t
    {
        aspect_fit = 0,  // scale to fit; may letterbox (empty bars)
        aspect_fill = 1, // scale to fill; may clip
        fill = 2,        // scale to fill exactly; non-uniform in X/Y
        center = 3       // center without scaling
    };
} // namespace maui::core
