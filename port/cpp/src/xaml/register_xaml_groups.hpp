#pragma once
// maui::xaml — per-group XAML registration entry points (internal header, NOT installed).
//
// The v1 control set's registrations are PARTITIONED into one free function per control group so the
// groups can be authored (and built) in parallel, each in its own TU (src/xaml/register_xaml_<g>.cpp).
// register_standard_xaml(...) (xaml_standard_types.cpp) composes the core 11 controls (the original
// register_standard_xaml_{types,properties,converters}) and then calls every group function below.
//
// Each register_xaml_<g> registers the group's controls into ALL THREE registries it is handed:
// types.register_type<T>("Elem"), the per-control property surface (register_view_properties<T> +
// register_bindable_property / content / child sinks), and — when a group needs a value converter not
// already in the standard table — that converter. register_xaml_extra_converters is the shared sink for
// converters multiple groups depend on (so a type-keyed converter is registered exactly once).
//
// See register_xaml_text_input.cpp for the fully-worked reference group (the pattern the others mirror).

namespace maui::xaml
{
    class xaml_type_registry;
    class xaml_property_registry;
    class xaml_converter_registry;

    // One per control group (see xaml_specs.json's group names).
    void register_xaml_text_input(xaml_type_registry& types, xaml_property_registry& properties,
                                  xaml_converter_registry& converters);
    void register_xaml_toggles_selections(xaml_type_registry& types, xaml_property_registry& properties,
                                          xaml_converter_registry& converters);
    void register_xaml_range_progress(xaml_type_registry& types, xaml_property_registry& properties,
                                      xaml_converter_registry& converters);
    void register_xaml_pickers(xaml_type_registry& types, xaml_property_registry& properties,
                               xaml_converter_registry& converters);
    void register_xaml_containers_content(xaml_type_registry& types, xaml_property_registry& properties,
                                          xaml_converter_registry& converters);
    void register_xaml_scrolling_interactive(xaml_type_registry& types, xaml_property_registry& properties,
                                             xaml_converter_registry& converters);
    void register_xaml_specialized_views(xaml_type_registry& types, xaml_property_registry& properties,
                                         xaml_converter_registry& converters);
    void register_xaml_layouts(xaml_type_registry& types, xaml_property_registry& properties,
                               xaml_converter_registry& converters);
    void register_xaml_pages(xaml_type_registry& types, xaml_property_registry& properties,
                             xaml_converter_registry& converters);
    void register_xaml_shapes(xaml_type_registry& types, xaml_property_registry& properties,
                              xaml_converter_registry& converters);
    // 2026-07: View.Clip's element-form geometry values (RectangleGeometry / EllipseGeometry /
    // GeometryGroup / RoundRectangleGeometry / PathGeometry) — closes the Image.Clip XAML gap.
    void register_xaml_geometries(xaml_type_registry& types, xaml_property_registry& properties,
                                  xaml_converter_registry& converters);
    // W4: the templated-collection group (CollectionView / CarouselView) — ItemsSource + ItemTemplate.
    void register_xaml_items(xaml_type_registry& types, xaml_property_registry& properties,
                             xaml_converter_registry& converters);
    // W7: gradient brushes in element form (LinearGradientBrush / RadialGradientBrush / GradientStop).
    void register_xaml_brushes(xaml_type_registry& types, xaml_property_registry& properties,
                               xaml_converter_registry& converters);
    // W8: FormattedString / Span element form (Label.FormattedText) + the font_attributes converter.
    void register_xaml_formatted_text(xaml_type_registry& types, xaml_property_registry& properties,
                                      xaml_converter_registry& converters);

    // Shared converter sink: value converters that more than one group needs (registered once).
    void register_xaml_extra_converters(xaml_converter_registry& converters);
} // namespace maui::xaml
