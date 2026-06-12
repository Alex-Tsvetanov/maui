// tabbed_page_handler — headless platform recipe. The "native tab host" is the set of mirrors in
// tabbed_page_platform (hosted_pages / tab_titles / hosted_current / selected_index / the four bar
// colors) so tests can observe that the host tracks the children, the selection, and the bar styling.
// The real twins are src/platform/{ios,apple}/tabbed_page_handler.mm (UITabBarController /
// NSTabViewController).

#include "maui/core/tabbed_page_handler.hpp"

#include <cstddef>
#include <memory>

#include "maui/core/i_tabbed_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    tabbed_page_platform::~tabbed_page_platform() = default;

    std::unique_ptr<tabbed_page_platform> tabbed_page_handler::create_platform_view()
    {
        return std::make_unique<tabbed_page_platform>();
    }

    void tabbed_page_handler::on_connect_handler(tabbed_page_platform& /*platform*/)
    {
        // Headless has no native tab chrome to wire; the native→virtual selection sync is exercised
        // through i_tabbed_view::on_tab_selected directly in the unit tests.
    }

    void tabbed_page_handler::set_pages(i_view& view)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        if (const auto* tabbed = dynamic_cast<i_tabbed_view*>(&view))
        {
            platform->hosted_pages = tabbed->tabbed_pages();
            platform->tab_titles = tabbed->tabbed_titles();
        }
    }

    void tabbed_page_handler::set_current(i_view& view)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        if (const auto* tabbed = dynamic_cast<i_tabbed_view*>(&view))
        {
            platform->hosted_current = tabbed->tabbed_current_page();
            platform->selected_index = -1;
            for (std::size_t i = 0; i < platform->hosted_pages.size(); ++i)
            {
                if (platform->hosted_pages[i] == platform->hosted_current)
                {
                    platform->selected_index = static_cast<int>(i);
                    break;
                }
            }
        }
    }

    void tabbed_page_handler::update_bar(i_view& view)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        if (const auto* tabbed = dynamic_cast<i_tabbed_view*>(&view))
        {
            platform->bar_background_color = tabbed->tab_bar_background_color();
            platform->bar_text_color = tabbed->tab_bar_text_color();
            platform->selected_tab_color = tabbed->tab_selected_color();
            platform->unselected_tab_color = tabbed->tab_unselected_color();
        }
    }

    maui::graphics::size tabbed_page_handler::get_desired_size(double /*width_constraint*/,
                                                               double /*height_constraint*/) const
    {
        // The tabbed page sizes from its current page, not the handler, so it reports nothing here.
        return {0, 0};
    }

    void tabbed_page_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native tab host to position.
    }
} // namespace maui::core
