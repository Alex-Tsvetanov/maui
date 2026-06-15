// maui::controls::tabbed_page — out-of-line definitions: the shared bindable-property descriptors for
// the tab-bar styling and the default-handler self-registration. See tabbed_page.hpp.

#include "maui/controls/tabbed_page.hpp"

#include <memory>

#include "maui/controls/brushes/brush.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/tabbed_page_handler.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    // One descriptor per type (C# TabbedPage.BarBackgroundColorProperty etc. — the BarElement pair plus
    // the tab-color pair). The default is an unset (default-constructed) color; the tabbed_page's
    // *_set_ flags distinguish "never set" so the handler keeps the system default (C# null Color).
    const maui::core::bindable_property<maui::graphics::color>& tabbed_page::bar_background_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"bar_background_color"};
        return descriptor;
    }

    // C# TabbedPage.BarBackgroundProperty (IBarElement.BarBackground, default(Brush) = null): a Brush fill
    // for the bar. The default is a null brush; the bar_background_set_ flag distinguishes "never set" so
    // the handler keeps the color-driven default (the BarBackgroundColor convention).
    const maui::core::bindable_property<std::shared_ptr<brush>>& tabbed_page::bar_background_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<brush>> descriptor{"bar_background"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& tabbed_page::bar_text_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"bar_text_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& tabbed_page::selected_tab_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"selected_tab_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& tabbed_page::unselected_tab_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"unselected_tab_color"};
        return descriptor;
    }
} // namespace maui::controls

// Self-register the default handler for tabbed_page (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::tabbed_page, maui::core::tabbed_page_handler)
