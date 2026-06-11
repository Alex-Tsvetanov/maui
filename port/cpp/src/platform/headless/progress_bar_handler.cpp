// progress_bar_handler — headless platform recipe. A testable stand-in for a native progress bar:
// the mapped properties mirror into progress_bar_platform (display-only — no inbound events). The
// Apple/iOS .mm partials are the real twins.

#include "maui/core/progress_bar_handler.hpp"

#include <memory>

#include "maui/core/i_progress.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Headless has no native view in the `native` slot, so destruction is trivial.
    progress_bar_platform::~progress_bar_platform() = default;

    std::unique_ptr<progress_bar_platform> progress_bar_handler::create_platform_view()
    {
        return std::make_unique<progress_bar_platform>();
    }

    void progress_bar_handler::map_progress(progress_bar_handler& handler, i_progress& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->progress = view.progress(); // ProgressBarExtensions.UpdateProgress
        }
    }

    void progress_bar_handler::map_progress_color(progress_bar_handler& handler, i_progress& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->progress_color = view.progress_color(); // ProgressBarExtensions.UpdateProgressColor
        }
    }

    maui::graphics::size progress_bar_handler::get_desired_size(double /*width_constraint*/,
                                                                double /*height_constraint*/) const
    {
        // Headless placeholder metric: a nominal track (UIProgressView's natural height is ~4).
        return {100.0, 4.0};
    }

    void progress_bar_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native layout to apply.
    }
} // namespace maui::core
