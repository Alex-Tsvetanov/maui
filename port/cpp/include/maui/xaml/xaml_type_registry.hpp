#pragma once
// maui::xaml::xaml_type_registry — the XAML element-name → factory table (M7 wave 1).
//
// C# counterpart: XamlParser.GetElementType (src/Controls/src/Xaml/XamlParser.cs) resolves an element
// name within its xmlns to a System.Type by reflection, and CreateValuesVisitor.Visit(ElementNode)
// instantiates it with Activator.CreateInstance. C++23 has no reflection (PROFILE §6), so the lookup
// table is built by EXPLICIT registration instead: register_type<TControl>("Button") records a
// default-construct factory plus the concrete type's type_tag (which keys the xaml_property_registry),
// keyed by the XAML-facing element name. Names are the C# XAML names — PascalCase, exactly as they
// appear in markup (`<Button …>`); the snake_case port names live behind the factories.
//
// xmlns notion — kept deliberately simple (the two namespaces the v1 loader understands): the default
// MAUI namespace and the XAML-language "x" namespace, modeled as the xaml_namespace enum rather than
// full URI mapping. Registrations live in the maui namespace; the x namespace exists so the loader can
// key its lookups uniformly. C#'s x-namespace LANGUAGE PRIMITIVES (x:String, x:Int32, …) are NOT
// registry types there either — CreateValuesVisitor special-cases IsXaml2009LanguagePrimitive BEFORE
// the GetElementType path — so they stay a loader concern here too (via the converter registry).
//
// Error strategy: lookups are throw-free — find()/create() return nullptr on a miss, mirroring
// GetElementType's "return null + out exception" convention. The M7 loader turns a miss into a thrown
// xaml_parse_exception ("Type {name} not found in xmlns {ns}"), matching XamlParser.cs. See
// xaml_parse_exception.hpp for the full error-channel decision.
//
// OWNERSHIP DECISION (loader-side, PROFILE §8). The factory returns shared_ptr<bindable_object>: the
// creator OWNS what it creates. The existing tree-wiring APIs all take NON-owning references — the
// caller owns every child's lifetime (content_page::set_content(i_view&), layout<>::add(i_view&),
// window::set_content(element&), navigation_page::push(content_page&)) — so parenting a loaded child
// transfers NO ownership. The M7 loader therefore keeps every object it creates alive in a
// xaml_object_graph (xaml_object_graph.hpp): the graph is the single owner of the loaded tree, and the
// controls' parent/child links merely borrow. Destroying the graph destroys the whole tree.
//
// Self-registration (the MAUI_REGISTER_HANDLER pattern): a static xaml_type_registrar<TControl> — or
// the MAUI_XAML_REGISTER_TYPE sugar — registers into the process-wide default_xaml_type_registry() at
// load time, for app-defined custom controls. Same tree-shaking caveat as handler_registry (PROFILE
// §6): a static library drops an unreferenced TU and its registrar with it; put self-registering TUs in
// a CMake OBJECT library or link with --whole-archive. The standard control set is registered
// EXPLICITLY instead (xaml_standard_types.hpp), the predictable primitive.

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "maui/core/bindable_object.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::xaml
{
    // The xmlns a XAML element name is resolved in (XamlParser.Namespaces.cs):
    //   maui — the default namespace, xmlns="http://schemas.microsoft.com/dotnet/2021/maui" (MauiUri)
    //   x    — the XAML language namespace, xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml" (X2009Uri)
    enum class xaml_namespace : std::uint8_t
    {
        maui,
        x,
    };

    class xaml_type_registry
    {
    public:
        // Default-constructs one control instance (the Activator.CreateInstance analog). The returned
        // shared_ptr is the OWNING handle — see the ownership decision in the header comment.
        using factory = std::function<std::shared_ptr<maui::core::bindable_object>()>;

        struct registration
        {
            factory create;            // instantiate the control
            maui::core::type_tag type; // the concrete type's identity (keys the xaml_property_registry)
        };

        // Register (or replace) `TControl` under the XAML element name `name` in namespace `ns`.
        template <class TControl> void register_type(std::string name, xaml_namespace ns = xaml_namespace::maui)
        {
            static_assert(std::is_base_of_v<maui::core::bindable_object, TControl>,
                          "TControl must derive maui::core::bindable_object");
            static_assert(std::is_default_constructible_v<TControl>,
                          "TControl must be default-constructible (Activator.CreateInstance analog)");
            entries_for(ns).insert_or_assign(std::move(name),
                                             registration{.create = [] { return std::make_shared<TControl>(); },
                                                          .type = maui::core::type_tag::of<TControl>()});
        }

        // The registration for `name`, or nullptr if unknown (no throw — see the error strategy above).
        // The pointer stays valid across later registrations (node-based map), until `name` is replaced.
        [[nodiscard]] const registration* find(std::string_view name, xaml_namespace ns = xaml_namespace::maui) const;

        // Instantiate the control registered under `name`, or nullptr if unknown (no throw).
        [[nodiscard]] std::shared_ptr<maui::core::bindable_object> create(
            std::string_view name, xaml_namespace ns = xaml_namespace::maui) const;

        [[nodiscard]] bool is_registered(std::string_view name, xaml_namespace ns = xaml_namespace::maui) const;

    private:
        using name_map = std::unordered_map<std::string, registration>;

        [[nodiscard]] name_map& entries_for(xaml_namespace ns)
        {
            return ns == xaml_namespace::x ? x_entries_ : maui_entries_;
        }
        [[nodiscard]] const name_map& entries_for(xaml_namespace ns) const
        {
            return ns == xaml_namespace::x ? x_entries_ : maui_entries_;
        }

        name_map maui_entries_;
        name_map x_entries_;
    };

    // The process-wide default registry the self-registration helpers populate, and the M7 loader
    // resolves from when no explicit registry is threaded through. Meyers singleton — no
    // static-init-order fiasco even when registrars run during dynamic initialization.
    [[nodiscard]] xaml_type_registry& default_xaml_type_registry();

    // Macro-free self-registration primitive: a `static const xaml_type_registrar<control>` at
    // namespace scope registers the type in the default registry at load time, via its constructor.
    // Same OBJECT-library tree-shaking caveat as handler_registrar (see the header comment).
    template <class TControl> struct xaml_type_registrar
    {
        // noexcept + a const char* parameter: this runs during dynamic initialization
        // (bugprone-throwing-static-initialization), so neither the constructor nor its argument
        // conversions may throw (string_view's const char* constructor is not noexcept). The only
        // failure mode is OOM on the map insertion, where terminating at load is acceptable.
        explicit xaml_type_registrar(const char* name, xaml_namespace ns = xaml_namespace::maui) noexcept
        {
            default_xaml_type_registry().register_type<TControl>(std::string{name}, ns);
        }
    };
} // namespace maui::xaml

// MAUI_XAML_REGISTER_TYPE("Name", control_type): one-line opt-in self-registration, sugar over
// xaml_type_registrar (same OBJECT-library caveat). MAUI_-prefixed macros are allow-listed in the
// project's .clang-tidy.
#define MAUI_XAML_DETAIL_CONCAT_IMPL(a, b) a##b
#define MAUI_XAML_DETAIL_CONCAT(a, b) MAUI_XAML_DETAIL_CONCAT_IMPL(a, b)
#define MAUI_XAML_REGISTER_TYPE(Name, TControl)                                                                        \
    namespace                                                                                                          \
    {                                                                                                                  \
        const ::maui::xaml::xaml_type_registrar<TControl> MAUI_XAML_DETAIL_CONCAT(maui_xaml_type_registrar_,           \
                                                                                  __LINE__){Name};                     \
    }
