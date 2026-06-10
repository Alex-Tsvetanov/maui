// maui::xaml::xaml_loader (xaml_loader.hpp), ported from src/Controls/src/Xaml/XamlLoader.cs:
// Load(view, xaml) -> load_into, Create(xaml, doNotThrow) -> load, and the private Visit sequence.
// The reflection-only surface (GetXamlForType's embedded-resource probing, RootAssembly,
// useDesignProperties) is out of scope — see the header.
#include "maui/xaml/xaml_loader.hpp"

#include <any>
#include <memory>
#include <string_view>
#include <utility>

#include "maui/core/bindable_object.hpp"
#include "maui/xaml/hydration_context.hpp"
#include "maui/xaml/i_markup_extension.hpp"
#include "maui/xaml/markup_extensions.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_node.hpp"
#include "maui/xaml/xaml_parser.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_standard_types.hpp"
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

        // XamlLoader.Visit — the fixed visitor sequence both entry points share.
        void visit(root_node& root, hydration_context& context)
        {
            xaml_node_visitor set_parents{[](i_xaml_node& node, i_xaml_node* parent) { node.set_parent(parent); }};
            root.accept(set_parents, nullptr); // set parents for {StaticResource}
            expand_markups_visitor expand{context};
            root.accept(expand, nullptr);
            prune_ignored_nodes_visitor prune;
            root.accept(prune, nullptr);
            namescoping_visitor namescope{context}; // set namescopes for {x:Reference}
            root.accept(namescope, nullptr);
            create_values_visitor create{context};
            root.accept(create, nullptr);
            register_x_names_visitor register_names{context};
            root.accept(register_names, nullptr);
            fill_resource_dictionaries_visitor fill{context};
            root.accept(fill, nullptr);
            apply_properties_visitor apply{context, /*stop_on_resource_dictionary=*/true};
            root.accept(apply, nullptr);
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
