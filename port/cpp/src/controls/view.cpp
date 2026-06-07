// maui::controls::view — out-of-line definitions: the shared bindable-property descriptors for the four
// generic IView properties (VisualElement.IsEnabled / Opacity / IsVisible(Visibility) +
// Element.AutomationId). These are NON-template free functions so there is exactly ONE descriptor per
// property — shared across every view<ViewInterface> instantiation — and the property name the
// view_mapper keys on is identical for every control. See view.hpp.
//
// Defaults mirror VisualElement.cs: IsEnabled = true, Opacity = 1.0 (clamped to [0,1], matching
// VisualElement's coerceValue), Visibility = Visible. AutomationId defaults to "" (C#'s is null; our
// value type is std::string). The property names match the view_mapper keys exactly.

#include "maui/controls/view.hpp"

#include <algorithm>
#include <string>

#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/visibility.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<bool>& is_enabled_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"is_enabled", true};
        return descriptor;
    }

    const maui::core::bindable_property<double>& opacity_property()
    {
        static const maui::core::bindable_property<double> descriptor{
            "opacity",
            1.0,
            // VisualElement.OpacityProperty clamps to [0,1].
            {.coerce_value = [](maui::core::bindable_object& /*owner*/, const double& value) {
                return std::clamp(value, 0.0, 1.0);
            }}};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::visibility>& visibility_property()
    {
        static const maui::core::bindable_property<maui::core::visibility> descriptor{"visibility",
                                                                                      maui::core::visibility::visible};
        return descriptor;
    }

    const maui::core::bindable_property<std::string>& automation_id_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"automation_id", std::string{}};
        return descriptor;
    }
} // namespace maui::controls
