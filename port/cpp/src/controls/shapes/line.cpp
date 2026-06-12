// maui::controls::shapes::line — out-of-line definitions: the X1/Y1/X2/Y2 descriptors + the
// default-handler self-registration (the shared shape_view_handler). See line.hpp.

#include "maui/controls/shapes/line.hpp"

#include "maui/core/bindable_property.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/shape_view_handler.hpp"

namespace maui::controls::shapes
{
    const maui::core::bindable_property<double>& line::x1_property()
    {
        // C# Line.X1Property default: 0.0. The keys match the handler's absorbed sub-handler entries.
        static const maui::core::bindable_property<double> descriptor{"x1", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& line::y1_property()
    {
        static const maui::core::bindable_property<double> descriptor{"y1", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& line::x2_property()
    {
        static const maui::core::bindable_property<double> descriptor{"x2", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& line::y2_property()
    {
        static const maui::core::bindable_property<double> descriptor{"y2", 0.0};
        return descriptor;
    }
} // namespace maui::controls::shapes

// Self-register the shared shape handler for line (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::shapes::line, maui::core::shape_view_handler)
