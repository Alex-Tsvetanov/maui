// maui::controls::stack_layout — out-of-line definitions: the shared bindable-property descriptors +
// the default-handler self-registration. See stack_layout.hpp + the C# oracle StackLayout.cs.

#include "maui/controls/stack_layout.hpp"

#include "maui/controls/stack_orientation.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/thickness.hpp"

namespace maui::controls
{
    // StackLayout.OrientationProperty: default Vertical; C# OrientationChanged calls InvalidateMeasure
    // on the owning layout, mirrored here. (invalidate_measure() is currently the M3 no-op seam — see
    // view::invalidate_measure — but wiring the callback keeps the structure faithful to C# and makes
    // the seam light up automatically once layout invalidation is implemented.)
    const maui::core::bindable_property<stack_orientation>& stack_layout::orientation_property()
    {
        static const maui::core::bindable_property<stack_orientation> descriptor{
            "orientation",
            stack_orientation::vertical,
            {.property_changed = [](maui::core::bindable_object& bindable, const stack_orientation& /*old_value*/,
                                    const stack_orientation& /*new_value*/) {
                dynamic_cast<stack_layout&>(bindable).invalidate_measure(); // descriptor owner is a stack_layout
            }}};
        return descriptor;
    }

    const maui::core::bindable_property<double>& stack_layout::spacing_property()
    {
        static const maui::core::bindable_property<double> descriptor{"spacing", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::thickness>& stack_layout::padding_property()
    {
        static const maui::core::bindable_property<maui::core::thickness> descriptor{"padding"};
        return descriptor;
    }
} // namespace maui::controls

// Self-register the default handler for stack_layout (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::stack_layout, maui::core::layout_handler)
