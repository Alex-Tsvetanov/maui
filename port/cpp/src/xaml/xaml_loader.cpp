// maui::xaml::xaml_loader (xaml_loader.hpp), ported from src/Controls/src/Xaml/XamlLoader.cs:
// Load(view, xaml) -> load_into, Create(xaml, doNotThrow) -> load, and the private Visit sequence.
// The reflection-only surface (GetXamlForType's embedded-resource probing, RootAssembly,
// useDesignProperties) is out of scope — see the header.
#include "maui/xaml/xaml_loader.hpp"

#include <any>
#include <format>
#include <memory>
#include <string_view>
#include <utility>

#include "maui/core/bindable_object.hpp"
#include "maui/xaml/hydration_context.hpp"
#include "maui/xaml/i_markup_extension.hpp"
#include "maui/xaml/markup_extensions.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_node.hpp"
#include "maui/xaml/xaml_parse_exception.hpp"
#include "maui/xaml/xaml_parser.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_standard_types.hpp"
#include "maui/xaml/xaml_template_inflater.hpp"
#include "maui/xaml/xaml_type_registry.hpp"
#include "maui/xaml/xaml_visitors.hpp"

namespace maui::xaml
{
    namespace
    {
        // Seed the process-wide defaults once (C# reaches types/properties/converters/extensions by
        // reflection; the port's loader performs the explicit one-call setup). Idempotent.
        void ensure_default_registrations()
        {
            static const bool registered = [] {
                register_standard_xaml(default_xaml_type_registry(), default_xaml_property_registry(),
                                       default_xaml_converter_registry());
                register_standard_markup_extensions();
                return true;
            }();
            (void)registered;
        }

        [[nodiscard]] hydration_context make_context(const xaml_load_options& options)
        {
            ensure_default_registrations();
            hydration_context context{
                options.types != nullptr ? *options.types : default_xaml_type_registry(),
                options.properties != nullptr ? *options.properties : default_xaml_property_registry(),
                options.converters != nullptr ? *options.converters : default_xaml_converter_registry(),
                options.extensions != nullptr ? *options.extensions : markup_extension_registry::instance(),
                options.exception_handler};
            context.application = options.application;
            return context;
        }

    } // namespace

    // XamlLoader.Visit — the fixed visitor sequence both the top-level load AND each DataTemplate stamp
    // share (W4). Extracted from the loader's anon namespace into a public free function so
    // inflate_template_body (xaml_template_inflater) can replay the SAME sequence on a cloned template
    // body. Declared in xaml_template_inflater.hpp.
    void run_hydration_pipeline(i_xaml_node& node, hydration_context& context)
    {
        xaml_node_visitor set_parents{[](i_xaml_node& n, i_xaml_node* parent) { n.set_parent(parent); }};
        node.accept(set_parents, nullptr); // set parents for {StaticResource}
        expand_markups_visitor expand{context};
        node.accept(expand, nullptr);
        prune_ignored_nodes_visitor prune;
        node.accept(prune, nullptr);
        namescoping_visitor namescope{context}; // set namescopes for {x:Reference}
        node.accept(namescope, nullptr);
        create_values_visitor create{context};
        node.accept(create, nullptr);
        register_x_names_visitor register_names{context};
        node.accept(register_names, nullptr);
        fill_resource_dictionaries_visitor fill{context};
        node.accept(fill, nullptr);
        apply_properties_visitor apply{context, /*stop_on_resource_dictionary=*/true};
        node.accept(apply, nullptr);

        // Place deferred attached properties (Grid.Row/Column/Span). The apply pass has now parented
        // every child into its layout, so each child's owning grid is reachable (logical_parent set);
        // these closures could not run earlier because a child's attached attribute is applied before
        // add() parents it. (See hydration_context::deferred_attached / try_apply_attached_property.)
        for (auto& place : context.deferred_attached())
        {
            place();
        }
        context.deferred_attached().clear();
    }

    namespace
    {
        // XamlLoader.Visit — the fixed visitor sequence both entry points share (delegates to the public
        // run_hydration_pipeline so the template stamp replays the identical sequence).
        void visit(root_node& root, hydration_context& context)
        {
            run_hydration_pipeline(root, context);
        }

        // Drain the context's accumulated owners + the root's scope/warnings into the result.
        [[nodiscard]] xaml_load_result make_result(root_node& root, hydration_context& context)
        {
            xaml_load_result result;
            result.graph = std::move(context.graph());
            result.root_scope = root.scope_ref() != nullptr ? root.scope_ref()->scope : nullptr;
            result.warnings = std::move(root.warnings());
            result.keep_alive = std::move(context.kept_alive());
            result.subscriptions = std::move(context.subscriptions());
            return result;
        }
    } // namespace

    xaml_load_result xaml_loader::load(std::string_view xaml, const xaml_load_options& options)
    {
        const std::shared_ptr<root_node> root = xaml_parser::parse(xaml, {.target_platform = options.target_platform});
        hydration_context context = make_context(options);

        // XamlLoader.Create: a direct CreateValuesVisitor pass on the root ELEMENT first — it mints
        // the root object (owned by the graph) before the full hydration runs.
        create_values_visitor create{context};
        create.visit(static_cast<element_node&>(*root), nullptr);
        const std::any* created = context.try_get_value(*root);
        if (created != nullptr && std::any_cast<std::shared_ptr<maui::core::bindable_object>>(created) == nullptr)
        {
            // The root hydrated to a NON-CONTROL payload — a <ResourceDictionary> or an x:String/…
            // language primitive. C# Create returns that object; the port's result is control-typed,
            // so this is a loud deferral rather than a silently empty result (the handler knob still
            // collects it).
            context.handle(xaml_parse_exception(
                std::format("Loading a non-control root element ({}) is not supported by the port yet "
                            "(STATUS.md M7 deferrals)",
                            root->type().name()),
                root->line_number(), root->line_position()));
            return make_result(*root, context);
        }
        const auto* owner =
            created != nullptr ? std::any_cast<std::shared_ptr<maui::core::bindable_object>>(created) : nullptr;
        if (owner == nullptr)
        {
            // Creation failed under an exception_handler (C#'s doNotThrow Create returns null).
            return make_result(*root, context);
        }
        context.set_root_element(owner->get()); // visitorContext.RootElement = inflatedView
        context.graph().set_root(*owner);

        visit(*root, context);
        return make_result(*root, context);
    }

    xaml_load_result xaml_loader::load_into(maui::core::bindable_object& root_object, std::string_view xaml,
                                            const xaml_load_options& options)
    {
        const std::shared_ptr<root_node> root = xaml_parser::parse(xaml, {.target_platform = options.target_platform});
        hydration_context context = make_context(options);
        context.set_root_element(&root_object); // RuntimeRootNode.Root = view
        visit(*root, context);
        return make_result(*root, context);
    }
} // namespace maui::xaml
