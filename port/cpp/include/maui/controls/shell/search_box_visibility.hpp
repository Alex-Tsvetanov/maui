#pragma once
// maui::controls::search_box_visibility  <=  Microsoft.Maui.Controls.SearchBoxVisibility
//
// The visibility behavior of a search_handler's search box in the shell chrome. Ported 1:1 from
// src/Controls/src/Core/SearchBoxVisibility.cs. The enumerator ORDER is preserved (hidden=0,
// collapsible=1, expanded=2) because the C# default is SearchBoxVisibility.Expanded — see
// SearchHandler.SearchBoxVisibilityProperty's default.

#include <cstdint>

namespace maui::controls
{
    enum class search_box_visibility : std::uint8_t
    {
        hidden = 0,      // the search box is hidden
        collapsible = 1, // the search box can be collapsed and expanded by the user
        expanded = 2     // the search box is always expanded and visible
    };
} // namespace maui::controls
