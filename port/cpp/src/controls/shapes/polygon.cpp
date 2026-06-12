// maui::controls::shapes::polygon — out-of-line definitions: the Points/FillRule descriptors + the
// default-handler self-registration (the shared shape_view_handler). See polygon.hpp.

#include "maui/controls/shapes/polygon.hpp"

#include "maui/controls/shapes/fill_rule.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/shape_view_handler.hpp"

namespace maui::controls::shapes
{
    const maui::core::bindable_property<point_collection>& polygon::points_property()
    {
        // C# Polygon.PointsProperty defaultValueCreator: an empty PointCollection.
        static const maui::core::bindable_property<point_collection> descriptor{"points"};
        return descriptor;
    }

    const maui::core::bindable_property<shapes::fill_rule>& polygon::fill_rule_property()
    {
        // C# Polygon.FillRuleProperty default: FillRule.EvenOdd.
        static const maui::core::bindable_property<shapes::fill_rule> descriptor{"fill_rule",
                                                                                 shapes::fill_rule::even_odd};
        return descriptor;
    }
} // namespace maui::controls::shapes

// Self-register the shared shape handler for polygon (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::shapes::polygon, maui::core::shape_view_handler)
