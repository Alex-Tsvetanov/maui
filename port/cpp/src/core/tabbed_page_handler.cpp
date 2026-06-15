// tabbed_page_handler — cross-platform part: the shared mapper tables + ctor + the map functions
// (TabbedViewHandler.cs + TabbedPage.Mapper.cs's key set). The platform recipe (the native tab host,
// set_pages / set_current / update_bar) lives in the per-backend partial.

#include "maui/core/tabbed_page_handler.hpp"

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // The shared view_mapper chains first (the generic IView properties reach the tab host), then the
    // TabbedPage keys (TabbedPage.Mapper.cs): the items set, the current page, and the bar styling.
    property_mapper<i_view, tabbed_page_handler>& tabbed_page_handler::mapper()
    {
        static property_mapper<i_view, tabbed_page_handler> table{
            view_mapper(),
            {
                {"items_source", &tabbed_page_handler::map_items_source},
                {"current_page", &tabbed_page_handler::map_current_page},
                {"bar_background_color", &tabbed_page_handler::map_bar},
                {"bar_background", &tabbed_page_handler::map_bar},
                {"bar_text_color", &tabbed_page_handler::map_bar},
                {"selected_tab_color", &tabbed_page_handler::map_bar},
                {"unselected_tab_color", &tabbed_page_handler::map_bar},
            },
        };
        return table;
    }

    // No tab commands (C#'s TabbedViewHandler.CommandMapper carries only the inherited view commands).
    // The type must be qualified inside the body: the method name shadows the template.
    maui::core::command_mapper<i_view, tabbed_page_handler>& tabbed_page_handler::command_mapper()
    {
        static maui::core::command_mapper<i_view, tabbed_page_handler> table{};
        return table;
    }

    tabbed_page_handler::tabbed_page_handler() : view_handler(&mapper(), &command_mapper())
    {
    }

    // C# MapItemsSource (the control refreshes this key on every PagesChanged and child-Title change):
    // rebuild the tab set, then re-apply the selection and the bar styling (a rebuilt tab set forgets
    // both natively).
    void tabbed_page_handler::map_items_source(tabbed_page_handler& handler, i_view& view)
    {
        handler.set_pages(view);
        handler.set_current(view);
        handler.update_bar(view);
    }

    // C# MapCurrentPage: select the tab matching the view's current page.
    void tabbed_page_handler::map_current_page(tabbed_page_handler& handler, i_view& view)
    {
        handler.set_current(view);
    }

    // C# MapBarBackgroundColor / MapBarTextColor / MapSelectedTabColor / MapUnselectedTabColor — all
    // re-apply the whole bar styling.
    void tabbed_page_handler::map_bar(tabbed_page_handler& handler, i_view& view)
    {
        handler.update_bar(view);
    }
} // namespace maui::core
