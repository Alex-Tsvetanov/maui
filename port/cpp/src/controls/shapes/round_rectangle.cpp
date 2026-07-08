// maui::controls::shapes::round_rectangle — out-of-line definitions: the CornerRadius descriptor +
// the default-handler self-registration (the shared shape_view_handler). See round_rectangle.hpp.

#include "maui/controls/shapes/round_rectangle.hpp"

#include "maui/core/bindable_property.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/shape_view_handler.hpp"
#include "maui/graphics/corner_radius.hpp"

namespace maui::controls::shapes
{
    const maui::core::bindable_property<maui::graphics::corner_radius>& round_rectangle::corner_radius_property()
    {
        // C# RoundRectangle.CornerRadiusProperty default: new CornerRadius() (all-zero). The key matches
        // the handler's absorbed sub-handler entry ("corner_radius" → InvalidateShape).
        static const maui::core::bindable_property<maui::graphics::corner_radius> descriptor{
            "corner_radius", maui::graphics::corner_radius{}};
        return descriptor;
    }
} // namespace maui::controls::shapes

// Self-register the shared shape handler for round_rectangle (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::shapes::round_rectangle, maui::core::shape_view_handler)
