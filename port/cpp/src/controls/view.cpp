// maui::controls::view — out-of-line definitions: the shared bindable-property descriptors for the
// generic IView properties (VisualElement.IsEnabled / Opacity / IsVisible(Visibility) +
// Element.AutomationId) plus the render-transform scalars and FlowDirection. These are NON-template free
// functions so there is exactly ONE descriptor per property — shared across every view<ViewInterface>
// instantiation — and the property name the view_mapper keys on is identical for every control. See
// view.hpp.
//
// Defaults mirror VisualElement.cs: IsEnabled = true, Opacity = 1.0 (clamped to [0,1], matching
// VisualElement's coerceValue), Visibility = Visible. AutomationId defaults to "" (C#'s is null; our
// value type is std::string). The render transform defaults to identity — translations/rotations 0,
// scales 1, anchors 0.5 — and FlowDirection to MatchParent (FlowDirection.cs). The property names match
// the view_mapper keys exactly.

#include "maui/controls/view.hpp"

#include <algorithm>
#include <string>

#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/flow_direction.hpp"
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

    // ---- render transform (VisualElement.cs identity defaults) ----
    const maui::core::bindable_property<double>& translation_x_property()
    {
        static const maui::core::bindable_property<double> descriptor{"translation_x", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& translation_y_property()
    {
        static const maui::core::bindable_property<double> descriptor{"translation_y", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& scale_property()
    {
        static const maui::core::bindable_property<double> descriptor{"scale", 1.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& scale_x_property()
    {
        static const maui::core::bindable_property<double> descriptor{"scale_x", 1.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& scale_y_property()
    {
        static const maui::core::bindable_property<double> descriptor{"scale_y", 1.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& rotation_property()
    {
        static const maui::core::bindable_property<double> descriptor{"rotation", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& rotation_x_property()
    {
        static const maui::core::bindable_property<double> descriptor{"rotation_x", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& rotation_y_property()
    {
        static const maui::core::bindable_property<double> descriptor{"rotation_y", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& anchor_x_property()
    {
        static const maui::core::bindable_property<double> descriptor{"anchor_x", 0.5};
        return descriptor;
    }

    const maui::core::bindable_property<double>& anchor_y_property()
    {
        static const maui::core::bindable_property<double> descriptor{"anchor_y", 0.5};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::flow_direction>& flow_direction_property()
    {
        static const maui::core::bindable_property<maui::core::flow_direction> descriptor{
            "flow_direction", maui::core::flow_direction::match_parent};
        return descriptor;
    }
} // namespace maui::controls
