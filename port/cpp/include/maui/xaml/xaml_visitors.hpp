#pragma once
// maui::xaml — the loader's visitor pipeline (M7 wave 2)  <=  src/Controls/src/Xaml/*Visitor.cs:
//
//   expand_markups_visitor             <=  Microsoft.Maui.Controls.Xaml.ExpandMarkupsVisitor
//   prune_ignored_nodes_visitor        <=  Microsoft.Maui.Controls.Xaml.PruneIgnoredNodesVisitor
//   namescoping_visitor                <=  Microsoft.Maui.Controls.Xaml.NamescopingVisitor
//   create_values_visitor              <=  Microsoft.Maui.Controls.Xaml.CreateValuesVisitor
//   register_x_names_visitor           <=  Microsoft.Maui.Controls.Xaml.RegisterXNamesVisitor
//   fill_resource_dictionaries_visitor <=  Microsoft.Maui.Controls.Xaml.FillResourceDictionariesVisitor
//   apply_properties_visitor           <=  Microsoft.Maui.Controls.Xaml.ApplyPropertiesVisitor
//
// They run over one shared hydration_context in XamlLoader.Visit's order (the parent-setting
// xaml_node_visitor first, then the seven above — the loader, xaml_loader.hpp, owns the sequence).
// Reflection-free substitutions, each documented at its class:
//   - type/property/converter lookups go through the context's explicit registries (PROFILE §6);
//   - a markup node is NOT replaced by an extension ElementNode (C# ExpandMarkups swaps the node and
//     CreateValues/ApplyProperties instantiate it reflectively): expansion mints the extension via
//     markup_extension_registry factories and keeps it in context values keyed by the (retained)
//     markup node; apply_properties_visitor calls provide_value when the value is applied — the same
//     point in time as C#'s ApplyPropertiesVisitor.ProvideValue;
//   - the namescopes stay on the NODES (element_node::scope_ref) and the loader result carries the
//     root scope — the port's bindable_object has no attached-property bag for C#'s
//     NameScope.SetNameScope (the placement deviation documented in name_scope.hpp);
//   - events (Clicked="OnClicked"), attached properties (Grid.Row), x:Arguments/x:FactoryMethod,
//     Style/DataTemplate element forms and RD Source loading are documented M7 deferrals — each
//     fails LOUDLY through the xaml_parse_exception channel rather than silently dropping markup.

#include <any>
#include <optional>
#include <string>
#include <unordered_map>

#include "maui/core/bindable_object.hpp"
#include "maui/core/type_tag.hpp"
#include "maui/xaml/hydration_context.hpp"
#include "maui/xaml/xaml_node.hpp"

namespace maui::xaml
{
    // ---- expand_markups_visitor  <=  ExpandMarkupsVisitor (+ its MarkupExpansionParser) ----
    // Bottom-up. For every markup node that IS a property value (collection-item markup is left
    // alone, like C#), tokenizes the "{…}" string with xaml_parser's MarkupExpressionParser
    // primitives (match_markup / get_next_piece / parse_markup_name), resolves the extension factory
    // in markup_extension_registry::instance() ("MarkupExtension not found for {match}" on a miss —
    // the "<name>Extension" suffix tolerance lives in the registry), and stores the MINTED extension
    // in context values keyed by the markup node. Nested extensions ("Converter={StaticResource x}")
    // recurse: the inner extension is minted too and its provide_value result lands in the outer
    // factory's `values` map when the outer is provided — deferred to apply time, exactly when C#
    // provides them. Skips: x:Key / x:TypeArguments / x:FactoryMethod / x:Name / x:DataType.
    class expand_markups_visitor final : public i_xaml_node_visitor
    {
    public:
        explicit expand_markups_visitor(hydration_context& context) : context_(&context)
        {
        }

        [[nodiscard]] tree_visiting_mode visiting_mode() const override
        {
            return tree_visiting_mode::bottom_up;
        }
        [[nodiscard]] bool stop_on_data_template() const override
        {
            return false;
        }
        [[nodiscard]] bool visit_node_on_data_template() const override
        {
            return true;
        }
        [[nodiscard]] bool stop_on_resource_dictionary() const override
        {
            return false;
        }

        void visit(value_node& node, i_xaml_node* parent_node) override;
        void visit(markup_node& node, i_xaml_node* parent_node) override;
        void visit(element_node& node, i_xaml_node* parent_node) override;
        void visit(root_node& node, i_xaml_node* parent_node) override;
        void visit(list_node& node, i_xaml_node* parent_node) override;
        [[nodiscard]] bool skip_children(i_xaml_node& node, i_xaml_node* parent_node) override;
        [[nodiscard]] bool is_resource_dictionary(element_node& node) override;

    private:
        hydration_context* context_;
    };

    // ---- prune_ignored_nodes_visitor  <=  PruneIgnoredNodesVisitor ----
    // Top-down. Collects mc:Ignorable prefix lists onto the parent's IgnorablePrefixes and removes
    // every property / collection item whose prefix (or whose element's namespace prefix) is
    // ignorable in scope. (The useDesignProperties knob is not ported — the port has no design mode.)
    class prune_ignored_nodes_visitor final : public i_xaml_node_visitor
    {
    public:
        prune_ignored_nodes_visitor() = default;

        [[nodiscard]] tree_visiting_mode visiting_mode() const override
        {
            return tree_visiting_mode::top_down;
        }
        [[nodiscard]] bool stop_on_data_template() const override
        {
            return false;
        }
        [[nodiscard]] bool visit_node_on_data_template() const override
        {
            return true;
        }
        [[nodiscard]] bool stop_on_resource_dictionary() const override
        {
            return false;
        }

        void visit(value_node& node, i_xaml_node* parent_node) override;
        void visit(markup_node& node, i_xaml_node* parent_node) override;
        void visit(element_node& node, i_xaml_node* parent_node) override;
        void visit(root_node& node, i_xaml_node* parent_node) override;
        void visit(list_node& node, i_xaml_node* parent_node) override;
        [[nodiscard]] bool skip_children(i_xaml_node& node, i_xaml_node* parent_node) override;
        [[nodiscard]] bool is_resource_dictionary(element_node& node) override;
    };

    // ---- namescoping_visitor  <=  NamescopingVisitor ----
    // Top-down. Assigns element_node::scope_ref: the root starts a fresh scope; an element under a
    // DataTemplate (_CreateContent), under a <Style>, or a VisualStateGroup in a list starts its own;
    // everything else SHARES the parent's ref object (the name_scope_ref indirection).
    class namescoping_visitor final : public i_xaml_node_visitor
    {
    public:
        explicit namescoping_visitor(hydration_context& context);

        [[nodiscard]] tree_visiting_mode visiting_mode() const override
        {
            return tree_visiting_mode::top_down;
        }
        [[nodiscard]] bool stop_on_data_template() const override
        {
            return false;
        }
        [[nodiscard]] bool visit_node_on_data_template() const override
        {
            return true;
        }
        [[nodiscard]] bool stop_on_resource_dictionary() const override
        {
            return false;
        }

        void visit(value_node& node, i_xaml_node* parent_node) override;
        void visit(markup_node& node, i_xaml_node* parent_node) override;
        void visit(element_node& node, i_xaml_node* parent_node) override;
        void visit(root_node& node, i_xaml_node* parent_node) override;
        void visit(list_node& node, i_xaml_node* parent_node) override;
        [[nodiscard]] bool skip_children(i_xaml_node& node, i_xaml_node* parent_node) override;
        [[nodiscard]] bool is_resource_dictionary(element_node& node) override;

    private:
        std::unordered_map<const i_xaml_node*, std::shared_ptr<name_scope_ref>> scopes_;
    };

    // ---- create_values_visitor  <=  CreateValuesVisitor ----
    // Bottom-up; stops on DataTemplates. Hydrates context values:
    //   - a value node's string;
    //   - an x2009 language primitive (<x:String>/<x:Int32>/<x:Double>/<x:Boolean>) parsed from its
    //     single text item (TryParse failures fall back to the default value, like C#);
    //   - <ResourceDictionary> minted as a real maui::controls::resource_dictionary (kept alive by
    //     the context — it is not a bindable_object, so it lives outside the graph), stored as a
    //     NON-owning resource_dictionary* value;
    //   - any other element resolved in the context's xaml_type_registry and default-constructed
    //     (Activator.CreateInstance), OWNED by the context's xaml_object_graph. A registry miss
    //     raises "Type {name} not found in xmlns {ns}"; x:Arguments / x:FactoryMethod raise the
    //     documented deferral error (the port registers default-constructible types only).
    // The root node takes the context's root_element (XamlLoader.RuntimeRootNode.Root); its concrete
    // type is resolved from the root ELEMENT NAME in the registry (the no-reflection stand-in for
    // C#'s Root.GetType() — load_into therefore requires a registered root element name).
    class create_values_visitor final : public i_xaml_node_visitor
    {
    public:
        explicit create_values_visitor(hydration_context& context) : context_(&context)
        {
        }

        [[nodiscard]] tree_visiting_mode visiting_mode() const override
        {
            return tree_visiting_mode::bottom_up;
        }
        [[nodiscard]] bool stop_on_data_template() const override
        {
            return true;
        }
        [[nodiscard]] bool visit_node_on_data_template() const override
        {
            return false;
        }
        [[nodiscard]] bool stop_on_resource_dictionary() const override
        {
            return false;
        }

        void visit(value_node& node, i_xaml_node* parent_node) override;
        void visit(markup_node& node, i_xaml_node* parent_node) override;
        void visit(element_node& node, i_xaml_node* parent_node) override;
        void visit(root_node& node, i_xaml_node* parent_node) override;
        void visit(list_node& node, i_xaml_node* parent_node) override;
        [[nodiscard]] bool skip_children(i_xaml_node& node, i_xaml_node* parent_node) override;
        [[nodiscard]] bool is_resource_dictionary(element_node& node) override;

    private:
        hydration_context* context_;
    };

    // ---- register_x_names_visitor  <=  RegisterXNamesVisitor ----
    // Top-down; stops on DataTemplates. Registers every x:Name value node's parent object in the
    // parent's namescope; a duplicate raises C#'s "An element with the name \"{name}\" already
    // exists in this NameScope". (C# also seeds Element.StyleId from x:Name — the port's element has
    // no StyleId yet; documented deviation.)
    class register_x_names_visitor final : public i_xaml_node_visitor
    {
    public:
        explicit register_x_names_visitor(hydration_context& context) : context_(&context)
        {
        }

        [[nodiscard]] tree_visiting_mode visiting_mode() const override
        {
            return tree_visiting_mode::top_down;
        }
        [[nodiscard]] bool stop_on_data_template() const override
        {
            return true;
        }
        [[nodiscard]] bool visit_node_on_data_template() const override
        {
            return false;
        }
        [[nodiscard]] bool stop_on_resource_dictionary() const override
        {
            return false;
        }

        void visit(value_node& node, i_xaml_node* parent_node) override;
        void visit(markup_node& node, i_xaml_node* parent_node) override;
        void visit(element_node& node, i_xaml_node* parent_node) override;
        void visit(root_node& node, i_xaml_node* parent_node) override;
        void visit(list_node& node, i_xaml_node* parent_node) override;
        [[nodiscard]] bool skip_children(i_xaml_node& node, i_xaml_node* parent_node) override;
        [[nodiscard]] bool is_resource_dictionary(element_node& node) override;

    private:
        hydration_context* context_;
    };

    // ---- fill_resource_dictionaries_visitor  <=  FillResourceDictionariesVisitor ----
    // Top-down; stops on DataTemplates. Populates resource dictionaries BEFORE the main apply pass
    // (so {StaticResource} finds them):
    //   - a <ResourceDictionary> that is the value of a "Resources" / "*.Resources" property is
    //     RETARGETED at the owning element's own resources() (C# assigns the RD object to
    //     VisualElement.Resources; the port's element owns its dictionary, so the node's value is
    //     rebound to it and the standalone dictionary minted by create_values is dropped);
    //   - each child of a keyless RD is fully applied through a nested apply_properties_visitor run
    //     (stopOnResourceDictionary: false), which adds it under its x:Key — the fill traversal
    //     itself skips those subtrees (SkipChildren), exactly like C#.
    class fill_resource_dictionaries_visitor final : public i_xaml_node_visitor
    {
    public:
        explicit fill_resource_dictionaries_visitor(hydration_context& context) : context_(&context)
        {
        }

        [[nodiscard]] tree_visiting_mode visiting_mode() const override
        {
            return tree_visiting_mode::top_down;
        }
        [[nodiscard]] bool stop_on_data_template() const override
        {
            return true;
        }
        [[nodiscard]] bool visit_node_on_data_template() const override
        {
            return false;
        }
        [[nodiscard]] bool stop_on_resource_dictionary() const override
        {
            return false;
        }

        void visit(value_node& node, i_xaml_node* parent_node) override;
        void visit(markup_node& node, i_xaml_node* parent_node) override;
        void visit(element_node& node, i_xaml_node* parent_node) override;
        void visit(root_node& node, i_xaml_node* parent_node) override;
        void visit(list_node& node, i_xaml_node* parent_node) override;
        [[nodiscard]] bool skip_children(i_xaml_node& node, i_xaml_node* parent_node) override;
        [[nodiscard]] bool is_resource_dictionary(element_node& node) override;

    private:
        hydration_context* context_;
    };

    // ---- apply_properties_visitor  <=  ApplyPropertiesVisitor ----
    // Bottom-up; stops on DataTemplates; the loader's MAIN pass runs with stopOnResourceDictionary
    // (RD subtrees were already handled by the fill pass). Applies every hydrated value to its
    // target, the port of SetPropertyValue/TrySetPropertyValue's route chain:
    //   - x:* directive properties and mc:Ignorable are skipped (the C# Skips list);
    //   - a markup node provides its minted extension's value (provide_value with a
    //     xaml_service_provider built from the node ancestry), then the result is applied;
    //   - a controls::dynamic_resource result routes to element::set_dynamic_resource over the
    //     property's bindable_name (TrySetDynamicResource);
    //   - a binding_request result routes to the registered binding applier hook — the M7 default
    //     REJECTS it (xaml_binding_applier.hpp lands with the loader; until then a clear
    //     xaml_parse_exception), the runtime-binding unit registers the real one (TrySetBinding);
    //   - an app_theme_binding result applies its picked slot now and re-applies on every
    //     application::requested_theme_changed (AppThemeBinding.Apply/ApplyCore), the subscription
    //     accumulating in the context;
    //   - an EMPTY std::any (OnPlatform/OnIdiom "no value") SKIPS the assignment; a xaml_null result
    //     is the documented {x:Null} load failure (markup_extensions.hpp value-form contract);
    //   - a std::string value converts against the registered property's value type
    //     (try_set_from_text — C#'s ConvertTo); other values apply typed (TrySetValue);
    //   - collection items route x:Key'd values into a ResourceDictionary source, element text into
    //     the [ContentProperty] value name, and child elements into the registered add_child sink
    //     (the IEnumerable+Add walk, including the named <Layout.Children> property-element form);
    //   - everything else raises C#'s "Cannot assign property" XamlParseException.
    class apply_properties_visitor final : public i_xaml_node_visitor
    {
    public:
        explicit apply_properties_visitor(hydration_context& context, bool stop_on_resource_dictionary = false)
            : context_(&context), stop_on_resource_dictionary_(stop_on_resource_dictionary)
        {
        }

        [[nodiscard]] tree_visiting_mode visiting_mode() const override
        {
            return tree_visiting_mode::bottom_up;
        }
        [[nodiscard]] bool stop_on_data_template() const override
        {
            return true;
        }
        [[nodiscard]] bool visit_node_on_data_template() const override
        {
            return true;
        }
        [[nodiscard]] bool stop_on_resource_dictionary() const override
        {
            return stop_on_resource_dictionary_;
        }

        void visit(value_node& node, i_xaml_node* parent_node) override;
        void visit(markup_node& node, i_xaml_node* parent_node) override;
        void visit(element_node& node, i_xaml_node* parent_node) override;
        void visit(root_node& node, i_xaml_node* parent_node) override;
        void visit(list_node& node, i_xaml_node* parent_node) override;
        [[nodiscard]] bool skip_children(i_xaml_node& node, i_xaml_node* parent_node) override;
        [[nodiscard]] bool is_resource_dictionary(element_node& node) override;

        // ApplyPropertiesVisitor.SetPropertyValue's reusable core (public static in C# — Hot Reload
        // and Setter.Apply call it): apply one already-provided `value` to `target`'s property
        // `local_name`, routing markers exactly as the visitor does (see the class comment). Throws
        // xaml_parse_exception on failure; `x_key` is the value's x:Key when it came from an element
        // node (the Resources fallback needs it); line/position feed the error messages.
        static void apply_value(hydration_context& context, maui::core::bindable_object& target,
                                maui::core::type_tag target_type, const std::string& local_name, const std::any& value,
                                const std::optional<std::string>& x_key, int line_number, int line_position);

    private:
        void visit_property_value(const std::any& value, i_xaml_node& node, element_node& parent_element,
                                  const xml_name& property_name);
        void visit_collection_item(const std::any& value, i_xaml_node& node, i_xaml_node& parent_node);

        // W3 — resolve a <Setter> against the enclosing <Style>'s TargetType (walking the node's parent
        // chain to the Style element node) and add the built setter to the Style's minted shell. The
        // reflection-free substitute for C#'s BindablePropertyConverter reading the parent Style's
        // TargetType via IProvideParentValues.ParentObjects.
        void apply_setter_to_parent_style(element_node& node, i_xaml_node* parent_node);

        // <VisualElement.Triggers><Trigger Property=.. Value=..><Setter/></Trigger>: walk the parent chain to
        // the owning view, resolve Property against its type, build an erased_property_trigger (condition +
        // the <Setter> children), and add it to the view's Triggers collection. The trigger analog of
        // apply_setter_to_parent_style; a <Trigger> outside a view is inert.
        void apply_trigger_to_parent_view(element_node& node, i_xaml_node* parent_node);

        // W4 — ApplyPropertiesVisitor.SetTemplate: at a <DataTemplate>'s _CreateContent body node,
        // install the parent template's per-item loader (a closure owning a clone of `body_node` + a
        // value snapshot of the load environment), so each stamp lazily inflates a fresh copy of the
        // captured body. `parent_node` is the <DataTemplate> element node carrying the minted template.
        void set_template(element_node& body_node, i_xaml_node* parent_node);

        hydration_context* context_;
        bool stop_on_resource_dictionary_;
    };
} // namespace maui::xaml
