#pragma once
// maui::controls::platform_configuration::tizen_specific::progress_bar
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.TizenSpecific.ProgressBar
// Ported from src/Controls/src/Core/PlatformConfiguration/TizenSpecific/ProgressBar.cs. STORED-INERT
// (no Tizen backend). C#'s SetPulsingStatus only stores when the element's Tizen ThemeStyle is
// ProgressBarStyle.Pending (otherwise it is a silent no-op) — ported faithfully.

#include <concepts>
#include <string_view>

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/tizen_specific/style_values.hpp"
#include "maui/controls/platform_configuration/tizen_specific/visual_element.hpp"
#include "maui/controls/progress_bar.hpp"

namespace maui::controls::platform_configuration::tizen_specific::progress_bar
{
    using forms_element = maui::controls::progress_bar; // C# `using FormsElement = ...ProgressBar`

    inline constexpr std::string_view progress_bar_pulsing_status_key = "tizen.ProgressBar.ProgressBarPulsingStatus";

    // ---- ProgressBarPulsingStatus (bool, default false) ----
    [[nodiscard]] inline bool get_pulsing_status(const element& target)
    {
        return target.platform_spec<bool>(progress_bar_pulsing_status_key, false);
    }
    inline void set_pulsing_status(element& target, bool is_pulsing)
    {
        if (visual_element::get_style(target) == progress_bar_style::pending)
        {
            target.set_platform_spec(progress_bar_pulsing_status_key, is_pulsing);
        }
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] bool get_pulsing_status(config<tizen, TElement> cfg)
    {
        return get_pulsing_status(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<tizen, TElement> set_pulsing_status(config<tizen, TElement> cfg, bool is_pulsing)
    {
        set_pulsing_status(cfg.element(), is_pulsing);
        return cfg;
    }
} // namespace maui::controls::platform_configuration::tizen_specific::progress_bar
