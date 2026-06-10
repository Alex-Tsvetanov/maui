// maui::xaml — the loader's visitor pipeline (xaml_visitors.hpp), ported from
// src/Controls/src/Xaml/{ExpandMarkupsVisitor,PruneIgnoredNodesVisitor,NamescopingVisitor,
// CreateValuesVisitor,RegisterXNamesVisitor,FillResourceDictionariesVisitor,
// ApplyPropertiesVisitor}.cs (+ the MarkupExpressionParser recursion of ExpandMarkupsVisitor's
// MarkupExpansionParser). The reflection-free substitutions are documented in the header.
#include "maui/xaml/xaml_visitors.hpp"

#include <algorithm>
#include <any>
#include <cstddef>
#include <format>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/application.hpp"
#include "maui/controls/dynamic_resource.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/resource_dictionary.hpp"
#include "maui/core/app_theme.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/event.hpp"
#include "maui/core/type_tag.hpp"
#include "maui/xaml/hydration_context.hpp"
#include "maui/xaml/i_markup_extension.hpp"
#include "maui/xaml/markup_extensions.hpp"
#include "maui/xaml/name_scope.hpp"
#include "maui/xaml/xaml_binding_applier.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_node.hpp"
#include "maui/xaml/xaml_parse_exception.hpp"
#include "maui/xaml/xaml_parser.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"

namespace maui::xaml
{
    namespace
    {
        // ---- shared node helpers -------------------------------------------------------------------

        // NodeExtensions.TryGetPropertyName: the xml_name under which `node` is a property value of
        // `parent_node`, or nullopt (it is a collection item / the parent is not an element).
        [[nodiscard]] std::optional<xml_name> try_get_property_name(const i_xaml_node& node,
                                                                    const i_xaml_node* parent_node)
        {
            const auto* parent_element = dynamic_cast<const element_node*>(parent_node);
            if (parent_element == nullptr)
            {
                return std::nullopt;
            }
            for (const auto& [name, value] : parent_element->properties())
            {
                if (value.get() == &node)
                {
                    return name;
                }
            }
            return std::nullopt;
        }

        // ApplyPropertiesVisitor.IsCollectionItem.
        [[nodiscard]] bool is_collection_item(const i_xaml_node& node, i_xaml_node* parent_node)
        {
            auto* parent_list = dynamic_cast<i_list_node*>(parent_node);
            if (parent_list == nullptr)
            {
                return false;
            }
            const auto& items = parent_list->collection_items();
            return std::ranges::any_of(
                items, [&node](const std::shared_ptr<i_xaml_node>& item) { return item.get() == &node; });
        }

        // ApplyPropertiesVisitor.Skips — the x:* directives the apply pass never assigns.
        [[nodiscard]] bool is_apply_skip(const xml_name& name)
        {
            return name == xml_name::x_arguments() || name == xml_name::x_class() ||
                   name == xml_name::x_class_modifier() || name == xml_name::x_data_type() ||
                   name == xml_name::x_factory_method() || name == xml_name::x_field_modifier() ||
                   name == xml_name::x_key() || name == xml_name::x_name() || name == xml_name::x_type_arguments();
        }

        // ExpandMarkupsVisitor.Skips.
        [[nodiscard]] bool is_expand_skip(const xml_name& name)
        {
            return name == xml_name::x_key() || name == xml_name::x_type_arguments() ||
                   name == xml_name::x_factory_method() || name == xml_name::x_name() ||
                   name == xml_name::x_data_type();
        }

        [[nodiscard]] bool is_skip_property(element_node& parent, const xml_name& name)
        {
            const auto& skips = parent.skip_properties();
            return std::ranges::find(skips, name) != skips.end();
        }

        // The hydrated-object unboxers (the std::any shapes context values carry — see
        // create_values_visitor's header comment).
        [[nodiscard]] maui::core::bindable_object* as_bindable(const std::any* value)
        {
            if (value == nullptr)
            {
                return nullptr;
            }
            const auto* object = std::any_cast<std::shared_ptr<maui::core::bindable_object>>(value);
            return object != nullptr ? object->get() : nullptr;
        }

        [[nodiscard]] maui::controls::resource_dictionary* as_dictionary(const std::any* value)
        {
            if (value == nullptr)
            {
                return nullptr;
            }
            const auto* const stored = std::any_cast<maui::controls::resource_dictionary*>(value);
            return stored != nullptr ? *stored : nullptr;
        }

        // The x:Key literal of an element node; throws C#'s "x:Key expects a string literal." when
        // the key is not a plain value node.
        [[nodiscard]] std::optional<std::string> x_key_of(const element_node& node)
        {
            const std::shared_ptr<i_xaml_node> key_node = node.properties().try_get(xml_name::x_key());
            if (key_node == nullptr)
            {
                return std::nullopt;
            }
            const auto* literal = dynamic_cast<const value_node*>(key_node.get());
            if (literal == nullptr)
            {
                throw xaml_parse_exception("x:Key expects a string literal.", node.line_number(), node.line_position());
            }
            return literal->value();
        }

        // The visitors' single raise pattern: run `action`; a xaml_parse_exception goes through the
        // context's handler knob (collect-and-continue when a handler is set, throw otherwise).
        template <class F> void guarded(const hydration_context& context, F&& action)
        {
            try
            {
                std::forward<F>(action)();
            }
            catch (const xaml_parse_exception& error)
            {
                context.handle(error);
            }
        }

        // ---- the value applier (ApplyPropertiesVisitor.TrySetPropertyValue's route chain) -----------

        // What the applier consumes from the context — flattened so the {AppThemeBinding} re-apply
        // closure can outlive the load (it captures this env by value; the REGISTRIES and the
        // APPLICATION must outlive the loaded tree, the loader's documented contract).
        struct applier_env
        {
            const xaml_property_registry* properties = nullptr;
            const xaml_converter_registry* converters = nullptr;
            maui::controls::application* application = nullptr;
            std::vector<maui::core::scoped_connection>* subscriptions = nullptr; // null = no re-subscribe
        };

        [[nodiscard]] applier_env env_from(hydration_context& context)
        {
            return {.properties = &context.property_registry(),
                    .converters = &context.converter_registry(),
                    .application = context.application,
                    .subscriptions = &context.subscriptions()};
        }

        [[noreturn]] void throw_cannot_assign(const std::string& local_name, int line_number, int line_position)
        {
            // TrySetPropertyValue's catch-all message, verbatim.
            throw xaml_parse_exception(std::format("Cannot assign property \"{}\": Property does not exist, or is "
                                                   "not assignable, or mismatching type between value and property",
                                                   local_name),
                                       line_number, line_position);
        }

        // ApplyPropertiesVisitor.TryAddToResourceDictionary (the value cases the port can load —
        // keyless Style/ResourceDictionary/StyleSheet items are M7 deferrals with styles-in-XAML).
        void add_to_resource_dictionary(maui::controls::resource_dictionary& dictionary, const std::any& value,
                                        const std::optional<std::string>& x_key, int line_number, int line_position)
        {
            if (!x_key.has_value())
            {
                throw xaml_parse_exception("resources in ResourceDictionary require a x:Key attribute", line_number,
                                           line_position);
            }
            if (!dictionary.add(*x_key, value))
            {
                // ResourceDictionary.Add's ArgumentException, surfaced on the XAML channel.
                throw xaml_parse_exception(
                    std::format("A resource with the key '{}' is already present in the ResourceDictionary", *x_key),
                    line_number, line_position);
            }
        }

        void apply_value_core(const applier_env& env, maui::core::bindable_object& target,
                              maui::core::type_tag target_type, const std::string& local_name, const std::any& value,
                              const std::optional<std::string>& x_key, int line_number, int line_position);

        // AppThemeBinding.Apply/ApplyCore: apply the picked slot now; with a live application,
        // re-apply on every RequestedThemeChanged (the C# binding subscribes through its
        // "__MAUI_ApplicationTheme__" proxy; the port subscribes the event directly and parks the
        // connection in the load's subscription accumulator).
        // Everything one theme re-apply needs, shared immutably by the subscription's closure (the
        // single shared_ptr capture keeps the closure's special members nothrow — the strings/anys
        // live here, copied once).
        struct theme_reapply_state
        {
            applier_env env; // subscriptions == nullptr: the re-apply path never re-subscribes
            maui::core::bindable_object* target = nullptr;
            maui::core::type_tag target_type;
            std::string local_name;
            app_theme_binding binding;
            int line_number = -1;
            int line_position = -1;
        };

        void apply_app_theme_binding(const applier_env& env, maui::core::bindable_object& target,
                                     maui::core::type_tag target_type, const std::string& local_name,
                                     const app_theme_binding& binding, int line_number, int line_position)
        {
            const maui::core::app_theme theme =
                env.application != nullptr ? env.application->requested_theme() : maui::core::app_theme::unspecified;
            applier_env reapply_env = env;
            reapply_env.subscriptions = nullptr; // the re-apply path never re-subscribes
            apply_value_core(reapply_env, target, target_type, local_name, binding.pick(theme), std::nullopt,
                             line_number, line_position);
            if (env.subscriptions == nullptr || env.application == nullptr)
            {
                return;
            }
            auto state =
                std::make_shared<const theme_reapply_state>(theme_reapply_state{.env = reapply_env,
                                                                                .target = &target,
                                                                                .target_type = target_type,
                                                                                .local_name = local_name,
                                                                                .binding = binding,
                                                                                .line_number = line_number,
                                                                                .line_position = line_position});
            const maui::core::connection_token token = env.application->requested_theme_changed.connect(
                [state = std::move(state)](const maui::core::app_theme& changed) {
                    apply_value_core(state->env, *state->target, state->target_type, state->local_name,
                                     state->binding.pick(changed), std::nullopt, state->line_number,
                                     state->line_position);
                });
            env.subscriptions->emplace_back(env.application->requested_theme_changed, token);
        }

        // The port of TrySetPropertyValue's route chain — see apply_properties_visitor's header
        // comment for the full route list. Throws xaml_parse_exception on failure.
        void apply_value_core(const applier_env& env, maui::core::bindable_object& target,
                              maui::core::type_tag target_type, const std::string& local_name, const std::any& value,
                              const std::optional<std::string>& x_key, int line_number, int line_position)
        {
            // {OnPlatform}/{OnIdiom} "no value for this platform/idiom": skip the assignment (the
            // documented deviation from C#'s BindableProperty.GetDefaultValue re-assignment).
            if (!value.has_value())
            {
                return;
            }

            // TrySetDynamicResource: a DynamicResource marker on a BINDABLE property routes to
            // SetDynamicResource; on anything else C# falls through to the catch-all error.
            if (const auto* marker = std::any_cast<maui::controls::dynamic_resource>(&value))
            {
                const xaml_property_registry::property_entry* entry = env.properties->find(target_type, local_name);
                if (entry == nullptr || entry->bindable_name.empty())
                {
                    throw_cannot_assign(local_name, line_number, line_position);
                }
                auto* element = dynamic_cast<maui::controls::element*>(&target);
                if (element == nullptr)
                {
                    // C#: "{type} is not a BindableObject" (every port control IS an element).
                    throw xaml_parse_exception(
                        std::format("Cannot set \"{}\" as DynamicResource: the target is not an element", local_name),
                        line_number, line_position);
                }
                element->set_dynamic_resource(std::string{entry->bindable_name}, marker->key());
                return;
            }

            // TrySetBinding — routed through the loader's replaceable hook (xaml_binding_applier
            // .hpp): the rejecting default makes {Binding} a loud load failure until the
            // runtime-binding unit registers the real SetBinding port.
            if (const auto* request = std::any_cast<binding_request>(&value))
            {
                current_xaml_binding_applier()(*env.properties, target, target_type, local_name, *request,
                                               line_number, line_position);
                return;
            }

            // AppThemeBinding.Apply.
            if (const auto* binding = std::any_cast<app_theme_binding>(&value))
            {
                apply_app_theme_binding(env, target, target_type, local_name, *binding, line_number, line_position);
                return;
            }

            // TrySetValue / TrySetProperty: the registered property surface (bindable + typed-lambda
            // registrations share one table — xaml_property_registry.hpp).
            const xaml_property_registry::property_entry* entry = env.properties->find(target_type, local_name);
            if (entry != nullptr)
            {
                if (std::any_cast<xaml_null>(&value) != nullptr)
                {
                    // The {x:Null} value-form contract (markup_extensions.hpp): no typed-null channel
                    // yet — a reported load failure rather than a silent skip.
                    throw xaml_parse_exception(
                        std::format("{{x:Null}} cannot be applied to \"{}\": the v1 property registry has no "
                                    "typed-null channel (STATUS.md M7 deferrals)",
                                    local_name),
                        line_number, line_position);
                }
                const auto* text = std::any_cast<std::string>(&value);
                if (text != nullptr && entry->value_type != maui::core::type_tag::of<std::string>())
                {
                    // The late string conversion (C# ConvertTo against the property's return type).
                    if (env.properties->try_set_from_text(target_type, target, local_name, *text, *env.converters))
                    {
                        return;
                    }
                    throw_cannot_assign(local_name, line_number, line_position);
                }
                if (env.properties->try_set(target_type, target, local_name, value))
                {
                    return;
                }
                throw_cannot_assign(local_name, line_number, line_position);
            }

            // TryAddToProperty, scoped to the one collection C# reaches this way in the v1 surface:
            // the element's lazily-created Resources dictionary (the IMPLICIT resource form).
            if (local_name == "Resources" || local_name.ends_with(".Resources"))
            {
                auto* element = dynamic_cast<maui::controls::element*>(&target);
                if (element != nullptr)
                {
                    add_to_resource_dictionary(element->resources(), value, x_key, line_number, line_position);
                    return;
                }
            }

            // The named child sink: the <Layout.Children> property-element spelling and
            // Content="{StaticResource …}" route through the registered add_child (the reflection-free
            // stand-in for C#'s IEnumerable + Add() / settable-property walk).
            if (env.properties->is_child_property(target_type, local_name))
            {
                maui::core::bindable_object* child = as_bindable(&value);
                if (child != nullptr && env.properties->try_add_child(target_type, target, *child))
                {
                    return;
                }
            }

            throw_cannot_assign(local_name, line_number, line_position);
        }

        // ---- the markup expansion parser (ExpandMarkupsVisitor.MarkupExpansionParser) --------------

        // The minted-extension holder expansion stores in context values: attributes captured at
        // expand time, nested extensions provided lazily — provide_value resolves them and mints the
        // real extension through its registry factory, at the same point in time C# calls
        // ProvideValue (apply). Factory/attribute errors therefore surface at APPLY time, like C#'s
        // reflective property assignment.
        class expanded_markup_extension final : public i_markup_extension
        {
        public:
            explicit expanded_markup_extension(const markup_extension_factory* factory) : factory_(factory)
            {
            }

            void add_attribute(std::string name, std::string value)
            {
                arguments_.attributes.insert_or_assign(std::move(name), std::move(value));
            }
            void add_nested(std::string name, std::shared_ptr<i_markup_extension> extension)
            {
                nested_.emplace_back(std::move(name), std::move(extension));
            }

            [[nodiscard]] std::any provide_value(const xaml_service_provider& services) override
            {
                markup_extension_arguments arguments = arguments_;
                for (const auto& [name, extension] : nested_)
                {
                    arguments.values.insert_or_assign(name, extension->provide_value(services));
                }
                return (*factory_)(arguments)->provide_value(services);
            }

        private:
            const markup_extension_factory* factory_;
            markup_extension_arguments arguments_;
            std::vector<std::pair<std::string, std::shared_ptr<i_markup_extension>>> nested_;
        };

        // One parsed expression: a minted extension, or the "{}"-escaped literal string.
        struct parsed_markup_value
        {
            std::shared_ptr<i_markup_extension> extension;
            std::optional<std::string> literal;
        };

        [[nodiscard]] std::string_view trim_markup_start(std::string_view text)
        {
            // C# TrimStart() — whitespace; markup attribute values only carry the ASCII set.
            while (!text.empty() && (text.front() == ' ' || text.front() == '\t' || text.front() == '\n' ||
                                     text.front() == '\r' || text.front() == '\f' || text.front() == '\v'))
            {
                text.remove_prefix(1);
            }
            return text;
        }

        // The registry key for "[prefix:]Name": the x namespace keeps its prefix spelling ("x:Static")
        // — every other prefix resolves to the BARE name, the flat-registry deviation from C#'s
        // per-xmlns CLR type resolution (markup names are unique across the registered set; apps
        // register custom extensions under the bare name).
        [[nodiscard]] std::string resolve_extension_name(const std::string& prefix, const std::string& name,
                                                         const xml_namespace_resolver& resolver)
        {
            if (prefix.empty())
            {
                return name;
            }
            const std::optional<std::string> uri = resolver.lookup_namespace(prefix);
            if (uri.has_value() && (*uri == x2006_uri || *uri == x2009_uri))
            {
                return "x:" + name;
            }
            return name;
        }

        [[nodiscard]] parsed_markup_value parse_markup_expression(std::string_view& remaining,
                                                                  const xml_namespace_resolver& resolver,
                                                                  int line_number, int line_position);

        // ParsePropertyExpression's tail: after a nested expression, expect ',' (more properties) or
        // '}' (the last one) and consume it.
        [[nodiscard]] bool consume_markup_delimiter(std::string_view& remaining, int line_number, int line_position)
        {
            remaining = trim_markup_start(remaining);
            if (remaining.empty())
            {
                throw xaml_parse_exception("Unexpected end of markup expression", line_number, line_position);
            }
            const char delimiter = remaining.front();
            if (delimiter != ',' && delimiter != '}')
            {
                throw xaml_parse_exception("Unexpected character following value string", line_number, line_position);
            }
            remaining.remove_prefix(1);
            return delimiter == '}';
        }

        // MarkupExpansionParser.Parse: the property loop after the extension name was matched.
        [[nodiscard]] std::shared_ptr<i_markup_extension> parse_markup_tail(const std::string& match,
                                                                            std::string_view& remaining,
                                                                            const xml_namespace_resolver& resolver,
                                                                            int line_number, int line_position)
        {
            const auto [prefix, name] = parse_markup_name(match);
            const std::string registry_name = resolve_extension_name(prefix, name, resolver);
            const markup_extension_factory* factory = markup_extension_registry::instance().find(registry_name);
            if (factory == nullptr)
            {
                throw xaml_parse_exception(std::format("MarkupExtension not found for {}", match), line_number,
                                           line_position);
            }
            auto extension = std::make_shared<expanded_markup_extension>(factory);

            if (remaining.starts_with("}"))
            {
                remaining.remove_prefix(1);
                return extension;
            }

            bool last = false;
            while (!last)
            {
                // MarkupExpressionParser.ParseProperty.
                remaining = trim_markup_start(remaining);
                if (remaining.empty())
                {
                    throw xaml_parse_exception("Unexpected end of markup expression", line_number, line_position);
                }
                std::optional<std::string> attribute_name;
                std::optional<std::string> string_value;
                parsed_markup_value nested;
                if (remaining.front() == '{')
                {
                    nested = parse_markup_expression(remaining, resolver, line_number, line_position);
                    last = consume_markup_delimiter(remaining, line_number, line_position);
                }
                else
                {
                    markup_piece piece = get_next_piece(remaining);
                    remaining = piece.remaining;
                    if (piece.next == '=')
                    {
                        attribute_name = std::move(piece.piece);
                        remaining = trim_markup_start(remaining);
                        if (remaining.empty())
                        {
                            throw xaml_parse_exception("Unexpected end of markup expression", line_number,
                                                       line_position);
                        }
                        if (remaining.front() == '{')
                        {
                            nested = parse_markup_expression(remaining, resolver, line_number, line_position);
                            last = consume_markup_delimiter(remaining, line_number, line_position);
                        }
                        else
                        {
                            markup_piece value_piece = get_next_piece(remaining);
                            remaining = value_piece.remaining;
                            string_value = std::move(value_piece.piece);
                            last = value_piece.next == '}';
                        }
                    }
                    else
                    {
                        string_value = std::move(piece.piece);
                        last = piece.next == '}';
                    }
                }

                // The positional piece maps to the extension's [ContentProperty] under the empty name
                // (i_markup_extension.hpp's attribute conventions).
                std::string key = attribute_name.value_or(std::string{});
                if (key == "x:TypeArguments")
                {
                    throw xaml_parse_exception("x:TypeArguments in a markup extension is not supported by the port yet "
                                               "(STATUS.md M7 deferrals)",
                                               line_number, line_position);
                }
                if (nested.extension != nullptr)
                {
                    extension->add_nested(std::move(key), nested.extension);
                }
                else if (nested.literal.has_value())
                {
                    extension->add_attribute(std::move(key), std::move(*nested.literal));
                }
                else if (string_value.has_value())
                {
                    extension->add_attribute(std::move(key), std::move(*string_value));
                }
            }
            return extension;
        }

        // MarkupExpressionParser.ParseExpression — `remaining` is advanced past the expression.
        [[nodiscard]] parsed_markup_value parse_markup_expression(std::string_view& remaining,
                                                                  const xml_namespace_resolver& resolver,
                                                                  int line_number, int line_position)
        {
            if (remaining.starts_with("{}"))
            {
                // The escaped-literal branch (C# returns Substring(2) and the caller treats it as a
                // plain string).
                parsed_markup_value literal{.extension = nullptr, .literal = std::string{remaining.substr(2)}};
                remaining = {};
                return literal;
            }
            if (remaining.empty() || remaining.back() != '}')
            {
                throw xaml_parse_exception("Expression must end with '}'", line_number, line_position);
            }
            const markup_match match = match_markup(remaining);
            if (!match.matched)
            {
                // C# throws a bare Exception here; the port keeps the single XAML error channel.
                throw xaml_parse_exception("Expression must end with '}'", line_number, line_position);
            }
            remaining.remove_prefix(match.end);
            remaining = trim_markup_start(remaining);
            if (remaining.empty())
            {
                throw xaml_parse_exception("Expression did not end in '}'", line_number, line_position);
            }
            return {.extension = parse_markup_tail(match.match, remaining, resolver, line_number, line_position),
                    .literal = std::nullopt};
        }

        // ---- the XamlServiceProvider builder --------------------------------------------------------

        // XamlServiceProvider(node, context): target + the IProvideParentValues.ParentObjects walk,
        // reduced to the resource dictionaries the v1 extensions consume (i_markup_extension.hpp).
        [[nodiscard]] xaml_service_provider make_service_provider(hydration_context& context, const i_xaml_node& node,
                                                                  maui::core::bindable_object* target,
                                                                  std::string target_property)
        {
            xaml_service_provider services;
            services.target_object = target;
            services.target_property = std::move(target_property);
            services.type_registry = &context.type_registry();
            services.application = context.application;
            for (const i_xaml_node* ancestor = node.parent(); ancestor != nullptr; ancestor = ancestor->parent())
            {
                const auto* ancestor_element = dynamic_cast<const element_node*>(ancestor);
                if (ancestor_element == nullptr)
                {
                    continue;
                }
                const std::any* value = context.try_get_value(*ancestor_element);
                if (const maui::controls::resource_dictionary* dictionary = as_dictionary(value))
                {
                    services.parent_resources.push_back(dictionary);
                    continue;
                }
                auto* element = dynamic_cast<maui::controls::element*>(as_bindable(value));
                if (element != nullptr && element->is_resources_created())
                {
                    services.parent_resources.push_back(&element->resources());
                }
            }
            return services;
        }
    } // namespace

    // ================================================================================================
    // expand_markups_visitor  <=  ExpandMarkupsVisitor
    // ================================================================================================

    void expand_markups_visitor::visit(value_node& /*node*/, i_xaml_node* /*parent_node*/)
    {
    }

    void expand_markups_visitor::visit(markup_node& node, i_xaml_node* parent_node)
    {
        auto* parent_element = dynamic_cast<element_node*>(parent_node);
        const std::optional<xml_name> property_name = try_get_property_name(node, parent_node);
        if (parent_element == nullptr || !property_name.has_value())
        {
            return; // collection-item markup is left alone, like C#
        }
        if (is_expand_skip(*property_name) || is_skip_property(*parent_element, *property_name))
        {
            return;
        }
        guarded(*context_, [this, &node] {
            std::string_view remaining = node.markup_string();
            const parsed_markup_value parsed = parse_markup_expression(remaining, *node.namespace_resolver(),
                                                                       node.line_number(), node.line_position());
            if (parsed.extension != nullptr)
            {
                context_->set_value(node, std::any{parsed.extension});
            }
            else if (parsed.literal.has_value())
            {
                context_->set_value(node, std::any{*parsed.literal});
            }
        });
    }

    void expand_markups_visitor::visit(element_node& /*node*/, i_xaml_node* /*parent_node*/)
    {
    }
    void expand_markups_visitor::visit(root_node& /*node*/, i_xaml_node* /*parent_node*/)
    {
    }
    void expand_markups_visitor::visit(list_node& /*node*/, i_xaml_node* /*parent_node*/)
    {
    }
    bool expand_markups_visitor::skip_children(i_xaml_node& /*node*/, i_xaml_node* /*parent_node*/)
    {
        return false;
    }
    bool expand_markups_visitor::is_resource_dictionary(element_node& /*node*/)
    {
        return false;
    }

    // ================================================================================================
    // prune_ignored_nodes_visitor  <=  PruneIgnoredNodesVisitor
    // ================================================================================================

    namespace
    {
        // The prefix of `namespace_uri` in `node`'s scope, or nullopt (C# LookupPrefix returns null).
        [[nodiscard]] std::optional<std::string> prefix_of(const i_xaml_node& node, std::string_view namespace_uri)
        {
            return node.namespace_resolver()->lookup_prefix(namespace_uri);
        }

        [[nodiscard]] bool is_prefix_ignored(const i_xaml_node& node, const std::optional<std::string>& prefix)
        {
            return prefix.has_value() && skip_prefix(node, *prefix);
        }

        void prune_element(element_node& node, i_xaml_node* parent_node)
        {
            // mc:Ignorable lists accumulate on the PARENT's IgnorablePrefixes (the root visit passes
            // itself as parent, exactly like C#'s Visit(RootNode)).
            for (const auto& [property_name, property_value] : node.properties())
            {
                const auto* literal = dynamic_cast<const value_node*>(property_value.get());
                if (literal == nullptr || property_name != xml_name::mc_ignorable())
                {
                    continue;
                }
                std::string_view text = literal->value();
                while (!text.empty())
                {
                    const std::size_t space = text.find(' ');
                    const std::string_view piece = text.substr(0, space);
                    if (!piece.empty())
                    {
                        parent_node->ignorable_prefixes().emplace_back(piece);
                    }
                    text = space == std::string_view::npos ? std::string_view{} : text.substr(space + 1);
                }
            }

            // Drop ignorable properties ("d:foo" attributes AND properties holding elements of an
            // ignorable namespace) — snapshot first, the map mutates.
            const std::vector<std::pair<xml_name, std::shared_ptr<i_xaml_node>>> properties{
                node.properties().begin(), node.properties().end()};
            for (const auto& [property_name, property_value] : properties)
            {
                if (is_prefix_ignored(node, prefix_of(node, property_name.namespace_uri)))
                {
                    (void)node.properties().remove(property_name);
                    continue;
                }
                const auto* property_element = dynamic_cast<const element_node*>(property_value.get());
                const std::string_view value_namespace = property_element != nullptr
                                                             ? std::string_view{property_element->namespace_uri()}
                                                             : std::string_view{};
                if (is_prefix_ignored(node, prefix_of(node, value_namespace)))
                {
                    (void)node.properties().remove(property_name);
                }
            }

            // Drop ignorable collection items.
            auto& items = node.collection_items();
            std::erase_if(items, [&node](const std::shared_ptr<i_xaml_node>& item) {
                const auto* item_element = dynamic_cast<const element_node*>(item.get());
                const std::string_view item_namespace =
                    item_element != nullptr ? std::string_view{item_element->namespace_uri()} : std::string_view{};
                return is_prefix_ignored(node, prefix_of(node, item_namespace));
            });

            // The node itself in an ignorable namespace: empty it (C# clears Properties +
            // CollectionItems; the node stays in place).
            if (is_prefix_ignored(node, prefix_of(node, node.namespace_uri())))
            {
                std::vector<xml_name> names;
                names.reserve(node.properties().size());
                for (const auto& [property_name, property_value] : node.properties())
                {
                    names.push_back(property_name);
                }
                for (const xml_name& property_name : names)
                {
                    (void)node.properties().remove(property_name);
                }
                node.collection_items().clear();
            }
        }
    } // namespace

    void prune_ignored_nodes_visitor::visit(value_node& /*node*/, i_xaml_node* /*parent_node*/)
    {
    }
    void prune_ignored_nodes_visitor::visit(markup_node& /*node*/, i_xaml_node* /*parent_node*/)
    {
    }
    void prune_ignored_nodes_visitor::visit(element_node& node, i_xaml_node* parent_node)
    {
        prune_element(node, parent_node);
    }
    void prune_ignored_nodes_visitor::visit(root_node& node, i_xaml_node* /*parent_node*/)
    {
        prune_element(node, &node); // C#: Visit((ElementNode)node, node)
    }
    void prune_ignored_nodes_visitor::visit(list_node& node, i_xaml_node* /*parent_node*/)
    {
        auto& items = node.collection_items();
        std::erase_if(items, [&node](const std::shared_ptr<i_xaml_node>& item) {
            const auto* item_element = dynamic_cast<const element_node*>(item.get());
            const std::string_view item_namespace =
                item_element != nullptr ? std::string_view{item_element->namespace_uri()} : std::string_view{};
            return is_prefix_ignored(node, prefix_of(node, item_namespace));
        });
    }
    bool prune_ignored_nodes_visitor::skip_children(i_xaml_node& /*node*/, i_xaml_node* /*parent_node*/)
    {
        return false;
    }
    bool prune_ignored_nodes_visitor::is_resource_dictionary(element_node& /*node*/)
    {
        return false;
    }

    // ================================================================================================
    // namescoping_visitor  <=  NamescopingVisitor
    // ================================================================================================

    namespace
    {
        [[nodiscard]] bool is_data_template_value(const i_xaml_node& node, const i_xaml_node* parent_node)
        {
            const auto* parent_element = dynamic_cast<const element_node*>(parent_node);
            if (parent_element == nullptr)
            {
                return false;
            }
            const std::shared_ptr<i_xaml_node> create_content =
                parent_element->properties().try_get(xml_name::create_content());
            return create_content.get() == &node;
        }

        [[nodiscard]] bool is_style_value(const i_xaml_node* parent_node)
        {
            const auto* parent_element = dynamic_cast<const element_node*>(parent_node);
            return parent_element != nullptr && parent_element->type().name() == "Style";
        }

        [[nodiscard]] bool is_visual_state_group_in_list(const element_node& node)
        {
            return node.type().name() == "VisualStateGroup" && dynamic_cast<i_list_node*>(node.parent()) != nullptr;
        }

        [[nodiscard]] std::shared_ptr<name_scope_ref> fresh_scope_ref()
        {
            auto ref = std::make_shared<name_scope_ref>();
            ref->scope = std::make_shared<name_scope>();
            return ref;
        }
    } // namespace

    namescoping_visitor::namescoping_visitor(hydration_context& /*context*/)
    {
        // C#'s constructor takes (and ignores) the context too — kept for the pipeline symmetry.
    }

    void namescoping_visitor::visit(value_node& node, i_xaml_node* parent_node)
    {
        scopes_[&node] = scopes_.at(parent_node);
    }
    void namescoping_visitor::visit(markup_node& node, i_xaml_node* parent_node)
    {
        scopes_[&node] = scopes_.at(parent_node);
    }
    void namescoping_visitor::visit(element_node& node, i_xaml_node* parent_node)
    {
        const bool starts_own_scope = parent_node == nullptr || is_data_template_value(node, parent_node) ||
                                      is_style_value(parent_node) || is_visual_state_group_in_list(node);
        std::shared_ptr<name_scope_ref> ref = starts_own_scope ? fresh_scope_ref() : scopes_.at(parent_node);
        node.set_scope_ref(ref);
        scopes_[&node] = std::move(ref);
    }
    void namescoping_visitor::visit(root_node& node, i_xaml_node* /*parent_node*/)
    {
        std::shared_ptr<name_scope_ref> ref = fresh_scope_ref();
        node.set_scope_ref(ref);
        scopes_[&node] = std::move(ref);
    }
    void namescoping_visitor::visit(list_node& node, i_xaml_node* parent_node)
    {
        scopes_[&node] = scopes_.at(parent_node);
    }
    bool namescoping_visitor::skip_children(i_xaml_node& /*node*/, i_xaml_node* /*parent_node*/)
    {
        return false;
    }
    bool namescoping_visitor::is_resource_dictionary(element_node& /*node*/)
    {
        return false;
    }

    // ================================================================================================
    // create_values_visitor  <=  CreateValuesVisitor
    // ================================================================================================

    namespace
    {
        // CreateLanguagePrimitive's text payload: the single ValueNode collection item, if any.
        [[nodiscard]] std::optional<std::string> primitive_text(element_node& node)
        {
            const auto& items = node.collection_items();
            if (items.size() != 1)
            {
                return std::nullopt;
            }
            const auto* literal = dynamic_cast<const value_node*>(items.front().get());
            return literal != nullptr ? std::optional<std::string>{literal->value()} : std::nullopt;
        }

        // The x2009 language primitives the v1 loader understands (CreateValuesVisitor
        // .IsXaml2009LanguagePrimitive + CreateLanguagePrimitive, over the registered built-in
        // converters). A TryParse failure falls back to the default value, silently — like C#.
        template <class T>
        [[nodiscard]] std::any create_primitive(const hydration_context& context, element_node& node, T fallback)
        {
            const std::optional<std::string> text = primitive_text(node);
            if (!text.has_value())
            {
                return std::any{std::move(fallback)};
            }
            try
            {
                std::any converted = context.converter_registry().convert(maui::core::type_tag::of<T>(), *text);
                if (converted.has_value())
                {
                    return converted;
                }
            }
            catch (const xaml_parse_exception&)
            {
                return std::any{std::move(fallback)}; // TryParse semantics: a malformed literal keeps the default
            }
            return std::any{std::move(fallback)};
        }
    } // namespace

    void create_values_visitor::visit(value_node& node, i_xaml_node* /*parent_node*/)
    {
        context_->set_value(node, std::any{node.value()});
    }

    void create_values_visitor::visit(markup_node& /*node*/, i_xaml_node* /*parent_node*/)
    {
        // The minted extension was stored by the expand pass; nothing to create (C# Visit(MarkupNode)
        // is empty too).
    }

    void create_values_visitor::visit(element_node& node, i_xaml_node* /*parent_node*/)
    {
        guarded(*context_, [this, &node] {
            const std::string& name = node.type().name();
            const std::string& namespace_uri = node.type().namespace_uri();

            // IsXaml2009LanguagePrimitive: the x namespace types stay a loader concern (the design
            // note in xaml_type_registry.hpp) — created through the built-in converters. The x2006
            // spelling is accepted too: C# reaches <x:String> & co. under the 2006 xmlns through
            // GetElementType's known-namespace table (mscorlib System types) + the single-ValueNode
            // ConvertTo branch — the same net value as the 2009 primitive route.
            if (namespace_uri == x2009_uri || namespace_uri == x2006_uri)
            {
                if (name == "String")
                {
                    context_->set_value(node, create_primitive<std::string>(*context_, node, std::string{}));
                    context_->set_type(node, maui::core::type_tag::of<std::string>());
                }
                else if (name == "Int32")
                {
                    context_->set_value(node, create_primitive<int>(*context_, node, 0));
                    context_->set_type(node, maui::core::type_tag::of<int>());
                }
                else if (name == "Double")
                {
                    context_->set_value(node, create_primitive<double>(*context_, node, 0.0));
                    context_->set_type(node, maui::core::type_tag::of<double>());
                }
                else if (name == "Boolean")
                {
                    context_->set_value(node, create_primitive<bool>(*context_, node, false));
                    context_->set_type(node, maui::core::type_tag::of<bool>());
                }
                else
                {
                    // The remaining x2009 primitives (x:Char, x:TimeSpan, …) are M7 deferrals.
                    throw xaml_parse_exception(std::format("Type {} not found in xmlns {}", name, namespace_uri),
                                               node.line_number(), node.line_position());
                }
                return;
            }

            // <ResourceDictionary>: not a bindable_object, so it cannot live in the type registry or
            // the object graph — minted here and kept alive by the context, stored as a NON-owning
            // pointer value.
            if (node.type().is_of_any_type({"ResourceDictionary"}))
            {
                auto dictionary = std::make_shared<maui::controls::resource_dictionary>();
                context_->set_value(node, std::any{dictionary.get()});
                context_->set_type(node, maui::core::type_tag::of<maui::controls::resource_dictionary>());
                context_->keep_alive(std::move(dictionary));
                return;
            }

            // x:Arguments / x:FactoryMethod need non-default construction — the registry's factories
            // are default-construct only (PROFILE §6); fail loudly instead of dropping the arguments.
            if (node.properties().contains(xml_name::x_arguments()) ||
                node.properties().contains(xml_name::x_factory_method()))
            {
                throw xaml_parse_exception(
                    "x:Arguments and x:FactoryMethod are not supported by the port yet (STATUS.md M7 deferrals)",
                    node.line_number(), node.line_position());
            }

            // XamlParser.GetElementType + Activator.CreateInstance — the explicit registry.
            const xaml_type_registry::registration* registration = nullptr;
            if (namespace_uri == maui_uri || namespace_uri == maui_global_uri)
            {
                registration = context_->type_registry().find(name, xaml_namespace::maui);
            }
            if (registration == nullptr)
            {
                throw xaml_parse_exception(std::format("Type {} not found in xmlns {}", name, namespace_uri),
                                           node.line_number(), node.line_position());
            }
            std::shared_ptr<maui::core::bindable_object> value = registration->create();
            context_->set_type(node, registration->type);
            context_->set_value(node, std::any{value});
            context_->graph().add(std::move(value));
            // (C#'s NameScope.SetNameScope / transientNamescope on the created object need an
            // element-side scope slot the port does not have — the placement deviation documented in
            // name_scope.hpp; scopes stay on the nodes and the loader result.)
        });
    }

    void create_values_visitor::visit(root_node& node, i_xaml_node* /*parent_node*/)
    {
        // XamlLoader.RuntimeRootNode: the root object is the caller's view (load_into) or the
        // instance the loader minted up front (load) — never created here.
        maui::core::bindable_object* root = context_->root_element();
        if (root == nullptr)
        {
            return;
        }
        // A NON-owning handle (aliasing constructor, empty owner): the root is owned by the caller
        // or by the graph entry the loader created — never by this extra values reference.
        context_->set_value(node,
                            std::any{std::shared_ptr<maui::core::bindable_object>(std::shared_ptr<void>{}, root)});
        guarded(*context_, [this, &node] {
            // The no-reflection stand-in for Context.Types[node] = Root.GetType(): resolve the root
            // ELEMENT NAME in the registry (load_into therefore requires a registered root name).
            if (context_->try_get_type(node) != nullptr)
            {
                return; // the load flow resolved it through the element-visit already
            }
            const std::string& name = node.type().name();
            const std::string& namespace_uri = node.type().namespace_uri();
            const xaml_type_registry::registration* registration =
                namespace_uri == maui_uri || namespace_uri == maui_global_uri
                    ? context_->type_registry().find(name, xaml_namespace::maui)
                    : nullptr;
            if (registration == nullptr)
            {
                throw xaml_parse_exception(std::format("Type {} not found in xmlns {}", name, namespace_uri),
                                           node.line_number(), node.line_position());
            }
            context_->set_type(node, registration->type);
        });
    }

    void create_values_visitor::visit(list_node& node, i_xaml_node* parent_node)
    {
        // C#'s "gross hack to keep ListNode alive": remember the property name the list stands for.
        const std::optional<xml_name> name = try_get_property_name(node, parent_node);
        if (name.has_value())
        {
            node.set_name(*name);
        }
    }

    bool create_values_visitor::skip_children(i_xaml_node& /*node*/, i_xaml_node* /*parent_node*/)
    {
        return false;
    }
    bool create_values_visitor::is_resource_dictionary(element_node& node)
    {
        const maui::core::type_tag* type = context_->try_get_type(node);
        return type != nullptr && *type == maui::core::type_tag::of<maui::controls::resource_dictionary>();
    }

    // ================================================================================================
    // register_x_names_visitor  <=  RegisterXNamesVisitor
    // ================================================================================================

    namespace
    {
        // RegisterXNamesVisitor.IsXNameProperty.
        [[nodiscard]] bool is_x_name_property(const value_node& node, const i_xaml_node* parent_node)
        {
            const auto* parent_element = dynamic_cast<const element_node*>(parent_node);
            if (parent_element == nullptr)
            {
                return false;
            }
            const std::shared_ptr<i_xaml_node> name_node = parent_element->properties().try_get(xml_name::x_name());
            return name_node.get() == &node;
        }
    } // namespace

    void register_x_names_visitor::visit(value_node& node, i_xaml_node* parent_node)
    {
        if (!is_x_name_property(node, parent_node))
        {
            return;
        }
        auto* parent_element = dynamic_cast<element_node*>(parent_node);
        const std::shared_ptr<name_scope_ref>& ref = parent_element->scope_ref();
        if (ref == nullptr || ref->scope == nullptr)
        {
            return; // namescoping has not run — nothing to register into
        }
        const std::any* value = context_->try_get_value(*parent_element);
        if (value == nullptr)
        {
            return; // creation failed under an exception handler (C#'s KeyNotFound route)
        }
        guarded(*context_, [&node, &ref, value] {
            try
            {
                ref->scope->register_name(node.value(), *value);
            }
            catch (const std::invalid_argument&)
            {
                // NameScope.RegisterName's ArgumentException, re-raised with the C# visitor's message.
                throw xaml_parse_exception(
                    std::format("An element with the name \"{}\" already exists in this NameScope", node.value()),
                    node.line_number(), node.line_position());
            }
        });
        // (C# also seeds Element.StyleId from x:Name — no StyleId on the port's element yet.)
    }

    void register_x_names_visitor::visit(markup_node& /*node*/, i_xaml_node* /*parent_node*/)
    {
    }
    void register_x_names_visitor::visit(element_node& /*node*/, i_xaml_node* /*parent_node*/)
    {
    }
    void register_x_names_visitor::visit(root_node& /*node*/, i_xaml_node* /*parent_node*/)
    {
    }
    void register_x_names_visitor::visit(list_node& /*node*/, i_xaml_node* /*parent_node*/)
    {
    }
    bool register_x_names_visitor::skip_children(i_xaml_node& /*node*/, i_xaml_node* /*parent_node*/)
    {
        return false;
    }
    bool register_x_names_visitor::is_resource_dictionary(element_node& node)
    {
        const maui::core::type_tag* type = context_->try_get_type(node);
        return type != nullptr && *type == maui::core::type_tag::of<maui::controls::resource_dictionary>();
    }

    // ================================================================================================
    // fill_resource_dictionaries_visitor  <=  FillResourceDictionariesVisitor
    // ================================================================================================

    namespace
    {
        [[nodiscard]] bool is_resource_dictionary_node(const hydration_context& context, const i_xaml_node* node)
        {
            const auto* element = dynamic_cast<const element_node*>(node);
            if (element == nullptr)
            {
                return false;
            }
            const maui::core::type_tag* type = context.try_get_type(*element);
            return type != nullptr && *type == maui::core::type_tag::of<maui::controls::resource_dictionary>();
        }

        // FillResourceDictionariesVisitor's "keyless RD parent" tests (Visit + SkipChildren).
        [[nodiscard]] bool is_keyless_resource_dictionary_parent(const hydration_context& context,
                                                                 i_xaml_node* parent_node)
        {
            if (const auto* parent_element = dynamic_cast<const element_node*>(parent_node))
            {
                return is_resource_dictionary_node(context, parent_element) &&
                       !parent_element->properties().contains(xml_name::x_key());
            }
            if (const auto* parent_list = dynamic_cast<const list_node*>(parent_node))
            {
                const auto* grandparent = dynamic_cast<const element_node*>(parent_list->parent());
                return grandparent != nullptr && is_resource_dictionary_node(context, grandparent) &&
                       !grandparent->properties().contains(xml_name::x_key());
            }
            return false;
        }
    } // namespace

    void fill_resource_dictionaries_visitor::visit(value_node& node, i_xaml_node* parent_node)
    {
        if (!is_resource_dictionary_node(*context_, parent_node))
        {
            return;
        }
        apply_properties_visitor apply{*context_, /*stop_on_resource_dictionary=*/false};
        node.accept(apply, parent_node);
    }

    void fill_resource_dictionaries_visitor::visit(markup_node& /*node*/, i_xaml_node* /*parent_node*/)
    {
    }

    void fill_resource_dictionaries_visitor::visit(element_node& node, i_xaml_node* parent_node)
    {
        const std::any* value = context_->try_get_value(node);
        if (value == nullptr && context_->has_handler())
        {
            return;
        }

        // A <ResourceDictionary> that IS the "Resources" property value: C# assigns the RD object to
        // VisualElement.Resources; the port's element owns its dictionary, so the node's value is
        // RETARGETED at element.resources() and the items below fill it directly.
        if (is_resource_dictionary_node(*context_, &node))
        {
            const std::optional<xml_name> property_name = try_get_property_name(node, parent_node);
            if (property_name.has_value() &&
                (property_name->local_name == "Resources" || property_name->local_name.ends_with(".Resources")))
            {
                const auto* parent_element_node = dynamic_cast<const element_node*>(parent_node);
                const std::any* source =
                    parent_element_node != nullptr ? context_->try_get_value(*parent_element_node) : nullptr;
                auto* element = dynamic_cast<maui::controls::element*>(as_bindable(source));
                if (element != nullptr)
                {
                    context_->set_value(node, std::any{&element->resources()});
                }
                return;
            }
        }

        // Children of a keyless RD are fully applied here (a nested apply run adds each under its
        // x:Key); the fill traversal itself skips those subtrees (skip_children below).
        if (is_keyless_resource_dictionary_parent(*context_, parent_node))
        {
            apply_properties_visitor apply{*context_, /*stop_on_resource_dictionary=*/false};
            node.accept(apply, parent_node);
        }
    }

    void fill_resource_dictionaries_visitor::visit(root_node& /*node*/, i_xaml_node* /*parent_node*/)
    {
    }
    void fill_resource_dictionaries_visitor::visit(list_node& /*node*/, i_xaml_node* /*parent_node*/)
    {
    }

    bool fill_resource_dictionaries_visitor::skip_children(i_xaml_node& node, i_xaml_node* parent_node)
    {
        if (dynamic_cast<element_node*>(&node) == nullptr)
        {
            return false;
        }
        return is_keyless_resource_dictionary_parent(*context_, parent_node);
    }

    bool fill_resource_dictionaries_visitor::is_resource_dictionary(element_node& node)
    {
        return is_resource_dictionary_node(*context_, &node);
    }

    // ================================================================================================
    // apply_properties_visitor  <=  ApplyPropertiesVisitor
    // ================================================================================================

    void apply_properties_visitor::apply_value(hydration_context& context, maui::core::bindable_object& target,
                                               maui::core::type_tag target_type, const std::string& local_name,
                                               const std::any& value, const std::optional<std::string>& x_key,
                                               int line_number, int line_position)
    {
        apply_value_core(env_from(context), target, target_type, local_name, value, x_key, line_number, line_position);
    }

    void apply_properties_visitor::visit_property_value(const std::any& value, i_xaml_node& node,
                                                        element_node& parent_element, const xml_name& property_name)
    {
        const std::any* source = context_->try_get_value(parent_element);
        if (source == nullptr && context_->has_handler())
        {
            return;
        }
        guarded(*context_, [this, &value, &node, &parent_element, &property_name, source] {
            maui::core::bindable_object* target = as_bindable(source);
            if (target == nullptr)
            {
                // RD value-properties (Source="…") and non-bindable sources are M7 deferrals.
                throw_cannot_assign(property_name.local_name, node.line_number(), node.line_position());
            }
            const maui::core::type_tag* target_type = context_->try_get_type(parent_element);
            if (target_type == nullptr)
            {
                throw_cannot_assign(property_name.local_name, node.line_number(), node.line_position());
            }
            const auto* element = dynamic_cast<const element_node*>(&node);
            const std::optional<std::string> x_key = element != nullptr ? x_key_of(*element) : std::nullopt;
            apply_value_core(env_from(*context_), *target, *target_type, property_name.local_name, value, x_key,
                             node.line_number(), node.line_position());
        });
    }

    void apply_properties_visitor::visit(value_node& node, i_xaml_node* parent_node)
    {
        auto* parent_element = dynamic_cast<element_node*>(parent_node);
        const std::any* value = context_->try_get_value(node);
        if (value == nullptr)
        {
            return;
        }
        const std::optional<xml_name> property_name = try_get_property_name(node, parent_node);
        if (property_name.has_value() && parent_element != nullptr)
        {
            // (TrySetRuntimeName — x:Name → StyleId — is the documented deviation; x:Name is in
            // Skips either way.)
            if (is_apply_skip(*property_name) || is_skip_property(*parent_element, *property_name) ||
                *property_name == xml_name::mc_ignorable())
            {
                return;
            }
            visit_property_value(*value, node, *parent_element, *property_name);
            return;
        }
        if (is_collection_item(node, parent_node) && parent_element != nullptr)
        {
            // Element text content routes to the [ContentProperty] VALUE name (Label → Text); a
            // parent without one ignores the text, like C#.
            const maui::core::type_tag* parent_type = context_->try_get_type(*parent_element);
            const std::string* content_property =
                parent_type != nullptr ? context_->property_registry().content_property(*parent_type) : nullptr;
            if (content_property == nullptr)
            {
                return;
            }
            const xml_name content_name{.namespace_uri = parent_element->namespace_uri(),
                                        .local_name = *content_property};
            if (is_skip_property(*parent_element, content_name))
            {
                return;
            }
            visit_property_value(*value, node, *parent_element, content_name);
        }
    }

    void apply_properties_visitor::visit(markup_node& node, i_xaml_node* parent_node)
    {
        auto* parent_element = dynamic_cast<element_node*>(parent_node);
        const std::optional<xml_name> property_name = try_get_property_name(node, parent_node);
        if (parent_element == nullptr || !property_name.has_value())
        {
            return; // unexpanded collection-item markup stays inert, like C#
        }
        if (is_apply_skip(*property_name) || is_skip_property(*parent_element, *property_name) ||
            *property_name == xml_name::mc_ignorable())
        {
            return;
        }
        const std::any* expansion = context_->try_get_value(node);
        if (expansion == nullptr)
        {
            return; // expansion failed under an exception handler (or was skipped)
        }
        const std::any* source = context_->try_get_value(*parent_element);
        if (source == nullptr && context_->has_handler())
        {
            return;
        }
        // A "{}"-escaped nested literal expands to a plain string — apply it like a value node.
        if (const auto* literal = std::any_cast<std::string>(expansion))
        {
            visit_property_value(std::any{*literal}, node, *parent_element, *property_name);
            return;
        }
        const auto* extension = std::any_cast<std::shared_ptr<i_markup_extension>>(expansion);
        if (extension == nullptr)
        {
            return;
        }
        guarded(*context_, [this, &node, parent_element, &property_name, source, extension] {
            const xaml_service_provider services =
                make_service_provider(*context_, node, as_bindable(source), property_name->local_name);
            const std::any provided = (*extension)->provide_value(services);
            visit_property_value(provided, node, *parent_element, *property_name);
        });
    }

    void apply_properties_visitor::visit_collection_item(const std::any& value, i_xaml_node& node,
                                                         i_xaml_node& parent_node)
    {
        auto* parent_element = dynamic_cast<element_node*>(&parent_node);
        if (parent_element != nullptr)
        {
            const std::any* source = context_->try_get_value(*parent_element);
            if (source == nullptr && context_->has_handler())
            {
                return;
            }
            guarded(*context_, [this, &value, &node, parent_element, source] {
                const auto* element = dynamic_cast<const element_node*>(&node);
                const std::optional<std::string> x_key = element != nullptr ? x_key_of(*element) : std::nullopt;

                // A ResourceDictionary parent collects the item under its x:Key.
                if (maui::controls::resource_dictionary* dictionary = as_dictionary(source))
                {
                    add_to_resource_dictionary(*dictionary, value, x_key, node.line_number(), node.line_position());
                    return;
                }

                maui::core::bindable_object* target = as_bindable(source);
                const maui::core::type_tag* target_type = context_->try_get_type(*parent_element);
                if (target == nullptr || target_type == nullptr)
                {
                    throw xaml_parse_exception(
                        std::format("Cannot set the content of {} as it doesn't have a ContentPropertyAttribute",
                                    parent_element->type().name()),
                        node.line_number(), node.line_position());
                }

                // [ContentProperty] VALUE name first (GetContentPropertyName), …
                const std::string* content_property = context_->property_registry().content_property(*target_type);
                if (content_property != nullptr)
                {
                    apply_value_core(env_from(*context_), *target, *target_type, *content_property, value, x_key,
                                     node.line_number(), node.line_position());
                    return;
                }
                // … then the child sink (C#'s IEnumerable + Add() walk).
                maui::core::bindable_object* child = as_bindable(&value);
                if (child != nullptr && context_->property_registry().try_add_child(*target_type, *target, *child))
                {
                    return;
                }
                throw xaml_parse_exception(
                    std::format("Cannot set the content of {} as it doesn't have a ContentPropertyAttribute",
                                parent_element->type().name()),
                    node.line_number(), node.line_position());
            });
            return;
        }

        auto* parent_list = dynamic_cast<list_node*>(&parent_node);
        if (parent_list == nullptr)
        {
            return;
        }
        // The ListNode branch: the items of <Type.Property> with several children.
        if (is_apply_skip(parent_list->name()))
        {
            return;
        }
        const auto* grandparent = dynamic_cast<const element_node*>(parent_list->parent());
        const std::any* source = grandparent != nullptr ? context_->try_get_value(*grandparent) : nullptr;
        if (source == nullptr && context_->has_handler())
        {
            return;
        }
        guarded(*context_, [this, &value, &node, parent_list, grandparent, source] {
            maui::core::bindable_object* target = as_bindable(source);
            const maui::core::type_tag* target_type =
                grandparent != nullptr ? context_->try_get_type(*grandparent) : nullptr;
            const std::string& list_name = parent_list->name().local_name;
            if (target == nullptr || target_type == nullptr)
            {
                throw xaml_parse_exception(std::format("Property {} is null or is not IEnumerable", list_name),
                                           node.line_number(), node.line_position());
            }
            const auto* element = dynamic_cast<const element_node*>(&node);
            const std::optional<std::string> x_key = element != nullptr ? x_key_of(*element) : std::nullopt;

            // Resources lists fill the element's dictionary; everything else goes through the named
            // child sink (the port of the IEnumerable + Add() walk).
            if (list_name == "Resources" || list_name.ends_with(".Resources"))
            {
                auto* owner = dynamic_cast<maui::controls::element*>(target);
                if (owner == nullptr)
                {
                    throw xaml_parse_exception(std::format("Property {} is null or is not IEnumerable", list_name),
                                               node.line_number(), node.line_position());
                }
                add_to_resource_dictionary(owner->resources(), value, x_key, node.line_number(), node.line_position());
                return;
            }
            maui::core::bindable_object* child = as_bindable(&value);
            if (child != nullptr && context_->property_registry().is_child_property(*target_type, list_name) &&
                context_->property_registry().try_add_child(*target_type, *target, *child))
            {
                return;
            }
            throw xaml_parse_exception(std::format("Property {} is null or is not IEnumerable", list_name),
                                       node.line_number(), node.line_position());
        });
    }

    void apply_properties_visitor::visit(element_node& node, i_xaml_node* parent_node)
    {
        // _CreateContent values are DataTemplate bodies — templates are an M7 deferral (their TYPE
        // already fails creation), so the template hook is inert here.
        const std::optional<xml_name> direct_name = try_get_property_name(node, parent_node);
        if (direct_name.has_value() && *direct_name == xml_name::create_content())
        {
            return;
        }

        // A ResourceDictionary node was fully handled by the fill pass (assignment + items).
        if (as_dictionary(context_->try_get_value(node)) != nullptr)
        {
            return;
        }

        // "Simplify ListNodes with single elements".
        xml_name property_name = xml_name::empty();
        i_xaml_node* effective_parent = parent_node;
        auto* parent_list = dynamic_cast<list_node*>(parent_node);
        if (parent_list != nullptr && parent_list->collection_items().size() == 1)
        {
            property_name = parent_list->name();
            effective_parent = parent_list->parent();
        }

        const std::any* value = context_->try_get_value(node);
        if (value == nullptr)
        {
            return; // creation failed under an exception handler
        }

        if (property_name == xml_name::empty())
        {
            const std::optional<xml_name> found = try_get_property_name(node, effective_parent);
            if (found.has_value())
            {
                property_name = *found;
            }
        }

        auto* parent_element = dynamic_cast<element_node*>(effective_parent);
        if (property_name != xml_name::empty() && parent_element != nullptr)
        {
            if (is_apply_skip(property_name) || is_skip_property(*parent_element, property_name))
            {
                return;
            }
            visit_property_value(*value, node, *parent_element, property_name);
            return;
        }
        if (is_collection_item(node, parent_node))
        {
            visit_collection_item(*value, node, *parent_node);
        }
    }

    void apply_properties_visitor::visit(root_node& /*node*/, i_xaml_node* /*parent_node*/)
    {
    }
    void apply_properties_visitor::visit(list_node& /*node*/, i_xaml_node* /*parent_node*/)
    {
    }
    bool apply_properties_visitor::skip_children(i_xaml_node& /*node*/, i_xaml_node* /*parent_node*/)
    {
        return false;
    }
    bool apply_properties_visitor::is_resource_dictionary(element_node& node)
    {
        const maui::core::type_tag* type = context_->try_get_type(node);
        return type != nullptr && *type == maui::core::type_tag::of<maui::controls::resource_dictionary>();
    }
} // namespace maui::xaml
