// navigation_page_handler — headless platform recipe. The "native container" is a single-page mirror
// (hosted_page) in navigation_page_platform so tests can observe that the host tracks the navigation
// stack's current page as it is pushed/popped. The Apple twin (a real NSView container subview swap) is
// src/platform/apple/navigation_page_handler.mm.

#include "maui/core/navigation_page_handler.hpp"

#include <memory>

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

    void navigation_page_handler::host_current(i_view* top)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // Mirror the navigation stack's current page (the Apple build re-parents the matching real NSView).
        platform->hosted_page = top;
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
} // namespace maui::core
