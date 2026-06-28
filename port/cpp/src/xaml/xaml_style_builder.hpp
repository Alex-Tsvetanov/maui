#pragma once
// maui::xaml — the loader-side <Style>/<Setter> resolution helper (W3, internal header, NOT installed).
//
// The code-first style RUNTIME (controls::style / controls::setter / resource_dictionary / merged_style /
// specificity) is complete and headless-green; the ONLY gap was the XAML loader, where <Style>/<Setter>
// were unregistered loadable types. These free functions are the reflection-free substitutes for the C#
// pieces a <Style> needs at load time:
//   - resolve_target_type  <=  TypeTypeConverter / IXamlTypeResolver: TargetType="Label" -> the concrete
//     type_tag (via xaml_type_registry::find), which keys the property + content registries.
//   - build_style           <=  the XAML Style ctor + its TargetType / BasedOn / Class / ApplyToDerivedTypes
//     setters: mint an EMPTY shared_ptr<style> shell from a <Style> element node (read TargetType, x:Key,
//     BasedOn, Class, ApplyToDerivedTypes). The shell's setters are filled later, in the apply pass, when
//     each child <Setter> is visited (mirrors C# IValueProvider.ProvideValue running at apply time — the
//     create-vs-apply split documented in xaml_style_builder.cpp).
//   - build_setter          <=  BindablePropertyConverter + SetterValueProvider: resolve a Setter's
//     Property name against the PARENT Style's TargetType (xaml_property_registry::find), convert the
//     Value literal via the property's value-type converter, and emit controls::setter::of_erased.
//
// Error strategy (xaml_parse_exception.hpp, the single M7 channel): every resolution failure throws —
// missing/unresolvable TargetType, a Property not found on the TargetType, no converter for the value
// type, a malformed literal (propagated from the converter). This matches MAUI's Style ctor null-check
// and BindablePropertyConverter throws.
//
// Scope (W3): the 1-part Property form (TextColor), implicit (keyless) styles, x:Key'd styles consumed
// via {StaticResource}, BasedOn-by-key (lazy via base_resource_key), Class, ApplyToDerivedTypes. The
// 2-part qualified Property form (Type.PropertyName), attached-property Setters (Setter Property="Grid.Row"),
// direct-child <Style.BasedOn>, and {Binding}/Triggers/Behaviors inside a Style stay deferred — each fails
// loudly (or is skipped) per the design risks list.

#include <any>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "maui/core/type_tag.hpp"

namespace maui::controls
{
    class style;
    class setter;
} // namespace maui::controls

namespace maui::xaml
{
    class xaml_type_registry;
    class xaml_property_registry;
    class xaml_converter_registry;

    // TypeTypeConverter / IXamlTypeResolver: resolve a TargetType element NAME ("Label") to its concrete
    // type_tag against the type registry. `namespace_uri` is the <Style> element's xmlns (only the maui
    // namespaces resolve). Throws xaml_parse_exception when the name is empty or unresolvable, mirroring
    // MAUI's "Can't resolve {name}" / Style-without-TargetType errors.
    [[nodiscard]] maui::core::type_tag resolve_target_type(std::string_view target_type_name,
                                                           std::string_view namespace_uri,
                                                           const xaml_type_registry& types, int line_number,
                                                           int line_position);

    // Mint an EMPTY style shell for `target_type`, with the optional x:Key (carried by the caller for
    // resource routing, not stored on the style), BasedOn resource key (lazy base_resource_key), Class
    // (Style.Class), and ApplyToDerivedTypes. The setters are added later by build_setter during the apply
    // pass.
    [[nodiscard]] std::shared_ptr<maui::controls::style> build_style(maui::core::type_tag target_type,
                                                                     std::optional<std::string> based_on_key,
                                                                     std::optional<std::string> style_class,
                                                                     bool apply_to_derived_types);

    // BindablePropertyConverter + SetterValueProvider: resolve `property_name` against `target_type`
    // (the parent Style's TargetType) and convert the Value literal via the property's value-type
    // converter, returning a type-erased controls::setter. `value_text` is the raw Value literal. Throws
    // xaml_parse_exception on: a Property not found on the TargetType, the 2-part qualified form, an
    // attached-property name (Grid.Row), no converter for the value type, or a malformed literal.
    [[nodiscard]] maui::controls::setter build_setter(maui::core::type_tag target_type, std::string_view property_name,
                                                      const std::string& value_text,
                                                      const xaml_property_registry& properties,
                                                      const xaml_converter_registry& converters, int line_number,
                                                      int line_position);

    // The of_erased overload for an already-CONVERTED, already-boxed Value (a Setter whose Value came from
    // a markup extension such as {StaticResource}, not a literal). Resolves the Property against the
    // TargetType for the bindable_name routing key + a value-type sanity check, then boxes `value` as-is.
    [[nodiscard]] maui::controls::setter build_setter_from_value(maui::core::type_tag target_type,
                                                                 std::string_view property_name, std::any value,
                                                                 const xaml_property_registry& properties,
                                                                 int line_number, int line_position);
} // namespace maui::xaml
