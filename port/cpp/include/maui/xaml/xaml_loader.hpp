#pragma once
// maui::xaml::xaml_loader  <=  Microsoft.Maui.Controls.Xaml.XamlLoader (XamlLoader.cs)
// maui::xaml::xaml_load_result / xaml_load_options — the port-specific load surface (see below).
//
// The public runtime-XAML entry points, mirroring the two C# shapes the unit tests drive:
//   - load(xaml)            <=  XamlLoader.Create(string xaml, bool doNotThrow): mint the ROOT from
//     markup (a CreateValuesVisitor pass on the root node first, then the full visitor sequence —
//     the C# flow line for line);
//   - load_into(root, xaml) <=  XamlLoader.Load(object view, string xaml): hydrate markup into the
//     CALLER-OWNED root object (the LoadFromXaml extension's engine). The port resolves the root's
//     concrete type from the root ELEMENT NAME in the type registry (the no-reflection stand-in for
//     C#'s view.GetType(); the root element name must therefore be registered).
//
// The visitor sequence is XamlLoader.Visit verbatim: set-parents, ExpandMarkups, PruneIgnoredNodes,
// Namescoping, CreateValues, RegisterXNames, FillResourceDictionaries, ApplyProperties(stopOnRD).
// (SimplifyTypeExtension/RemoveDuplicateDesignNodes are design/optimization passes the port defers
// with useDesignProperties; LoadResources and the embedded-resource GetXamlForType overloads are
// out of scope — no assemblies to probe.)
//
// PORT-SPECIFIC RESULT (no GC): C# returns the inflated object and the GC keeps the tree alive
// through parent→child references; the port's tree-wiring APIs are non-owning (PROFILE §8), so the
// loader returns a xaml_load_result CARRYING the owners — the xaml_object_graph of created
// controls, the keep-alive list (resource dictionaries), and the load's event subscriptions
// ({AppThemeBinding} re-applies) — plus the root namescope for caller-side FindByName (the
// placement deviation documented in name_scope.hpp) and the parser's root warnings. Destroying the
// result destroys the loaded tree (subscriptions disconnect first — they are declared last).
//
// ERROR KNOB: options.exception_handler is HydrationContext.ExceptionHandler (C#'s doNotThrow):
// unset → the first load error THROWS xaml_parse_exception; set → errors are handed to it and the
// load continues past the failed node (Create's inflatedView may then be null → empty result).
//
// {Binding} results route through the replaceable xaml_binding_applier hook
// (xaml_binding_applier.hpp) — the default REJECTS loudly; the runtime-binding unit registers the
// real SetBinding port (the stitch lands with that unit).

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "maui/core/bindable_object.hpp"
#include "maui/core/event.hpp"
#include "maui/xaml/hydration_context.hpp"
#include "maui/xaml/name_scope.hpp"
#include "maui/xaml/xaml_node.hpp"
#include "maui/xaml/xaml_object_graph.hpp"

namespace maui::controls
{
    class application;
} // namespace maui::controls

namespace maui::xaml
{
    class xaml_type_registry;
    class xaml_property_registry;
    class xaml_converter_registry;
    class markup_extension_registry;

    // ---- xaml_load_options — the knobs C# reads from statics/ambient state ----
    struct xaml_load_options
    {
        // Application.Current's stand-in ({StaticResource} app-level fallback, {AppThemeBinding}
        // re-apply). Non-owning; must outlive the loaded tree when set.
        maui::controls::application* application = nullptr;
        // HydrationContext.ExceptionHandler — the doNotThrow knob (see the header note).
        hydration_context::exception_handler exception_handler;
        // XamlParser.PrefixesToIgnore's DeviceInfo.Platform seam (parse_options.target_platform).
        std::optional<std::string> target_platform;
        // The explicit registries (null = the process-wide defaults, seeded once with the standard
        // registrations — register_standard_xaml + register_standard_markup_extensions).
        const xaml_type_registry* types = nullptr;
        const xaml_property_registry* properties = nullptr;
        const xaml_converter_registry* converters = nullptr;
        const markup_extension_registry* extensions = nullptr;
    };

    // ---- xaml_load_result — the owners + namescope one load produced ----
    class xaml_load_result
    {
    public:
        // The owner of every control the load created (root() is set by load(); load_into's root
        // stays caller-owned and is NOT in the graph). Destroy the result, destroy the tree.
        xaml_object_graph graph;
        // The root namescope (x:Name registrations) — the caller-side FindByName surface.
        std::shared_ptr<name_scope> root_scope;
        // The parser's accumulated warnings (RootNode warnings — unknown property-element
        // attributes and the like).
        std::vector<root_node::warning> warnings;
        // Non-control objects the tree references (standalone resource dictionaries).
        std::vector<std::shared_ptr<void>> keep_alive;
        // The load's event subscriptions ({AppThemeBinding}'s RequestedThemeChanged re-applies).
        // Declared last: destroyed (disconnected) FIRST, before the graph tears the targets down.
        std::vector<maui::core::scoped_connection> subscriptions;

        // The loaded root as the concrete control type (load() only), or nullptr.
        template <class TControl> [[nodiscard]] std::shared_ptr<TControl> root_as() const
        {
            return graph.root_as<TControl>();
        }

        // Element.FindByName over the root scope: the registered control, or nullptr.
        template <class TControl> [[nodiscard]] std::shared_ptr<TControl> find_by_name(std::string_view name) const
        {
            return root_scope != nullptr ? root_scope->find_by_name_as<TControl>(name) : nullptr;
        }
    };

    // ---- xaml_loader  <=  Microsoft.Maui.Controls.Xaml.XamlLoader (static class) ----
    class xaml_loader
    {
    public:
        xaml_loader() = delete;

        // XamlLoader.Create: parse + mint the root object from the markup's root element, then run
        // the full hydration. result.graph.root() / root_as<T>() is the inflated view (null when
        // creation failed under an exception_handler — C# returns null then).
        [[nodiscard]] static xaml_load_result load(std::string_view xaml, const xaml_load_options& options = {});

        // XamlLoader.Load(view, xaml): hydrate into the caller-owned root object. The root element
        // name must resolve in the type registry to root's concrete type (see the header note).
        [[nodiscard]] static xaml_load_result load_into(maui::core::bindable_object& root, std::string_view xaml,
                                                        const xaml_load_options& options = {});
    };
} // namespace maui::xaml
