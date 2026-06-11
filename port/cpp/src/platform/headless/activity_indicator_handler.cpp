// activity_indicator_handler — headless platform recipe. A testable stand-in for a native spinner:
// the mapped properties mirror into activity_indicator_platform; map_is_running mirrors C#'s
// UpdateIsRunning coupling (animating only while IsRunning && Visible, hidden tracking visibility).
// The Apple/iOS .mm partials are the real twins.

#include "maui/core/activity_indicator_handler.hpp"

#include <memory>

#include "maui/core/i_activity_indicator.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Headless has no native view in the `native` slot, so destruction is trivial.
    activity_indicator_platform::~activity_indicator_platform() = default;

    std::unique_ptr<activity_indicator_platform> activity_indicator_handler::create_platform_view()
    {
        return std::make_unique<activity_indicator_platform>();
    }

    void activity_indicator_handler::map_is_running(activity_indicator_handler& handler, i_activity_indicator& view)
    {
        // ActivityIndicatorExtensions.UpdateIsRunning: animate only while IsRunning && Visible; the
        // visibility half is handled here too (the mapper's Visibility key routes to this function).
        // Hidden and Collapsed both hide (the Collapse() constraint dance is the shared deferral).
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const bool visible = view.visibility() == visibility::visible;
        platform->is_running = view.is_running() && visible;
        platform->hidden = !visible;
    }

    void activity_indicator_handler::map_color(activity_indicator_handler& handler, i_activity_indicator& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->color = view.color(); // ActivityIndicatorExtensions.UpdateColor
        }
    }

    maui::graphics::size activity_indicator_handler::get_desired_size(double /*width_constraint*/,
                                                                      double /*height_constraint*/) const
    {
        // Headless placeholder metric: the UIActivityIndicatorView medium-style square (~20x20).
        return {20.0, 20.0};
    }

    void activity_indicator_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native layout to apply.
    }
} // namespace maui::core
