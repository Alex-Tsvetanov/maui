#pragma once
// maui::core::web_navigation_result  <=  Microsoft.Maui.WebNavigationResult
// The outcome of a web navigation. Ported from src/Core/src/Primitives/WebNavigationResult.cs —
// member set, order and VALUES match the C# enum (Success=1, Cancel=2, Timeout=3, Failure=4).

#include <cstdint>

namespace maui::core
{
    enum class web_navigation_result : std::uint8_t
    {
        success = 1, // the navigation succeeded
        cancel = 2,  // the navigation was cancelled
        timeout = 3, // the navigation timed out
        failure = 4  // the navigation failed
    };
} // namespace maui::core
