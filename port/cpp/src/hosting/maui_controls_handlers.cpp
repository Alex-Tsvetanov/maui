// maui::hosting::add_maui_controls_handlers — the default control → handler table
// (maui_controls_handlers.hpp). Ported from Microsoft.Maui.Controls.Hosting.AppHostBuilderExtensions
// .AddControlsHandlers, narrowed to the port's v1 control set: the same pairs the controls'
// MAUI_REGISTER_HANDLER registrars publish (src/controls/*.cpp), listed once per control here.

#include "maui/hosting/maui_controls_handlers.hpp"

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/flyout_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/navigation_page.hpp"
#include "maui/controls/tabbed_page.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/entry_handler.hpp"
#include "maui/core/flyout_page_handler.hpp"
#include "maui/core/image_handler.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/navigation_page_handler.hpp"
#include "maui/core/tabbed_page_handler.hpp"
#include "maui/core/window_handler.hpp"
#include "maui/hosting/i_maui_handlers_collection.hpp"

namespace maui::hosting
{
    i_maui_handlers_collection& add_maui_controls_handlers(i_maui_handlers_collection& handlers)
    {
        handlers.add_handler<maui::controls::button, maui::core::button_handler>();
        handlers.add_handler<maui::controls::label, maui::core::label_handler>();
        handlers.add_handler<maui::controls::entry, maui::core::entry_handler>();
        handlers.add_handler<maui::controls::image, maui::core::image_handler>();
        // The three layout controls share the one layout_handler (their managers differ, not the panel).
        handlers.add_handler<maui::controls::vertical_stack_layout, maui::core::layout_handler>();
        handlers.add_handler<maui::controls::horizontal_stack_layout, maui::core::layout_handler>();
        handlers.add_handler<maui::controls::grid, maui::core::layout_handler>();
        handlers.add_handler<maui::controls::content_page, maui::core::content_page_handler>();
        handlers.add_handler<maui::controls::navigation_page, maui::core::navigation_page_handler>();
        // (W1-10) the tabbed + flyout page hosts.
        handlers.add_handler<maui::controls::tabbed_page, maui::core::tabbed_page_handler>();
        handlers.add_handler<maui::controls::flyout_page, maui::core::flyout_page_handler>();
        handlers.add_handler<maui::controls::window, maui::core::window_handler>();
        return handlers;
    }
} // namespace maui::hosting
