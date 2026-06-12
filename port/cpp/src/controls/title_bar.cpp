// maui::controls::title_bar — the shared bindable-property descriptors. See title_bar.hpp; ported
// from src/Controls/src/Core/TitleBar/TitleBar.cs (the Title/Subtitle basics).

#include "maui/controls/title_bar.hpp"

#include <string>

#include "maui/core/bindable_property.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<std::string>& title_bar::title_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"title"};
        return descriptor;
    }

    const maui::core::bindable_property<std::string>& title_bar::subtitle_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"subtitle"};
        return descriptor;
    }
} // namespace maui::controls
