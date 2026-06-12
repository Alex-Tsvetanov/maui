// flyout_page_handler — headless platform recipe. The "native two-pane host" is the set of mirrors in
// flyout_page_platform (hosted_flyout / hosted_detail / presented / behavior / gesture_enabled) so
// tests can observe that the host tracks the panes and the presented state. The real twins are
// src/platform/{ios,apple}/flyout_page_handler.mm (UISplitViewController / NSSplitViewController).

#include "maui/core/flyout_page_handler.hpp"

#include <memory>

#include "maui/core/i_flyout_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    flyout_page_platform::~flyout_page_platform() = default;

    std::unique_ptr<flyout_page_platform> flyout_page_handler::create_platform_view()
    {
        return std::make_unique<flyout_page_platform>();
    }

    void flyout_page_handler::set_panes(i_view& view)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        if (const auto* flyout = dynamic_cast<i_flyout_view*>(&view))
        {
            platform->hosted_flyout = flyout->flyout_view();
            platform->hosted_detail = flyout->flyout_detail();
        }
    }

    void flyout_page_handler::update_presentation(i_view& view)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        if (const auto* flyout = dynamic_cast<i_flyout_view*>(&view))
        {
            platform->presented = flyout->flyout_is_presented();
            platform->behavior = flyout->flyout_behavior_value();
        }
    }

    maui::graphics::size flyout_page_handler::get_desired_size(double /*width_constraint*/,
                                                               double /*height_constraint*/) const
    {
        // The flyout page sizes from its panes, not the handler, so it reports nothing here.
        return {0, 0};
    }

    void flyout_page_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native split host to position.
    }
} // namespace maui::core
