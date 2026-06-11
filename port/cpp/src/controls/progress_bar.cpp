// maui::controls::progress_bar — out-of-line definitions: the shared bindable-property descriptors
// (one instance per type, like ProgressBar.*Property) and the default-handler self-registration.

#include "maui/controls/progress_bar.hpp"

#include "maui/core/bindable_property.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/progress_bar_handler.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    // ProgressBar.ProgressProperty: default 0, coerced through Clamp(0, 1).
    const maui::core::bindable_property<double>& progress_bar::progress_property()
    {
        static const maui::core::bindable_property<double> descriptor{
            "progress", 0.0, {.coerce_value = [](maui::core::bindable_object& /*bindable*/, const double& value) {
                if (value < 0.0)
                {
                    return 0.0;
                }
                if (value > 1.0)
                {
                    return 1.0;
                }
                return value;
            }}};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& progress_bar::progress_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"progress_color"};
        return descriptor;
    }
} // namespace maui::controls

// Self-register the default handler (opt-in, PROFILE §6). This TU is always linked (the descriptors
// above are referenced by every user of the control), so the registrar runs.
MAUI_REGISTER_HANDLER(maui::controls::progress_bar, maui::core::progress_bar_handler)
