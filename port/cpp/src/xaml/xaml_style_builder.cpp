// maui::xaml — the loader-side <Style>/<Setter> resolution helper (W3). See xaml_style_builder.hpp for
// the design and the C# mapping.
//
// CREATE-vs-APPLY timing (the central subtlety, mirroring C#): a Setter's Value converter needs the
// property's value type, which needs the parent Style's TargetType resolved — and the Setter resolution
// is THE point that needs the parent. So the loader mints an EMPTY style shell in the create pass (when
// the <Style> element node is visited bottom-up — its TargetType attribute is a plain literal, available
// then) and FILLS its setters in the apply pass, when each child <Setter> is visited and can walk to the
// parent <Style> node for the resolved TargetType + the minted shell. This matches C#'s
// IValueProvider.ProvideValue, which runs the Setter resolution at apply time.
#include "xaml_style_builder.hpp"

#include <format>
#include <utility>

#include "maui/controls/setter.hpp"
#include "maui/controls/style.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_node.hpp" // the maui xmlns URIs
#include "maui/xaml/xaml_parse_exception.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"

namespace maui::xaml
{
    namespace
    {
        // Whether a namespace URI is one of the maui xmlns the type registry resolves under (the same
        // gate create_values_visitor uses). The x namespace never declares controls, so a Style/Setter
        // outside the maui namespace cannot resolve a TargetType.
        [[nodiscard]] bool is_maui_namespace(std::string_view namespace_uri)
        {
            return namespace_uri == maui_uri || namespace_uri == maui_global_uri;
        }
    } // namespace

    maui::core::type_tag resolve_target_type(std::string_view target_type_name, std::string_view namespace_uri,
                                             const xaml_type_registry& types, int line_number, int line_position)
    {
        if (target_type_name.empty())
        {
            // MAUI: Style throws when TargetType is null ("Style requires a TargetType").
            throw xaml_parse_exception("Style requires a TargetType", line_number, line_position);
        }
        if (!is_maui_namespace(namespace_uri))
        {
            // Mirrors TypeTypeConverter failing to resolve the type's xmlns.
            throw xaml_parse_exception(
                std::format("Can't resolve type {} in xmlns {}", target_type_name, namespace_uri), line_number,
                line_position);
        }
        const xaml_type_registry::registration* registration = types.find(target_type_name, xaml_namespace::maui);
        if (registration == nullptr)
        {
            throw xaml_parse_exception(std::format("Can't resolve type {}", target_type_name), line_number,
                                       line_position);
        }
        return registration->type;
    }

    std::shared_ptr<maui::controls::style> build_style(maui::core::type_tag target_type,
                                                       std::optional<std::string> based_on_key,
                                                       std::optional<std::string> style_class,
                                                       bool apply_to_derived_types)
    {
        auto built = std::make_shared<maui::controls::style>(target_type);
        if (based_on_key.has_value() && !based_on_key->empty())
        {
            // BasedOn / BaseResourceKey stays LAZY (resolved at apply time via the resource_resolver
            // merged_style / set_style supplies) — a forward-reference key may not be present in the
            // dictionary yet at load time.
            built->set_base_resource_key(std::move(*based_on_key));
        }
        if (style_class.has_value() && !style_class->empty())
        {
            built->set_style_class(std::move(*style_class));
        }
        built->set_apply_to_derived_types(apply_to_derived_types);
        return built;
    }

    namespace
    {
        // Resolve the Property name against the TargetType and return its registration, throwing the
        // BindablePropertyConverter-equivalent error on a miss / an unsupported form. Shared by both
        // build_setter overloads.
        [[nodiscard]] const xaml_property_registry::property_entry& resolve_setter_property(
            maui::core::type_tag target_type, std::string_view property_name, const xaml_property_registry& properties,
            int line_number, int line_position)
        {
            if (property_name.empty())
            {
                throw xaml_parse_exception("Setter requires a Property", line_number, line_position);
            }
            // The 2-part qualified form (Type.PropertyName) and attached-property names (Grid.Row) need
            // the declaring-type resolution C# does by reflection — a W3 deferral; reject loudly.
            if (property_name.find('.') != std::string_view::npos)
            {
                throw xaml_parse_exception(
                    std::format("Setter Property \"{}\": qualified / attached-property setters are not "
                                "supported by the port yet (STATUS.md M7 deferrals)",
                                property_name),
                    line_number, line_position);
            }
            const xaml_property_registry::property_entry* entry = properties.find(target_type, property_name);
            if (entry == nullptr || entry->bindable_name.empty())
            {
                // BindablePropertyConverter: "Can't resolve {property} on {TargetType}." The bindable_name
                // must be non-empty (a Setter routes through apply_setter, which needs the descriptor name).
                throw xaml_parse_exception(
                    std::format("Can't resolve the Setter Property \"{}\" on the Style's TargetType, or it is not "
                                "a bindable property",
                                property_name),
                    line_number, line_position);
            }
            return *entry;
        }
    } // namespace

    maui::controls::setter build_setter(maui::core::type_tag target_type, std::string_view property_name,
                                        const std::string& value_text, const xaml_property_registry& properties,
                                        const xaml_converter_registry& converters, int line_number, int line_position)
    {
        const xaml_property_registry::property_entry& entry =
            resolve_setter_property(target_type, property_name, properties, line_number, line_position);

        // Convert the Value literal via the SAME converter the property uses (named implicitly by the
        // property's value_type) — a string property keeps the literal verbatim.
        if (entry.value_type == maui::core::type_tag::of<std::string>())
        {
            return maui::controls::setter::of_erased(entry.bindable_name, std::any{value_text});
        }
        std::any converted = converters.convert(entry.value_type, value_text);
        if (!converted.has_value())
        {
            // No converter registered for the property's value type (a documented converter deferral —
            // FontSize names, image sources, …). Fail loudly, like C#'s ConvertTo returning null.
            throw xaml_parse_exception(
                std::format("Cannot convert the Setter Value \"{}\" for property \"{}\": no converter is "
                            "registered for its value type",
                            value_text, property_name),
                line_number, line_position);
        }
        return maui::controls::setter::of_erased(entry.bindable_name, std::move(converted));
    }

    maui::controls::setter build_setter_from_value(maui::core::type_tag target_type, std::string_view property_name,
                                                   std::any value, const xaml_property_registry& properties,
                                                   int line_number, int line_position)
    {
        const xaml_property_registry::property_entry& entry =
            resolve_setter_property(target_type, property_name, properties, line_number, line_position);
        return maui::controls::setter::of_erased(entry.bindable_name, std::move(value));
    }
} // namespace maui::xaml
