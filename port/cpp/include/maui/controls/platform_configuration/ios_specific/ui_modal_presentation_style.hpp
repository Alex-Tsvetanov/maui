#pragma once
// maui::controls::platform_configuration::ios_specific::ui_modal_presentation_style
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.UIModalPresentationStyle
// Ported from src/Controls/src/Core/PlatformConfiguration/iOSSpecific/UIModalPresentationStyle.cs.

#include <cstdint>

namespace maui::controls::platform_configuration::ios_specific
{
    enum class ui_modal_presentation_style : std::uint8_t
    {
        full_screen = 0,
        form_sheet = 1,
        automatic = 2,
        over_full_screen = 3,
        page_sheet = 4,
        popover = 5,
    };
} // namespace maui::controls::platform_configuration::ios_specific
