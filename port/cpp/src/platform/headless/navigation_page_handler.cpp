// navigation_page_handler — headless platform recipe. The "native container" is a set of mirrors in
// navigation_page_platform (hosted_page / hosted_modal / bar_title / back_button_visible / last_animated)
// so tests can observe that the host tracks the navigation stack's current page, the bar chrome, the modal
// overlay, and the animation flag as the stacks change. The Apple twin (a real NSView container with a
// custom bar + content swap + modal overlay) is src/platform/apple/navigation_page_handler.mm.

#include "maui/core/navigation_page_handler.hpp"

#include <memory>
#include <string>

#include "maui/core/i_stack_navigation.hpp"
#include "maui/core/i_view.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    navigation_page_platform::~navigation_page_platform() = default;

    std::unique_ptr<navigation_page_platform> navigation_page_handler::create_platform_view()
    {
        return std::make_unique<navigation_page_platform>();
    }

    void navigation_page_handler::on_connect_handler(navigation_page_platform& /*platform*/)
    {
        // Headless has no native bar / back button to wire; the back-button routing is exercised through
        // navigation_page::send_back_button_pressed() directly in the unit tests.
    }

    void navigation_page_handler::host_current(i_view* top, i_view& view, bool animated)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // Mirror the navigation stack's current page (the Apple build re-parents the matching real NSView).
        platform->hosted_page = top;
        platform->last_animated = animated;
        // Refresh the bar chrome from the view's navigation state (the Apple build populates a real
        // NSTextField + back NSButton from these, paints the bar background, and hosts the title view).
        if (auto* navigation = dynamic_cast<i_stack_navigation*>(&view))
        {
            platform->bar_title = std::string(navigation->navigation_bar_title());
            platform->back_button_visible = navigation->navigation_back_button_visible();
            platform->bar_background_color = navigation->navigation_bar_background_color();
            platform->bar_text_color = navigation->navigation_bar_text_color();
            platform->hosted_title_view = navigation->navigation_bar_title_view();
            // chrome (W1-11): mirror the page-surfaced toolbar items (the iOS twin materializes them as
            // bar buttons; AppKit surfaces them through the window's NSToolbar instead).
            platform->toolbar_items = navigation->navigation_toolbar_items();
        }
    }

    void navigation_page_handler::host_modal(i_view* top_modal, bool animated)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // Mirror the presented modal (the Apple build overlays the matching real NSView on the container).
        platform->hosted_modal = top_modal;
        platform->last_animated = animated;
    }

    maui::graphics::size navigation_page_handler::get_desired_size(double /*width_constraint*/,
                                                                   double /*height_constraint*/) const
    {
        // The navigation page sizes from its current page, not the handler, so it reports nothing here.
        return {0, 0};
    }

    void navigation_page_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native container to position; the current page is arranged by the control directly.
    }

    // --- platform configuration (W2-24): the iOSSpecific IsNavigationBarTranslucent push — headless
    // keeps the cross-platform mirror only.
    void navigation_page_handler::update_bar_translucent(bool value)
    {
        if (auto* platform = typed_platform_view())
        {
            platform->bar_translucent = value;
        }
    }
} // namespace maui::core
