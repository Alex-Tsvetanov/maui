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
//   deferrals (enum/struct-typed attributes wait on unit U4's converters; attached properties and
//   font sub-attributes are loader-side work).
// register_standard_xaml_converters — the trivially-available built-ins only: std::string, double,
//   int, bool (TypeConversionExtensions' invariant Parse behaviors). The MAUI value converters
//   (color/thickness/…) are unit U4's deliverable into the same seam.

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
