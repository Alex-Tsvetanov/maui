#pragma once
// maui::core::web_navigation_event  <=  Microsoft.Maui.WebNavigationEvent
// Why a web navigation event was raised. Ported from src/Core/src/Primitives/WebNavigationEvent.cs —
// member set, order and VALUES match the C# enum (Back=1, Forward=2, NewPage=3, Refresh=4).

#include <cstdint>

namespace maui::core
{
    enum class web_navigation_event : std::uint8_t
    {
        back = 1,     // the user went back to a previous page in the navigation history
        forward = 2,  // the user went forward to a later page in the navigation history
        new_page = 3, // navigation to a previously unvisited page
        refresh = 4   // a page refresh
    };
} // namespace maui::core
