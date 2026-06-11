#pragma once
// maui::core::scroll_to_request  <=  Microsoft.Maui.ScrollToRequest
// The payload of the IScrollView.RequestScrollTo command: the target offsets plus the instant flag
// (instant = NOT animated). Ported from src/Core/src/Primitives/ScrollToRequest.cs (a record carried
// through CommandMapper args — here a std::any payload, like navigation_request).

namespace maui::core
{
    struct scroll_to_request
    {
        double horizontal_offset = 0;
        double vertical_offset = 0;
        bool instant = false;
    };
} // namespace maui::core
