// maui::xaml — the loader's visitor pipeline (xaml_visitors.hpp), ported from
// src/Controls/src/Xaml/{ExpandMarkupsVisitor,PruneIgnoredNodesVisitor,NamescopingVisitor,
// CreateValuesVisitor,RegisterXNamesVisitor,FillResourceDictionariesVisitor,
// ApplyPropertiesVisitor}.cs (+ the MarkupExpressionParser recursion of ExpandMarkupsVisitor's
// MarkupExpansionParser). The reflection-free substitutions are documented in the header.
#include "maui/xaml/xaml_visitors.hpp"

#include <algorithm>
#include <any>
#include <charconv>
#include <cstddef>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include "maui/controls/absolute_layout.hpp" // W10: AbsoluteLayout.LayoutBounds/LayoutFlags attached props
#include "maui/controls/application.hpp"
#include "maui/controls/brushes/brush.hpp" // W7: element-form brush object-coercion for Background
#include "maui/controls/dynamic_resource.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/flex_layout.hpp"       // W11: FlexLayout.Grow/Shrink/Basis/Order/AlignSelf attached props
#include "maui/controls/font_image_source.hpp" // W17: <FontImageSource> element form (Image.Source)
#include "maui/controls/formatted_string.hpp"  // W8: element-form formatted_string object-coercion (FormattedText)
#include "maui/controls/grid.hpp"
#include "maui/controls/items/items_view.hpp" // W4: ItemTemplate target (CollectionView/CarouselView)
#include "maui/controls/picker.hpp"           // W12: <Picker.Items> x:String child sink
#include "maui/controls/resource_dictionary.hpp"
#include "maui/controls/setter.hpp"
#include "maui/controls/style.hpp"
#include "maui/controls/templates/control_template.hpp" // W16: <ControlTemplate> minting (DataTemplate sibling)
#include "maui/controls/templates/data_template.hpp"    // W4: <DataTemplate> minting + the loader factory
#include "maui/core/app_theme.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/type_tag.hpp"
#include "maui/graphics/i_shape.hpp" // W9: Border.StrokeShape object-coercion (controls::shape is-a i_shape)
#include "maui/graphics/shapes/round_rectangle.hpp" // W9: <RoundRectangle CornerRadius=…> minting (no controls equivalent)
#include "maui/layouts/flex_basis.hpp"              // W11
#include "maui/layouts/flex_enums.hpp"              // W11: flex_align_self
#include "maui/xaml/hydration_context.hpp"
#include "maui/xaml/i_markup_extension.hpp"
#include "maui/xaml/markup_extensions.hpp"
#include "maui/xaml/name_scope.hpp"
#include "maui/xaml/xaml_binding_applier.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_converters.hpp" // W2: convert_grid_length for element-form Row/ColumnDefinition
#include "maui/xaml/xaml_node.hpp"
#include "maui/xaml/xaml_parse_exception.hpp"
#include "maui/xaml/xaml_parser.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"

#include "maui/xaml/xaml_template_inflater.hpp" // W4: DataTemplate body inflation (the loader factory)

#include "xaml_style_builder.hpp" // W3: loader-side <Style>/<Setter> resolution

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

        // W3: the boxed shape a minted <Style> carries in context values (shared_ptr<style>), or null.
        [[nodiscard]] std::shared_ptr<maui::controls::style> as_style(const std::any* value)
        {
            if (value == nullptr)
            {
                return nullptr;
            }
            const auto* stored = std::any_cast<std::shared_ptr<maui::controls::style>>(value);
            return stored != nullptr ? *stored : nullptr;
        }

        // W3: whether an element node is a <Style> in the maui namespace (the loader special-case gate,
        // mirroring the ResourceDictionary one). Style is not a bindable_object, so it is never in the
        // type registry — recognized purely by name + namespace here.
        [[nodiscard]] bool is_style_element(const element_node& node)
        {
            return node.type().is_of_any_type({"Style"});
        }

        // W3: read a plain-literal attribute (e.g. TargetType, Property, Value, BasedOn, Class) off a
        // Style/Setter element node's property map. Returns nullopt when the attribute is absent or is not
        // a value_node literal (a markup-valued attribute is handled separately by the caller).
        [[nodiscard]] std::optional<std::string> literal_attribute(const element_node& node,
                                                                   std::string_view local_name)
        {
            xml_name matched = xml_name::empty();
            const std::shared_ptr<i_xaml_node> attribute = node.properties().try_get(local_name, matched);
            if (attribute == nullptr)
            {
                return std::nullopt;
            }
            const auto* literal = dynamic_cast<const value_node*>(attribute.get());
            return literal != nullptr ? std::optional<std::string>{literal->value()} : std::nullopt;
        }

        // W3: extract the resource KEY from a Style's BasedOn attribute. MAUI authors BasedOn as
        // BasedOn="{StaticResource baseKey}"; the port keeps it LAZY by storing the key as the style's
        // base_resource_key (resolved at apply time via the resource_resolver, so a forward-reference is
        // tolerated). The attribute is a markup_node after the expand pass; its markup_string is parsed
        // here for the key. A bare-literal BasedOn (rare) is also accepted as a key. Returns nullopt when
        // there is no BasedOn or it is not a recognizable StaticResource key.
        [[nodiscard]] std::optional<std::string> based_on_key(const element_node& node)
        {
            xml_name matched = xml_name::empty();
            const std::shared_ptr<i_xaml_node> attribute = node.properties().try_get("BasedOn", matched);
            if (attribute == nullptr)
            {
                return std::nullopt;
            }
            if (const auto* literal = dynamic_cast<const value_node*>(attribute.get()))
            {
                const std::string& text = literal->value();
                return text.empty() ? std::nullopt : std::optional<std::string>{text};
            }
            const auto* markup = dynamic_cast<const markup_node*>(attribute.get());
            if (markup == nullptr)
            {
                return std::nullopt;
            }
            // Parse "{StaticResource key}" textually (key may be the bare value or "ResourceKey=key").
            std::string_view text = markup->markup_string();
            const auto trim = [](std::string_view value) {
                const std::size_t begin = value.find_first_not_of(" \t");
                if (begin == std::string_view::npos)
                {
                    return std::string_view{};
                }
                const std::size_t end = value.find_last_not_of(" \t");
                return value.substr(begin, end - begin + 1);
            };
            text = trim(text);
            if (!text.starts_with('{') || !text.ends_with('}'))
            {
                return std::nullopt;
            }
            text = trim(text.substr(1, text.size() - 2));
            constexpr std::string_view marker = "StaticResource";
            if (!text.starts_with(marker))
            {
                return std::nullopt;
            }
            std::string_view argument = trim(text.substr(marker.size()));
            constexpr std::string_view named = "ResourceKey=";
            if (argument.starts_with(named))
            {
                argument = trim(argument.substr(named.size()));
            }
            return argument.empty() ? std::nullopt : std::optional<std::string>{std::string{argument}};
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
            std::vector<std::function<void()>>* deferred_attached = nullptr;     // null = no deferral (re-apply)
        };

        [[nodiscard]] applier_env env_from(hydration_context& context)
        {
            return {.properties = &context.property_registry(),
                    .converters = &context.converter_registry(),
                    .application = context.application,
                    .subscriptions = &context.subscriptions(),
                    .deferred_attached = &context.deferred_attached()};
        }

        [[noreturn]] void throw_cannot_assign(const std::string& local_name, int line_number, int line_position)
        {
            // TrySetPropertyValue's catch-all message, verbatim.
            throw xaml_parse_exception(std::format("Cannot assign property \"{}\": Property does not exist, or is "
                                                   "not assignable, or mismatching type between value and property",
                                                   local_name),
                                       line_number, line_position);
        }

        // Attached property on a CHILD (e.g. "Grid.Row"): the markup sets it on the child, but the value is
        // stored by the PARENT layout — the port keeps it in the parent's per-child side-map (PROFILE §7
        // removed C#'s central attached-property bag, so SetRow(child, n) lives on the grid). C# resolves the
        // declaring type's attached BindableProperty through reflection; the reflection-free port special-cases
        // the in-scope Grid attached set here.
        //
        // The value is parsed + validated NOW (so a bad integer fails at its own line), but the PLACEMENT is
        // DEFERRED: a child's attached attribute is applied BEFORE add() parents it into the layout, so the
        // owning grid is not reachable yet (logical_parent() is still null). The loader drains the deferred
        // closures once the whole tree is parented. A child that ends up outside a Grid is silently left
        // unplaced, exactly as MAUI ignores a stray Grid.Row. Returns false for any non-attached / unknown
        // dotted name → the caller falls through to its catch-all error.
        // W10 — AbsoluteLayout.LayoutBounds / LayoutFlags parse the [Flags] enum string (None / *Proportional
        // combos). Mirrors C# AbsoluteLayoutFlagsTypeConverter; comma-separated like a [Flags] Enum.Parse.
        [[nodiscard]] maui::layouts::absolute_layout_flags parse_absolute_layout_flags(std::string_view text)
        {
            using maui::layouts::absolute_layout_flags;
            absolute_layout_flags result = absolute_layout_flags::none;
            std::size_t start = 0;
            while (start <= text.size())
            {
                const std::size_t comma = text.find(',', start);
                const std::size_t end = comma == std::string_view::npos ? text.size() : comma;
                std::string_view token = text.substr(start, end - start);
                while (!token.empty() && token.front() == ' ')
                {
                    token.remove_prefix(1);
                }
                while (!token.empty() && token.back() == ' ')
                {
                    token.remove_suffix(1);
                }
                if (token == "XProportional")
                {
                    result = result | absolute_layout_flags::x_proportional;
                }
                else if (token == "YProportional")
                {
                    result = result | absolute_layout_flags::y_proportional;
                }
                else if (token == "WidthProportional")
                {
                    result = result | absolute_layout_flags::width_proportional;
                }
                else if (token == "HeightProportional")
                {
                    result = result | absolute_layout_flags::height_proportional;
                }
                else if (token == "PositionProportional")
                {
                    result = result | absolute_layout_flags::position_proportional;
                }
                else if (token == "SizeProportional")
                {
                    result = result | absolute_layout_flags::size_proportional;
                }
                else if (token == "All")
                {
                    result = absolute_layout_flags::all;
                }
                else if (!token.empty() && token != "None")
                {
                    throw xaml_convert_error("Invalid AbsoluteLayoutFlags value: " + std::string(token));
                }
                if (comma == std::string_view::npos)
                {
                    break;
                }
                start = comma + 1;
            }
            return result;
        }

        [[nodiscard]] bool try_apply_attached_property(maui::core::bindable_object& target,
                                                       const std::string& local_name, const std::any& value,
                                                       std::vector<std::function<void()>>* deferred, int line_number,
                                                       int line_position)
        {
            // W10 — AbsoluteLayout.LayoutBounds (a Rect "x,y,w,h") / LayoutFlags (the [Flags] enum). Like the
            // Grid attached set below, these are set on the CHILD before add() parents it into the layout, so the
            // placement is DEFERRED until the child's logical_parent (the absolute_layout) is reachable.
            constexpr std::string_view al_prefix = "AbsoluteLayout.";
            if (local_name.starts_with(al_prefix))
            {
                const std::string_view property = std::string_view{local_name}.substr(al_prefix.size());
                if (property != "LayoutBounds" && property != "LayoutFlags")
                {
                    return false;
                }
                auto* view = dynamic_cast<maui::core::i_view*>(&target);
                auto* element = dynamic_cast<maui::controls::element*>(&target);
                const auto* text = std::any_cast<std::string>(&value);
                if (view == nullptr || element == nullptr || text == nullptr)
                {
                    return false;
                }
                if (deferred == nullptr)
                {
                    return true; // re-apply path: nothing to place
                }
                if (property == "LayoutBounds")
                {
                    maui::graphics::rect bounds;
                    try
                    {
                        bounds = convert_rect(*text);
                    }
                    catch (const xaml_convert_error& error)
                    {
                        throw xaml_parse_exception(error.what(), line_number, line_position);
                    }
                    deferred->emplace_back([view, element, bounds] {
                        if (auto* al = dynamic_cast<maui::controls::absolute_layout*>(element->logical_parent()))
                        {
                            al->set_layout_bounds(*view, bounds);
                        }
                    });
                }
                else
                {
                    maui::layouts::absolute_layout_flags flags{};
                    try
                    {
                        flags = parse_absolute_layout_flags(*text);
                    }
                    catch (const xaml_convert_error& error)
                    {
                        throw xaml_parse_exception(error.what(), line_number, line_position);
                    }
                    deferred->emplace_back([view, element, flags] {
                        if (auto* al = dynamic_cast<maui::controls::absolute_layout*>(element->logical_parent()))
                        {
                            al->set_layout_flags(*view, flags);
                        }
                    });
                }
                return true;
            }

            // W11 — FlexLayout.Grow/Shrink (float) / Basis (flex_basis) / Order (int) / AlignSelf (enum):
            // the per-child flex attached values. Deferred + applied via the parent flex_layout's setters,
            // like AbsoluteLayout/Grid. The container-level props (Direction/JustifyContent/…) are plain
            // bindable registrations in register_xaml_layouts; these per-child ones live here.
            constexpr std::string_view flex_prefix = "FlexLayout.";
            if (local_name.starts_with(flex_prefix))
            {
                const std::string_view property = std::string_view{local_name}.substr(flex_prefix.size());
                const bool known = property == "Grow" || property == "Shrink" || property == "Basis" ||
                                   property == "Order" || property == "AlignSelf";
                if (!known)
                {
                    return false;
                }
                auto* view = dynamic_cast<maui::core::i_view*>(&target);
                auto* element = dynamic_cast<maui::controls::element*>(&target);
                const auto* text = std::any_cast<std::string>(&value);
                if (view == nullptr || element == nullptr || text == nullptr)
                {
                    return false;
                }
                if (deferred == nullptr)
                {
                    return true;
                }
                // Parse now (so a bad literal fails loudly at its node), capture the typed value, place later.
                std::function<void(maui::controls::flex_layout&, maui::core::i_view&)> place;
                try
                {
                    if (property == "Grow")
                    {
                        const float grow = convert_float(*text);
                        place = [grow](maui::controls::flex_layout& flex, maui::core::i_view& child) {
                            flex.set_grow(child, grow);
                        };
                    }
                    else if (property == "Shrink")
                    {
                        const float shrink = convert_float(*text);
                        place = [shrink](maui::controls::flex_layout& flex, maui::core::i_view& child) {
                            flex.set_shrink(child, shrink);
                        };
                    }
                    else if (property == "Basis")
                    {
                        const maui::layouts::flex_basis basis = convert_flex_basis(*text);
                        place = [basis](maui::controls::flex_layout& flex, maui::core::i_view& child) {
                            flex.set_basis(child, basis);
                        };
                    }
                    else if (property == "AlignSelf")
                    {
                        const maui::layouts::flex_align_self align = convert_flex_align_self(*text);
                        place = [align](maui::controls::flex_layout& flex, maui::core::i_view& child) {
                            flex.set_align_self(child, align);
                        };
                    }
                    else // Order
                    {
                        const int order = convert_int(*text);
                        place = [order](maui::controls::flex_layout& flex, maui::core::i_view& child) {
                            flex.set_order(child, order);
                        };
                    }
                }
                catch (const xaml_convert_error& error)
                {
                    throw xaml_parse_exception(error.what(), line_number, line_position);
                }
                deferred->emplace_back([view, element, place = std::move(place)] {
                    if (auto* flex = dynamic_cast<maui::controls::flex_layout*>(element->logical_parent()))
                    {
                        place(*flex, *view);
                    }
                });
                return true;
            }

            constexpr std::string_view grid_prefix = "Grid.";
            if (!local_name.starts_with(grid_prefix))
            {
                return false;
            }
            const std::string_view property = std::string_view{local_name}.substr(grid_prefix.size());
            const bool known =
                property == "Row" || property == "Column" || property == "RowSpan" || property == "ColumnSpan";
            if (!known)
            {
                return false; // an unrecognized Grid.* attribute → caller's catch-all error
            }

            auto* view = dynamic_cast<maui::core::i_view*>(&target);
            auto* element = dynamic_cast<maui::controls::element*>(&target);
            if (view == nullptr || element == nullptr)
            {
                return false;
            }

            // The Grid attached properties are all int; the attribute arrives as raw text ("0").
            const auto* text = std::any_cast<std::string>(&value);
            if (text == nullptr)
            {
                return false;
            }
            int parsed = 0;
            const auto* const begin = text->data();
            const auto* const end = begin + text->size();
            const auto [stop, error] = std::from_chars(begin, end, parsed);
            if (error != std::errc{} || stop != end)
            {
                throw xaml_parse_exception(
                    std::format("Cannot set \"{}\": \"{}\" is not a valid integer", local_name, *text), line_number,
                    line_position);
            }

            // No deferral sink (the {AppThemeBinding} re-apply path): nothing to place, accept the value.
            if (deferred == nullptr)
            {
                return true;
            }
            deferred->emplace_back([view, element, prop = std::string{property}, parsed] {
                auto* grid = dynamic_cast<maui::controls::grid*>(element->logical_parent());
                if (grid == nullptr)
                {
                    return; // the child is not inside a Grid → no placement, as MAUI does
                }
                if (prop == "Row")
                {
                    grid->set_row(*view, parsed);
                }
                else if (prop == "Column")
                {
                    grid->set_column(*view, parsed);
                }
                else if (prop == "RowSpan")
                {
                    grid->set_row_span(*view, parsed);
                }
                else if (prop == "ColumnSpan")
                {
                    grid->set_column_span(*view, parsed);
                }
            });
            return true;
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
        // W2 — Grid.RowDefinitions / Grid.ColumnDefinitions element form. The <RowDefinition>/
        // <ColumnDefinition> items mint plain row_definition/column_definition VALUES (a create_values
        // special-case), not bindable_objects, so they bypass both the registered-property surface and
        // the bindable child sink; they push straight into the grid's definition vectors (mirrors C#
        // RowDefinitionCollection.Add — appended in document order). Returns true when it consumed the
        // value. The string form Grid.RowDefinitions="Auto,*" is the converter twin (xaml_standard_types)
        // — same grid_length parse, different markup shape.
        [[nodiscard]] bool try_add_grid_definition(maui::core::bindable_object& target,
                                                   const std::string& property_name, const std::any& value)
        {
            auto* grid = dynamic_cast<maui::controls::grid*>(&target);
            if (grid == nullptr)
            {
                return false;
            }
            if (property_name == "RowDefinitions")
            {
                if (const auto* definition = std::any_cast<maui::controls::row_definition>(&value))
                {
                    grid->add_row_definition(definition->height());
                    return true;
                }
            }
            else if (property_name == "ColumnDefinitions")
            {
                if (const auto* definition = std::any_cast<maui::controls::column_definition>(&value))
                {
                    grid->add_column_definition(definition->width());
                    return true;
                }
            }
            return false;
        }

        // W12 — element-form <Picker.Items> with <x:String> children. Like the Grid definitions above,
        // the items are plain std::string VALUES (created by the x:String primitive route), so they
        // bypass both the registered-property surface and the bindable child sink; each is pushed onto
        // the picker's Items face (mirrors C# Picker.Items.Add — appended in document order, which the
        // C# gallery PickerPage.xaml uses for its <Picker.Items> markup). Returns true when consumed.
        [[nodiscard]] bool try_add_picker_item(maui::core::bindable_object& target, const std::string& property_name,
                                               const std::any& value)
        {
            if (property_name != "Items")
            {
                return false;
            }
            auto* picker = dynamic_cast<maui::controls::picker*>(&target);
            if (picker == nullptr)
            {
                return false;
            }
            if (const auto* item = std::any_cast<std::string>(&value))
            {
                picker->items().add(*item);
                return true;
            }
            return false;
        }

        // W13 — element-form <CollectionView.ItemsSource><x:Array Type="{x:Type x:String}"><x:String>…
        // The <x:Array> create-pass mints an xaml_array carrying its item children (here std::string from
        // the <x:String> primitives); the ItemsSource property is a shared_ptr<i_item_collection> set only
        // via {Binding} (no text/object converter), so a static inline array bypasses the registered
        // surface and builds a fixed-snapshot item_collection here (the C# array-ItemsSource: a vector<T>
        // source whose changed() is null). Mirrors try_add_picker_item / try_add_grid_definition. Returns
        // true when consumed. (Only string element types are supported — the gallery's inline lists are
        // caption strings; a non-string item falls through to the normal "cannot assign" path.)
        [[nodiscard]] bool try_set_items_source_from_array(maui::core::bindable_object& target,
                                                           const std::string& property_name, const std::any& value)
        {
            if (property_name != "ItemsSource")
            {
                return false;
            }
            const auto* array = std::any_cast<xaml_array>(&value);
            if (array == nullptr)
            {
                return false;
            }
            auto* view = dynamic_cast<maui::controls::items_view*>(&target);
            if (view == nullptr)
            {
                return false;
            }
            std::vector<std::string> strings;
            strings.reserve(array->items.size());
            for (const std::any& item : array->items)
            {
                const auto* text = std::any_cast<std::string>(&item);
                if (text == nullptr)
                {
                    return false; // a non-string element type — not the string-list subset W13 supports
                }
                strings.push_back(*text);
            }
            view->set_items_source(std::move(strings));
            return true;
        }

        // W7/W8 — element-form object-property coercion. A property element value is a CREATED element (boxed
        // as shared_ptr<bindable_object> by register_type), but the property expects shared_ptr<Derived>
        // (Derived : bindable_object) — e.g. Background<-brush, FormattedText<-formatted_string. try_set's
        // exact any_cast can't downcast, so re-box via dynamic_pointer_cast and retry. Returns true if it set.
        template <class Derived>
        [[nodiscard]] bool try_set_created_object(const applier_env& env, maui::core::bindable_object& target,
                                                  maui::core::type_tag target_type, const std::string& local_name,
                                                  const xaml_property_registry::property_entry& entry,
                                                  const std::any& value)
        {
            if (entry.value_type != maui::core::type_tag::of<std::shared_ptr<Derived>>())
            {
                return false;
            }
            const auto* object = std::any_cast<std::shared_ptr<maui::core::bindable_object>>(&value);
            if (object == nullptr)
            {
                return false;
            }
            std::shared_ptr<Derived> derived = std::dynamic_pointer_cast<Derived>(*object);
            if (!derived)
            {
                return false;
            }
            return env.properties->try_set(target_type, target, local_name, std::any{std::move(derived)});
        }

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

            // W2 — element-form Grid row/column definitions route to the grid's vectors, not the
            // registered-property table (this is the single-<RowDefinition>-child path; the multi-child
            // list path is handled in visit_collection_item).
            if (try_add_grid_definition(target, local_name, value))
            {
                return;
            }

            // W12 — element-form <Picker.Items> string items (single-<x:String>-child path; the
            // multi-child list path is handled in visit_collection_item).
            if (try_add_picker_item(target, local_name, value))
            {
                return;
            }

            // W13 — element-form <CollectionView.ItemsSource><x:Array> static string-list items source.
            if (try_set_items_source_from_array(target, local_name, value))
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
                current_xaml_binding_applier()(*env.properties, target, target_type, local_name, *request, line_number,
                                               line_position);
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
                // W7/W8/W9 — element-form object property: a created element (boxed as shared_ptr<bindable_object>)
                // assigned to a property typed shared_ptr<Derived> is coerced via dynamic_pointer_cast so
                // try_set's exact any_cast succeeds. Covers Background (brush, e.g.
                // <BoxView.Background><LinearGradientBrush/>), Label.FormattedText (formatted_string), and
                // Border.StrokeShape (graphics::i_shape — controls::shape multiply-inherits it, so a
                // <Border.StrokeShape><Ellipse/> coerces). The string forms took the converter path above.
                if (try_set_created_object<maui::controls::brush>(env, target, target_type, local_name, *entry,
                                                                  value) ||
                    try_set_created_object<maui::controls::formatted_string>(env, target, target_type, local_name,
                                                                             *entry, value) ||
                    try_set_created_object<maui::graphics::i_shape>(env, target, target_type, local_name, *entry,
                                                                    value))
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

            // Attached property (Grid.Row, …): set on the child, stored by the parent layout (deferred until
            // the child is parented — see try_apply_attached_property).
            if (try_apply_attached_property(target, local_name, value, env.deferred_attached, line_number,
                                            line_position))
            {
                return;
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
                // IReferenceProvider: the nearest enclosing element's name scope (the register_x_names
                // pass already populated it). The first scope found wins (nearest-first walk) — the
                // {x:Reference} resolution surface (ReferenceExtension.FindByName + the ParentObjects
                // fallback collapse into one scope lookup here).
                if (services.reference_provider == nullptr && ancestor_element->scope_ref() &&
                    ancestor_element->scope_ref()->scope)
                {
                    services.reference_provider = ancestor_element->scope_ref()->scope.get();
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
            const std::vector<std::pair<xml_name, std::shared_ptr<i_xaml_node>>> properties{node.properties().begin(),
                                                                                            node.properties().end()};
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
                else if (name == "Array")
                {
                    // W13 — element form <x:Array Type="{x:Type x:String}"><x:String>…</x:String></x:Array>
                    // (Microsoft.Maui.Controls.Xaml.ArrayExtension as an element). The create pass is
                    // bottom-up, so the item children are already created VALUES (x:String → std::string)
                    // in the context; gather them into an xaml_array here (the same marker the curly
                    // {x:Array} extension mints). The Type attribute is left to the apply-pass skip (it is
                    // a {x:Type} markup on a non-bindable value, like a <RowDefinition>'s Height). The
                    // xaml_array → ItemsSource conversion happens in apply_value_core
                    // (try_set_items_source_from_array). Items in document order, mirroring C# Array order.
                    xaml_array array;
                    for (const std::shared_ptr<i_xaml_node>& child : node.collection_items())
                    {
                        if (const std::any* item = context_->try_get_value(*child);
                            item != nullptr && item->has_value())
                        {
                            array.items.push_back(*item);
                        }
                    }
                    context_->set_value(node, std::any{std::move(array)});
                    context_->set_type(node, maui::core::type_tag::of<xaml_array>());
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

            // W3 — <Style>: not a bindable_object either (so not in the type registry), minted here like
            // a ResourceDictionary. The create pass is bottom-up, so child <Setter> nodes are visited
            // FIRST; the Style is minted as an EMPTY shell now (TargetType / x:Key / BasedOn / Class /
            // ApplyToDerivedTypes are plain literals available at create time) and its setters are filled
            // in the apply pass, when each <Setter> can walk to this resolved shell (mirrors C#
            // IValueProvider.ProvideValue at apply time). The shared_ptr<style> is owned by the context's
            // keep-alive list, and a copy is the node's boxed value (so the apply pass + the explicit
            // Style-property / resource routing all reach the same instance).
            if (is_style_element(node))
            {
                const std::optional<std::string> target_type_name = literal_attribute(node, "TargetType");
                const maui::core::type_tag target_type =
                    resolve_target_type(target_type_name.value_or(std::string{}), node.namespace_uri(),
                                        context_->type_registry(), node.line_number(), node.line_position());
                std::optional<std::string> apply_to_derived = literal_attribute(node, "ApplyToDerivedTypes");
                const bool apply_to_derived_types =
                    apply_to_derived.has_value() && (*apply_to_derived == "true" || *apply_to_derived == "True");
                std::shared_ptr<maui::controls::style> built = build_style(
                    target_type, based_on_key(node), literal_attribute(node, "Class"), apply_to_derived_types);
                context_->set_type(node, target_type); // the Setter resolution reads the TargetType here
                context_->set_value(node, std::any{built});
                context_->keep_alive(built);
                return;
            }

            // W3 — <Setter>: a collection item of a <Style>. It has no created value of its own; the
            // apply pass resolves its Property/Value against the parent Style's TargetType and adds the
            // built setter to the parent's minted style. Recognized here so it no longer fails the
            // "Type Setter not found" type-registry lookup below. (A stray <Setter> outside a <Style>
            // stays inert — the apply pass ignores it, mirroring C# where a Setter only means something
            // to its Style parent.)
            if (node.type().is_of_any_type({"Setter"}))
            {
                return;
            }

            // W4 — <DataTemplate>: not a bindable_object (so not in the type registry), minted here like
            // a ResourceDictionary / Style. The create pass STOPS on the data-template body (its
            // _CreateContent child is NOT created now — stop_on_data_template==true), so only the EMPTY
            // template shell is minted in this pass; the apply pass installs the loader that lazily
            // inflates a fresh copy of the captured body per item (apply_properties_visitor::set_template).
            // An EMPTY <DataTemplate/> (no _CreateContent child) leaves the loader unset, so
            // create_content() returns the element_template Label fallback (C# LoaderTests.TestEmptyTemplate).
            // The value is the shared_ptr<data_template> itself (so the standard object->property routing
            // sets CollectionView.ItemTemplate via register_property<items_view, shared_ptr<data_template>>);
            // it is also kept alive by the context for the tree's lifetime.
            if (node.type().is_of_any_type({"DataTemplate"}))
            {
                auto tmpl = std::make_shared<maui::controls::data_template>();
                context_->set_value(node, std::any{tmpl});
                context_->set_type(node, maui::core::type_tag::of<maui::controls::data_template>());
                context_->keep_alive(tmpl);
                return;
            }

            // W16 — <ControlTemplate>: the DataTemplate sibling (also an element_template, not a
            // bindable_object, so name-special-cased by the parser + minted here, never register_type'd).
            // The empty shell is minted now; set_template installs the body loader in the apply pass. The
            // value is shared_ptr<control_template> so the object→property routing sets a control's
            // ControlTemplate (register_property<templated_view, shared_ptr<control_template>>).
            if (node.type().is_of_any_type({"ControlTemplate"}))
            {
                auto tmpl = std::make_shared<maui::controls::control_template>();
                context_->set_value(node, std::any{tmpl});
                context_->set_type(node, maui::core::type_tag::of<maui::controls::control_template>());
                context_->keep_alive(tmpl);
                return;
            }

            // W2 — <RowDefinition>/<ColumnDefinition>: the element form of Grid.RowDefinitions /
            // Grid.ColumnDefinitions. Not bindable_objects (PROFILE — the port models them as plain value
            // types, RowDefinition.cs/ColumnDefinition.cs reduced to their single Height/Width), so not in
            // the type registry; minted here from that one grid_length attribute (default Star, matching the
            // C# RowDefinition()/ColumnDefinition() ctor) and boxed BY VALUE. The apply pass pulls each
            // minted definition out of its <Grid.RowDefinitions> property/list and pushes it onto the grid
            // (try_add_grid_definition). The converter throws xaml_convert_error; translate it to the loader's
            // single xaml_parse_exception channel so guarded() (which only catches the latter) routes it.
            if (node.type().is_of_any_type({"RowDefinition"}))
            {
                const std::optional<std::string> height = literal_attribute(node, "Height");
                maui::core::grid_length length = maui::core::grid_length::star();
                if (height.has_value())
                {
                    try
                    {
                        length = convert_grid_length(*height);
                    }
                    catch (const xaml_convert_error& error)
                    {
                        throw xaml_parse_exception(error.what(), node.line_number(), node.line_position());
                    }
                }
                context_->set_value(node, std::any{maui::controls::row_definition{length}});
                context_->set_type(node, maui::core::type_tag::of<maui::controls::row_definition>());
                return;
            }
            if (node.type().is_of_any_type({"ColumnDefinition"}))
            {
                const std::optional<std::string> width = literal_attribute(node, "Width");
                maui::core::grid_length length = maui::core::grid_length::star();
                if (width.has_value())
                {
                    try
                    {
                        length = convert_grid_length(*width);
                    }
                    catch (const xaml_convert_error& error)
                    {
                        throw xaml_parse_exception(error.what(), node.line_number(), node.line_position());
                    }
                }
                context_->set_value(node, std::any{maui::controls::column_definition{length}});
                context_->set_type(node, maui::core::type_tag::of<maui::controls::column_definition>());
                return;
            }

            // W9 — <RoundRectangle CornerRadius="…">: the rounded Border.StrokeShape. The port has NO controls
            // RoundRectangle (only the non-bindable graphics::round_rectangle, an i_shape), so it can't be
            // register_type'd; minted here from the CornerRadius literal and boxed AS shared_ptr<i_shape> (the
            // StrokeShape property's exact type, so apply_value_core's any_cast matches directly — no
            // coercion). The other stroke shapes (Ellipse/Rectangle/Polygon) ARE register_type'd controls
            // shapes that multiply-inherit i_shape, reaching StrokeShape via the apply object-coercion instead.
            // CornerRadius is consumed here, so the apply pass skips it (see visit(value_node)).
            if (node.type().is_of_any_type({"RoundRectangle"}))
            {
                maui::graphics::corner_radius radius{};
                const std::optional<std::string> corner = literal_attribute(node, "CornerRadius");
                if (corner.has_value())
                {
                    try
                    {
                        radius = convert_corner_radius(*corner);
                    }
                    catch (const xaml_convert_error& error)
                    {
                        throw xaml_parse_exception(error.what(), node.line_number(), node.line_position());
                    }
                }
                context_->set_value(node, std::any{std::shared_ptr<maui::graphics::i_shape>(
                                              std::make_shared<maui::graphics::shapes::round_rectangle>(radius))});
                context_->set_type(node, maui::core::type_tag::of<maui::graphics::shapes::round_rectangle>());
                return;
            }

            // W17 — <FontImageSource Glyph="…" FontFamily="…" Size="…" Color="…" FontAutoScalingEnabled="…">:
            // the element form of Image.Source (the real C# ImagePage uses it). The port's font_image_source
            // is a CTOR-ONLY i_image_source (NOT a bindable_object, so not register_type'd) — like
            // <RoundRectangle>, its attributes are consumed here and it is minted boxed AS
            // shared_ptr<i_image_source> (Image.Source's exact type — image::source_property(), so
            // apply_value_core's any_cast matches directly). Glyph/FontFamily are literals; Size/Color parse
            // via convert_double/convert_color; FontFamily+Size+AutoScaling compose onto the font.
            if (node.type().is_of_any_type({"FontImageSource"}))
            {
                const std::string glyph = literal_attribute(node, "Glyph").value_or(std::string{});
                const std::string family = literal_attribute(node, "FontFamily").value_or(std::string{});
                double size = maui::controls::font_image_source::default_size; // C# SizeProperty default 30
                maui::graphics::color color{};                                 // C# default (transparent)
                bool auto_scaling = false;                                     // C# FontAutoScalingEnabled default
                try
                {
                    if (const std::optional<std::string> s = literal_attribute(node, "Size"); s.has_value())
                    {
                        size = convert_double(*s);
                    }
                    if (const std::optional<std::string> c = literal_attribute(node, "Color"); c.has_value())
                    {
                        color = convert_color(*c);
                    }
                }
                catch (const xaml_convert_error& error)
                {
                    throw xaml_parse_exception(error.what(), node.line_number(), node.line_position());
                }
                if (const std::optional<std::string> a = literal_attribute(node, "FontAutoScalingEnabled");
                    a.has_value())
                {
                    auto_scaling = (*a == "true" || *a == "True");
                }
                auto font = maui::core::font::of_size(family, size).with_auto_scaling(auto_scaling);
                context_->set_value(node,
                                    std::any{std::shared_ptr<maui::core::i_image_source>(
                                        std::make_shared<maui::controls::font_image_source>(glyph, font, color))});
                context_->set_type(node, maui::core::type_tag::of<maui::controls::font_image_source>());
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
        // W13 — an <x:Array>'s own attributes (Type="{x:Type …}") are consumed/ignored at create time;
        // the value is a non-bindable xaml_array, so the generic apply must skip them (else it throws
        // "Cannot assign property Type"), the same create-time-consumed pattern as <RowDefinition> Height.
        if (source != nullptr && std::any_cast<xaml_array>(source) != nullptr)
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
            // W3: a <Style>'s own attributes (TargetType/BasedOn/Class/ApplyToDerivedTypes) are consumed at
            // CREATE time by build_style, and a <Setter>'s (Property/Value) by apply_setter_to_parent_style;
            // neither is a runtime bindable property, so the generic apply must skip them (else it throws
            // "Cannot assign property TargetType").
            // W2: likewise a <RowDefinition>/<ColumnDefinition>'s Height/Width is consumed at CREATE time
            // (the minted value type is not a bindable_object, so the generic apply would throw
            // "Cannot assign property Height"). W9: a <RoundRectangle>'s CornerRadius is consumed at CREATE
            // time too (minted as a non-bindable graphics::i_shape). W17: a <FontImageSource>'s
            // Glyph/FontFamily/Size/Color are consumed at CREATE time (minted as a non-bindable i_image_source).
            if (parent_element->type().is_of_any_type(
                    {"Style", "Setter", "RowDefinition", "ColumnDefinition", "RoundRectangle", "FontImageSource"}))
            {
                return;
            }
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

                // W13 — an <x:Array> parent: its item children were already gathered into the xaml_array
                // at create time (create_values), so the apply pass leaves them alone (the create-time-
                // consumed pattern, like a <RowDefinition>'s Height).
                if (source != nullptr && std::any_cast<xaml_array>(source) != nullptr)
                {
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

            // W2 — element-form Grid.RowDefinitions / Grid.ColumnDefinitions with several items: each
            // row_definition/column_definition VALUE is pushed onto the grid's vectors (the multi-child
            // twin of the apply_value_core single-child path) before the bindable child sink, which
            // cannot accept a non-bindable value type.
            if (try_add_grid_definition(*target, list_name, value))
            {
                return;
            }

            // W12 — element-form <Picker.Items> with several <x:String> children: each string VALUE is
            // pushed onto the picker's Items face (the multi-child twin of the apply_value_core path).
            if (try_add_picker_item(*target, list_name, value))
            {
                return;
            }

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

    // W3: resolve a <Setter Property=… Value=…/> against its enclosing <Style>'s TargetType and add the
    // built setter to that Style's minted shell. The reflection-free stand-in for C#'s
    // BindablePropertyConverter walking IProvideParentValues.ParentObjects: walk the node's parent chain to
    // the <Style> element explicitly, read the resolved shell + target type the create pass stored on it,
    // then build + add the setter. A <Setter> outside a <Style> is inert (matches C#).
    void apply_properties_visitor::apply_setter_to_parent_style(element_node& node, i_xaml_node* parent_node)
    {
        guarded(*context_, [this, &node, parent_node] {
            element_node* style_node = nullptr;
            for (i_xaml_node* ancestor = parent_node; ancestor != nullptr; ancestor = ancestor->parent())
            {
                auto* element = dynamic_cast<element_node*>(ancestor);
                if (element != nullptr && element->type().is_of_any_type({"Style"}))
                {
                    style_node = element;
                    break;
                }
            }
            if (style_node == nullptr)
            {
                return; // a stray <Setter> outside a <Style> — inert, as in C#
            }
            const std::shared_ptr<maui::controls::style> style = as_style(context_->try_get_value(*style_node));
            const maui::core::type_tag* target_type = context_->try_get_type(*style_node);
            if (style == nullptr || target_type == nullptr)
            {
                return; // the Style shell failed to mint (already reported on the error channel)
            }
            const std::optional<std::string> property = literal_attribute(node, "Property");
            const std::optional<std::string> value = literal_attribute(node, "Value");
            if (!property.has_value() || !value.has_value())
            {
                throw xaml_parse_exception("A <Setter> requires both a Property and a Value attribute",
                                           node.line_number(), node.line_position());
            }
            const applier_env env = env_from(*context_);
            style->add(build_setter(*target_type, *property, *value, *env.properties, *env.converters,
                                    node.line_number(), node.line_position()));
        });
    }

    void apply_properties_visitor::set_template(element_node& body_node, i_xaml_node* parent_node)
    {
        guarded(*context_, [this, &body_node, parent_node] {
            // The body's parent is the <DataTemplate>/<ControlTemplate> element node (the parser stored
            // the body as the parent's _CreateContent property), whose created value is the minted
            // element_template (a data_template for <DataTemplate>). Resolve it; a non-template parent
            // (or one that failed to mint) leaves the hook inert.
            auto* template_node = dynamic_cast<element_node*>(parent_node);
            if (template_node == nullptr)
            {
                return;
            }
            const std::any* template_value = context_->try_get_value(*template_node);
            if (template_value == nullptr)
            {
                return;
            }
            // SetTemplate (ApplyPropertiesVisitor.SetTemplate): install the per-stamp loader. The closure
            // OWNS a clone of the body subtree (the captured master, kept pristine — inflate clones it
            // AGAIN per stamp) and a VALUE snapshot of the load environment (NOT the live parent
            // hydration_context — a CollectionView stamps items / a templated control stamps its tree
            // after the load returns). Each loader() call inflates one fresh subtree from the master.
            // (PROFILE §8 ownership note: the captured clone lives in the move_only_function, which lives on
            // the template, which the consuming control owns — torn down with the loaded tree.) W16 — the
            // SAME loader serves both <DataTemplate> (data_template) and <ControlTemplate> (control_template),
            // both element_templates with set_load_template; pick whichever the parent minted.
            auto make_loader = [&body_node, this] {
                return [captured = body_node.clone(), env = template_inflater::from(*context_)]() mutable
                           -> std::shared_ptr<maui::core::bindable_object> {
                    return inflate_template_body(captured, env);
                };
            };
            if (const auto* data_tmpl = std::any_cast<std::shared_ptr<maui::controls::data_template>>(template_value);
                data_tmpl != nullptr && *data_tmpl != nullptr)
            {
                (*data_tmpl)->set_load_template(make_loader());
                // Record the body ROOT's registered control type so a native cell can realize this
                // template's inflated content (create_handler + host the native view), exactly like a
                // type-activated of<TControl>() template. Without this, a XAML-authored ItemTemplate
                // rendered only the item-text fallback in native cells (data_template.hpp
                // set_content_type). The registry miss (an unregistered root) keeps the old fallback.
                const std::string& root_name = body_node.type().name();
                const std::string& root_namespace = body_node.type().namespace_uri();
                if (root_namespace == maui_uri || root_namespace == maui_global_uri)
                {
                    if (const xaml_type_registry::registration* registration =
                            context_->type_registry().find(root_name, xaml_namespace::maui);
                        registration != nullptr)
                    {
                        (*data_tmpl)->set_content_type(registration->type);
                    }
                }
                return;
            }
            if (const auto* ctrl_tmpl =
                    std::any_cast<std::shared_ptr<maui::controls::control_template>>(template_value);
                ctrl_tmpl != nullptr && *ctrl_tmpl != nullptr)
            {
                (*ctrl_tmpl)->set_load_template(make_loader());
                return;
            }
            // a non-template parent / mint failure — deferred / inert.
        });
    }

    void apply_properties_visitor::visit(element_node& node, i_xaml_node* parent_node)
    {
        // W4 — _CreateContent values are <DataTemplate>/<ControlTemplate> bodies (the parser promotes a
        // template's single child to the _CreateContent property). The apply pass STOPS at the body (its
        // own children are not walked — stop_on_data_template) but still VISITS the body node, so this is
        // the SetTemplate hook: install the parent template's loader so each item lazily inflates a fresh
        // copy of this body. The reflection-free port of ApplyPropertiesVisitor.SetTemplate.
        const std::optional<xml_name> direct_name = try_get_property_name(node, parent_node);
        if (direct_name.has_value() && *direct_name == xml_name::create_content())
        {
            set_template(node, parent_node);
            return;
        }

        // W3 — a <Setter> collection item of a <Style>: resolve its Property/Value against the PARENT
        // Style's TargetType (the reflection-free stand-in for C#'s IProvideParentValues.ParentObjects
        // walk in BindablePropertyConverter — the port walks the node's parent chain to the enclosing
        // <Style> explicitly) and add the built setter to that Style's minted shell. Handled here, before
        // the generic collection routing, because a Setter has no created value (visit_collection_item
        // would otherwise fail "Cannot set the content of Style").
        if (node.type().is_of_any_type({"Setter"}))
        {
            apply_setter_to_parent_style(node, parent_node);
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
