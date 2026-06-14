// indicator_view_handler — headless platform recipe. A testable stand-in for the native dot control
// (UIPageControl / NSStackView): the mapped properties mirror into indicator_view_platform. dot_count
// tracks GetMaximumVisible (UpdatePages); current_page tracks the clamped Position (the C#
// MauiPageControl.GetCurrentPage: position >= maxVisible ? maxVisible - 1 : position). The Apple/iOS
// .mm partials are the real twins.

#include "maui/core/indicator_view_handler.hpp"

#include <algorithm>
#include <memory>

#include "maui/core/i_indicator_view.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Headless has no native view in the `native` slot, so destruction is trivial.
    indicator_view_platform::indicator_view_platform() = default;
    indicator_view_platform::~indicator_view_platform() = default;

    std::unique_ptr<indicator_view_platform> indicator_view_handler::create_platform_view()
    {
        return std::make_unique<indicator_view_platform>();
    }

    // C# ConnectHandler: SetIndicatorView + UpdateIndicator. Headless has no template-override layout to
    // host, so connect leaves the mirror to the mapper pass (which runs right after connect).
    void indicator_view_handler::on_connect_handler(indicator_view_platform& /*platform*/)
    {
    }

    void indicator_view_handler::on_disconnect_handler(indicator_view_platform& /*platform*/)
    {
    }

    namespace
    {
        // UpdateIndicatorCount → UpdatePages(GetMaximumVisible) + UpdatePosition (the C# coupling).
        void refresh_count_and_position(indicator_view_platform& platform, i_indicator_view& view)
        {
            platform.dot_count = max_visible_indicators(view);
            // MauiPageControl.GetCurrentPage: clamp the position into [0, dot_count - 1].
            const int position = view.position();
            platform.current_page = platform.dot_count > 0 ? std::min(position, platform.dot_count - 1) : -1;
        }
    } // namespace

    void indicator_view_handler::map_count(indicator_view_handler& handler, i_indicator_view& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            refresh_count_and_position(*platform, view); // UpdateIndicatorCount
        }
    }

    void indicator_view_handler::map_maximum_visible(indicator_view_handler& handler, i_indicator_view& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            refresh_count_and_position(*platform, view); // UpdateIndicatorCount
        }
    }

    void indicator_view_handler::map_hide_single(indicator_view_handler& handler, i_indicator_view& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            refresh_count_and_position(*platform, view); // UpdateHideSingle → recount
        }
    }

    void indicator_view_handler::map_position(indicator_view_handler& handler, i_indicator_view& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            const int position = view.position();
            platform->current_page = platform->dot_count > 0 ? std::min(position, platform->dot_count - 1) : -1;
        }
    }

    void indicator_view_handler::map_indicator_size(indicator_view_handler& handler, i_indicator_view& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->indicator_size = view.indicator_size();
        }
    }

    void indicator_view_handler::map_indicator_color(indicator_view_handler& handler, i_indicator_view& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->indicator_color = view.indicator_color();
        }
    }

    void indicator_view_handler::map_selected_indicator_color(indicator_view_handler& handler, i_indicator_view& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->selected_indicator_color = view.selected_indicator_color();
        }
    }

    void indicator_view_handler::map_indicator_shape(indicator_view_handler& handler, i_indicator_view& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->shape = view.indicators_shape();
        }
    }

    maui::graphics::size indicator_view_handler::get_desired_size(double /*width_constraint*/,
                                                                  double /*height_constraint*/) const
    {
        // Headless placeholder metric: a row of `dot_count` dots of indicator_size each, default 6.
        const auto* platform = typed_platform_view();
        const double size = platform != nullptr ? platform->indicator_size : 6.0;
        const int dots = platform != nullptr ? platform->dot_count : 0;
        return {size * static_cast<double>(std::max(dots, 1)), size};
    }

    void indicator_view_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native layout to apply.
    }
} // namespace maui::core
