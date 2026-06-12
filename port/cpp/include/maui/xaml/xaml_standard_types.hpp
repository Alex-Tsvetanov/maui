#pragma once
// maui::xaml — standard registrations: the v1 control set + built-in converters (M7 wave 1).
//
// C# counterpart: what reflection gives the XAML loader for free — every public type in the MAUI
// xmlns (XamlParser.GetElementType), every `{Name}Property` field and CLR property
// (ApplyPropertiesVisitor.GetBindableProperty / GetRuntimeProperties), every [ContentProperty]
// attribute, and TypeConversionExtensions' built-in invariant conversions. The reflection-free port
// registers all of it EXPLICITLY (PROFILE §6) through these entry points; the M7 loader calls
// register_standard_xaml(…) once on the registries it resolves against (typically the process-wide
// defaults).
//
// register_standard_xaml_types — Button / Label / Entry / Image / VerticalStackLayout /
//   HorizontalStackLayout / Grid / ContentPage / NavigationPage / Window, under their markup names.
// register_standard_xaml_properties — each control's v1 XAML attribute surface (the bindable
//   properties routed through apply_setter + the explicit non-bindable members) and the
//   [ContentProperty] metadata (Label→Text; ContentPage→set_content, layouts→add, Window→set_content,
//   NavigationPage→push). See xaml_standard_types.cpp for the per-control derivations + documented
//   deferrals (attached properties and font sub-attributes are loader-side work).
// register_standard_xaml_converters — the FULL converter table (M7 converter parity):
//   TypeConversionExtensions' invariant built-ins (std::string/double/float/int/bool) plus every
//   maui/xaml/xaml_converters.hpp value converter (color/point/rect/size/size_f/thickness/
//   corner_radius/grid_length/row+column definitions/layout_alignment/easing) and enum table
//   (text_alignment/aspect/visibility/return_type/clear_button_visibility/flow_direction/
//   text_decorations), keyed by value type. Per-PROPERTY C# converters (FontSizeConverter,
//   VisibilityConverter's IsVisible aliases) have no type-keyed slot — documented deferral.

namespace maui::xaml
{
    class xaml_type_registry;
    class xaml_property_registry;
    class xaml_converter_registry;

    void register_standard_xaml_types(xaml_type_registry& types);
    void register_standard_xaml_properties(xaml_property_registry& properties);
    void register_standard_xaml_converters(xaml_converter_registry& converters);

    // All three at once — the loader's one-call setup.
    void register_standard_xaml(xaml_type_registry& types, xaml_property_registry& properties,
                                xaml_converter_registry& converters);
} // namespace maui::xaml
