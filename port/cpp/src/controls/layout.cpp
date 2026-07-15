// maui::controls::layout — out-of-line definitions: the shared bindable-property descriptor for
// Layout.IsClippedToBounds (ILayout.ClipsToBounds). A NON-template free function so there is exactly ONE
// descriptor shared across every layout<LayoutInterface> instantiation — and the property name the
// layout_handler's mapper keys on ("clips_to_bounds") is identical for every layout control. See
// layout.hpp.
//
// Default false, matching Layout.IsClippedToBoundsProperty (Layout.cs).

#include "maui/controls/layout.hpp"

#include "maui/core/bindable_property.hpp"
#include "maui/core/safe_area_edges.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<bool>& clips_to_bounds_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"clips_to_bounds", false};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::safe_area_edges>& layout_safe_area_edges_property()
    {
        // C# SafeAreaElement.SafeAreaEdgesProperty: the static metadata default is SafeAreaEdges.Default, but
        // the per-element default-value creator (Layout.SafeAreaEdgesDefaultValueCreator) returns
        // SafeAreaEdges.Container — so every layout reads Container by default. Contrast ContentPage / Border /
        // ContentView, whose creators return None; this Container default is why MAUI keeps a page's content
        // layout clear of the bars/notch while the page itself stays edge-to-edge.
        static const maui::core::bindable_property<maui::core::safe_area_edges> descriptor{
            "safe_area_edges",
            maui::core::safe_area_edges::default_edges(),
            {.default_value_creator = [](const maui::core::bindable_object&) {
                return maui::core::safe_area_edges::container();
            }}};
        return descriptor;
    }
} // namespace maui::controls
