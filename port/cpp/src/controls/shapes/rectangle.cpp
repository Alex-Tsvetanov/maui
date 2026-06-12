// maui::controls::shapes::rectangle — out-of-line definitions: the RadiusX/RadiusY descriptors +
// the default-handler self-registration (the shared shape_view_handler). See rectangle.hpp.

#include "maui/controls/shapes/rectangle.hpp"

#include "maui/core/bindable_property.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/shape_view_handler.hpp"

namespace maui::controls::shapes
{
    const maui::core::bindable_property<double>& rectangle::radius_x_property()
    {
        // C# Rectangle.RadiusXProperty default: 0.0. The key matches the handler's absorbed
        // sub-handler entry ("radius_x" → InvalidateShape).
        static const maui::core::bindable_property<double> descriptor{"radius_x", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& rectangle::radius_y_property()
    {
        // C# Rectangle.RadiusYProperty default: 0.0.
        static const maui::core::bindable_property<double> descriptor{"radius_y", 0.0};
        return descriptor;
    }
} // namespace maui::controls::shapes

// Self-register the shared shape handler for rectangle (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::shapes::rectangle, maui::core::shape_view_handler)
