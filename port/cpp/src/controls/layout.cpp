// maui::controls::layout — out-of-line definitions: the shared bindable-property descriptor for
// Layout.IsClippedToBounds (ILayout.ClipsToBounds). A NON-template free function so there is exactly ONE
// descriptor shared across every layout<LayoutInterface> instantiation — and the property name the
// layout_handler's mapper keys on ("clips_to_bounds") is identical for every layout control. See
// layout.hpp.
//
// Default false, matching Layout.IsClippedToBoundsProperty (Layout.cs).

#include "maui/controls/layout.hpp"

#include "maui/core/bindable_property.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<bool>& clips_to_bounds_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"clips_to_bounds", false};
        return descriptor;
    }
} // namespace maui::controls
