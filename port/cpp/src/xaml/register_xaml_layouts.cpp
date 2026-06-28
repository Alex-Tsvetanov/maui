// maui::xaml — layouts group: AbsoluteLayout, FlexLayout
//
// Follows the register_xaml_helpers.hpp conventions exactly:
//   - types.register_type<cpp_type>("XamlElement") per control;
//   - register_view_properties<cpp_type>(properties) to flatten the shared IView surface;
//   - one properties.register_bindable_property<cpp_type>("Attr", accessor) per property;
//   - register_layout_children<TLayout>(properties) for the Layout.cs [ContentProperty(Children)] sink.
//
// AbsoluteLayout: only padding_property() is a static bindable descriptor. The per-child attached
// properties LayoutBounds (maui::graphics::rect) and LayoutFlags (maui::layouts::absolute_layout_flags)
// are stored in an instance map and exposed only via set_layout_bounds/set_layout_flags — they have
// no static bindable_property<T> accessor and must be handled by the dotted-name attached-property
// path (try_apply_attached_property in xaml_visitors.cpp), same as Grid.Row/Column. Deferred.
//
// FlexLayout: all six container-level properties have static bindable descriptors and are registered.
// The per-child attached values (Order/Grow/Shrink/AlignSelf/Basis) similarly have no static
// bindable_property<T> accessors and are deferred to the attached-property path.
//
// Missing converter for Position: flex_position has no convert_flex_position in xaml_converters.hpp
// and is not in register_standard_xaml_converters. The register_bindable_property call below is still
// correct — it enables the binding path (apply_setter routes through the descriptor); only XAML text
// literals for Position= are inoperable until a convert_flex_position is added and registered.

#include "register_xaml_groups.hpp"  // declares void register_xaml_layouts(...)
#include "register_xaml_helpers.hpp" // register_view_properties<T>, register_layout_children<T>

#include "maui/controls/absolute_layout.hpp"
#include "maui/controls/flex_layout.hpp"
#include "maui/core/thickness.hpp"
#include "maui/layouts/flex_enums.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"

namespace maui::xaml
{
    void register_xaml_layouts(xaml_type_registry& types, xaml_property_registry& properties,
                               xaml_converter_registry& /*converters*/)
    {
        namespace controls = maui::controls;

        // ---- AbsoluteLayout (AbsoluteLayout.cs; [ContentProperty(nameof(Children))]) ----
        // Only static bindable: padding_property(). Per-child LayoutBounds/LayoutFlags are attached
        // properties (instance map, no static descriptor) — deferred to dotted-name attached path.
        types.register_type<controls::absolute_layout>("AbsoluteLayout");
        register_view_properties<controls::absolute_layout>(properties);
        properties.register_bindable_property<controls::absolute_layout>("Padding",
                                                                         controls::absolute_layout::padding_property());
        register_layout_children<controls::absolute_layout>(properties);

        // ---- FlexLayout (FlexLayout.cs; [ContentProperty(nameof(Children))]) ----
        // Six container-level bindable descriptors. Per-child Order/Grow/Shrink/AlignSelf/Basis are
        // attached properties (instance map, no static descriptor) — deferred to dotted-name attached
        // path. NOTE: flex_position has no registered text converter; Position bindings still work.
        types.register_type<controls::flex_layout>("FlexLayout");
        register_view_properties<controls::flex_layout>(properties);
        properties.register_bindable_property<controls::flex_layout>("Direction",
                                                                     controls::flex_layout::direction_property());
        properties.register_bindable_property<controls::flex_layout>("JustifyContent",
                                                                     controls::flex_layout::justify_content_property());
        properties.register_bindable_property<controls::flex_layout>("AlignContent",
                                                                     controls::flex_layout::align_content_property());
        properties.register_bindable_property<controls::flex_layout>("AlignItems",
                                                                     controls::flex_layout::align_items_property());
        properties.register_bindable_property<controls::flex_layout>("Position",
                                                                     controls::flex_layout::position_property());
        properties.register_bindable_property<controls::flex_layout>("Wrap", controls::flex_layout::wrap_property());
        properties.register_bindable_property<controls::flex_layout>("Padding",
                                                                     controls::flex_layout::padding_property());
        register_layout_children<controls::flex_layout>(properties);
    }
} // namespace maui::xaml
