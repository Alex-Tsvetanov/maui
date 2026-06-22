// maui::controls::tabbed_page — out-of-line definitions: the shared bindable-property descriptors for
// the tab-bar styling and the default-handler self-registration. See tabbed_page.hpp.

#include "maui/controls/tabbed_page.hpp"

#include <memory>

#include "maui/controls/brushes/brush.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/tabbed_page_handler.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

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

    // C# TabbedPage measure: the tab host fills its constraint (the native UITabBarController/UITabBar own
    // the chrome + content split); the page reports the constraint as its desired size, like the other
    // container pages. Each tab page is measured against the full content constraint (the tab controller
    // gives each tab the whole content area below the bar).
    maui::graphics::size tabbed_page::measure(double width_constraint, double height_constraint)
    {
        for (content_page* const page : children())
        {
            if (page != nullptr)
            {
                page->measure(width_constraint, height_constraint);
            }
        }
        const maui::graphics::size measured{width_constraint, height_constraint};
        desired_size_ = measured;
        return measured;
    }

    // C# TabbedPage arrange: record the frame, size the native tab host (the handler's platform_arrange),
    // then arrange EACH tab page HOST-RELATIVE. content_page::arrange (the inherited base) only knows the
    // single `content_` (null on a tabbed_page), so without this the tab content never gets a frame and
    // renders blank — the flyout_page::arrange precedent for its panes. Each page arranges at {0,0,w,h}:
    // its native view is a subview of the tab controller's per-tab child VC view (UIKit positions that),
    // so the content must start at the tab's origin. All pages are arranged (the tab controller keeps
    // every child view alive + laid out, so a later tab switch shows already-arranged content).
    maui::graphics::size tabbed_page::arrange(const maui::graphics::rect& bounds)
    {
        frame_ = bounds;
        if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(handler().get()))
        {
            view_handler->platform_arrange(bounds); // size/position the native tab host
        }
        const maui::graphics::rect page_bounds{0, 0, bounds.width, bounds.height};
        for (content_page* const page : children())
        {
            if (page != nullptr)
            {
                page->arrange(page_bounds);
            }
        }
        return {bounds.width, bounds.height};
    }
} // namespace maui::controls

// Self-register the default handler for tabbed_page (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::tabbed_page, maui::core::tabbed_page_handler)
