// maui::controls::activity_indicator — out-of-line definitions: the shared bindable-property
// descriptors (one instance per type, like ActivityIndicator.*Property) and the default-handler
// self-registration.

#include "maui/controls/activity_indicator.hpp"

#include "maui/core/activity_indicator_handler.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    // ActivityIndicator.IsRunningProperty: default false.
    const maui::core::bindable_property<bool>& activity_indicator::is_running_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"is_running", false};
        return descriptor;
    }

    // ActivityIndicator.ColorProperty (ColorElement.ColorProperty).
    const maui::core::bindable_property<maui::graphics::color>& activity_indicator::color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"color"};
        return descriptor;
    }
} // namespace maui::controls

// Self-register the default handler (opt-in, PROFILE §6). This TU is always linked (the descriptors
// above are referenced by every user of the control), so the registrar runs.
MAUI_REGISTER_HANDLER(maui::controls::activity_indicator, maui::core::activity_indicator_handler)
