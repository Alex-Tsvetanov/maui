// view_platform_base — the default (headless) update_* bodies: each stores the corresponding mirror so
// the deterministic headless tests can observe the shared view_mapper ran with the right value. Real
// backends override these to push to the native view (src/platform/<backend>/*_handler.mm). See
// view_platform_base.hpp.

#include "maui/core/view_platform_base.hpp"

#include <string>
#include <string_view>

#include "maui/core/flow_direction.hpp"
#include "maui/core/visibility.hpp"

namespace maui::core
{
    view_platform_base::~view_platform_base() = default;

    void view_platform_base::update_visibility(maui::core::visibility value)
    {
        // C# Visibility: only Visible shows the view; Hidden and Collapsed both hide it (the
        // collapsed-vs-hidden layout distinction lives in the cross-platform layout pass, not here).
        hidden = value != maui::core::visibility::visible;
    }

    void view_platform_base::update_opacity(double value)
    {
        alpha = value;
    }

    void view_platform_base::update_is_enabled(bool value)
    {
        enabled = value;
    }

    void view_platform_base::update_automation_id(std::string_view value)
    {
        automation_id = std::string(value);
    }

    void view_platform_base::update_transform(const transform_spec& value)
    {
        transform = value;
    }

    void view_platform_base::update_flow_direction(maui::core::flow_direction value)
    {
        flow_direction = value;
    }
} // namespace maui::core
