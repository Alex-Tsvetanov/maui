// maui::xaml — XAML registrations for gradient BRUSHES in ELEMENT form (W7):
//   LinearGradientBrush, RadialGradientBrush, GradientStop.
//
// The STRING form (Background="Red" / "linear-gradient(...)") goes through convert_brush (W1/X1). This is
// the ELEMENT form the gallery pages use:
//   <BoxView.Background>
//     <LinearGradientBrush EndPoint="1,0">
//       <GradientStop Color="Yellow" Offset="0.1"/>
//       <GradientStop Color="Green"  Offset="1.0"/>
//     </LinearGradientBrush>
//   </BoxView.Background>
//
// The brushes are bindable_objects (brush : element) and default-constructible, so register_type creates
// them like any control; their StartPoint/EndPoint/Center/Radius are bindable properties; their
// <GradientStop> children are GradientBrush's [ContentProperty] GradientStops, routed through the child
// sink. The created brush (boxed as shared_ptr<bindable_object>) reaches a Background property — which
// expects shared_ptr<brush> — via the object-coercion in apply_properties_visitor (xaml_visitors.cpp):
// a created element whose property value-type is shared_ptr<brush> is dynamic_pointer_cast to brush.

#include "register_xaml_groups.hpp"

#include <memory>

#include "maui/controls/brushes/gradient_brush.hpp"
#include "maui/controls/brushes/gradient_stop.hpp"
#include "maui/controls/brushes/linear_gradient_brush.hpp"
#include "maui/controls/brushes/radial_gradient_brush.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/graphics/color.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"

namespace maui::xaml
{
    namespace
    {
        // GradientBrush's [ContentProperty("GradientStops")] child sink: each <GradientStop> child is added
        // to the brush's stops collection. The stop is owned by the XAML object graph, so the collection
        // takes a NON-OWNING aliasing shared_ptr (the content_view pattern) — no double free.
        template <class TBrush> void register_gradient_stops_sink(xaml_property_registry& properties)
        {
            properties.register_add_child<TBrush>([](TBrush& brush, maui::core::bindable_object& child) {
                auto* stop = dynamic_cast<maui::controls::gradient_stop*>(&child);
                if (stop == nullptr)
                {
                    return false;
                }
                brush.gradient_stops().add(
                    std::shared_ptr<maui::controls::gradient_stop>(std::shared_ptr<void>{}, stop));
                return true;
            });
        }
    } // namespace

    void register_xaml_brushes(xaml_type_registry& types, xaml_property_registry& properties,
                               xaml_converter_registry& /*converters*/)
    {
        namespace controls = maui::controls;

        // ---- GradientStop (Color is optional<color> in the port; register a color lambda so the existing
        //      color converter applies, wrapping into the optional. Offset is a plain float bindable). ----
        types.register_type<controls::gradient_stop>("GradientStop");
        properties.register_property<controls::gradient_stop, maui::graphics::color>(
            "Color", [](controls::gradient_stop& stop, const maui::graphics::color& value) { stop.set_color(value); });
        properties.register_bindable_property<controls::gradient_stop>("Offset",
                                                                       controls::gradient_stop::offset_property());

        // ---- LinearGradientBrush (StartPoint/EndPoint + GradientStops) ----
        types.register_type<controls::linear_gradient_brush>("LinearGradientBrush");
        properties.register_bindable_property<controls::linear_gradient_brush>(
            "StartPoint", controls::linear_gradient_brush::start_point_property());
        properties.register_bindable_property<controls::linear_gradient_brush>(
            "EndPoint", controls::linear_gradient_brush::end_point_property());
        register_gradient_stops_sink<controls::linear_gradient_brush>(properties);

        // ---- RadialGradientBrush (Center/Radius + GradientStops) ----
        types.register_type<controls::radial_gradient_brush>("RadialGradientBrush");
        properties.register_bindable_property<controls::radial_gradient_brush>(
            "Center", controls::radial_gradient_brush::center_property());
        properties.register_bindable_property<controls::radial_gradient_brush>(
            "Radius", controls::radial_gradient_brush::radius_property());
        register_gradient_stops_sink<controls::radial_gradient_brush>(properties);
    }
} // namespace maui::xaml
