#pragma once
// maui::xaml::i_markup_extension  <=  Microsoft.Maui.Controls.Xaml.IMarkupExtension
// maui::xaml::xaml_service_provider  <=  System.IServiceProvider (the XAML provider bundle)
// maui::xaml::markup_extension_registry  <=  the reflection-free substitute for resolving
//   "{Prefix:Name …}" to a markup-extension TYPE (C# resolves via GetElementType + Activator).
//
// The seam between the loader pipeline (the ExpandMarkups/ApplyProperties visitors, M7 wave 2 unit U3)
// and the markup-extension implementations ({Binding}, {StaticResource}, …, unit U5). C#'s
// IMarkupExtension.ProvideValue(IServiceProvider) returns object; the port returns std::any — the SAME
// boundary-confined erasure the setter/apply_setter seam already uses, unboxed by the property registry
// when applied. The C# IServiceProvider is a grab-bag service locator; the port carries the few services
// the v1 extensions actually consume as plain (non-owning) members — IProvideValueTarget (target object +
// property name), the resource-lookup element chain (ResourcesExtensions walks the parents), and the
// target property's expected type (for {OnPlatform}/{OnIdiom} conversion) — extend as wave-2 needs.
//
// An extension instance is minted per use by a registered factory taking the PARSED key=value attribute
// map (MarkupExtensionParser's tokenization output, xaml_parser.hpp) — properties a C# extension would
// expose as settable CLR properties arrive here as strings (already-resolved nested extensions arrive
// pre-boxed via the `values` map). A factory throws xaml_parse_exception on unknown/invalid attributes
// (C# throws XamlParseException through the visitor).

#include <any>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>

namespace maui::core
{
    class bindable_object;
} // namespace maui::core

namespace maui::controls
{
    class element;
} // namespace maui::controls

namespace maui::xaml
{
    // The services a ProvideValue call can consume (C# IServiceProvider + IProvideValueTarget +
    // the resource-lookup chain). All pointers are NON-owning borrows valid only during the call.
    struct xaml_service_provider
    {
        // IProvideValueTarget.TargetObject / TargetProperty (the property NAME — the port's registries
        // key properties by name, not by descriptor object).
        maui::core::bindable_object* target_object = nullptr;
        std::string target_property;
        // The element whose parent chain resolves {StaticResource}/{DynamicResource} lookups (the
        // closest enclosing element when the target itself is not an element — e.g. a setter).
        maui::controls::element* resource_scope = nullptr;
    };

    class i_markup_extension
    {
    public:
        virtual ~i_markup_extension() = default;

        // IMarkupExtension.ProvideValue: produce the value the surrounding property assignment applies.
        // The std::any is unboxed by the property registry (or interpreted by the caller for the special
        // cases C# special-cases too: a BindingBase result becomes SetBinding, a DynamicResource result
        // becomes set_dynamic_resource — see Setter.Apply / ApplyPropertiesVisitor).
        [[nodiscard]] virtual std::any provide_value(const xaml_service_provider& services) = 0;

    protected:
        i_markup_extension() = default;
        i_markup_extension(const i_markup_extension&) = default;
        i_markup_extension(i_markup_extension&&) = default;
        i_markup_extension& operator=(const i_markup_extension&) = default;
        i_markup_extension& operator=(i_markup_extension&&) = default;
    };

    // One parsed "{Name key=value, …}" use: positional/named string attributes plus any values that were
    // themselves markup extensions, already resolved to boxed values by the pipeline (keyed by the same
    // attribute name; a name present in `values` takes precedence over its raw string form).
    struct markup_extension_arguments
    {
        std::map<std::string, std::string> attributes;
        std::map<std::string, std::any> values;
    };

    // name ("Binding", "StaticResource", "x:Static", …) -> factory minting the extension for one use.
    // Mirrors the C# type resolution of "<name>Extension" without reflection (PROFILE §6).
    using markup_extension_factory =
        std::function<std::unique_ptr<i_markup_extension>(const markup_extension_arguments&)>;

    class markup_extension_registry
    {
    public:
        // The process-wide registry (function-local static, like the handler/type registries).
        [[nodiscard]] static markup_extension_registry& instance();

        // Register `factory` under `name` (and implicitly under "<name>Extension", matching C#'s
        // "Extension"-suffix tolerance). Later registrations replace earlier ones (test seam).
        void register_extension(std::string name, markup_extension_factory factory);

        // The factory for `name` (either spelling), or nullptr when unknown.
        [[nodiscard]] const markup_extension_factory* find(std::string_view name) const;

    private:
        std::map<std::string, markup_extension_factory, std::less<>> factories_;
    };
} // namespace maui::xaml
