// Tests for the M7 wave-2 visitor pipeline (maui::xaml xaml_visitors.hpp) — each case drives the
// visitors directly over a parsed node tree, in XamlLoader.Visit's order, the way the C# loader does
// before the public Load/Create wrappers (xaml_loader tests cover those e2e). Behavior derived from
// src/Controls/src/Xaml/{ExpandMarkupsVisitor,NamescopingVisitor,CreateValuesVisitor,
// RegisterXNamesVisitor,FillResourceDictionariesVisitor,ApplyPropertiesVisitor}.cs and the C# tests:
// LoaderTests.cs (property setting, content/collection routing, resources, errors), NameScopeTests.cs
// (scope sharing + Style/DataTemplate isolation), MarkupExtensionTests.cs (expansion + unknown
// extension), XStatic/OnPlatform/OnAppTheme fixtures (the provide_value appliers).
#include "maui/xaml/xaml_visitors.hpp"

#include <any>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/application.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/resource_dictionary.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/app_theme.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/type_tag.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/xaml/hydration_context.hpp"
#include "maui/xaml/i_markup_extension.hpp"
#include "maui/xaml/markup_extensions.hpp"
#include "maui/xaml/name_scope.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_node.hpp"
#include "maui/xaml/xaml_parse_exception.hpp"
#include "maui/xaml/xaml_parser.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_runtime_environment.hpp"
#include "maui/xaml/xaml_standard_types.hpp"
#include "maui/xaml/xaml_type_registry.hpp"
#include <gtest/gtest.h>

namespace
{
    namespace controls = maui::controls;
    using maui::core::app_theme;
    using maui::core::bindable_object;
    using maui::core::type_tag;
    using maui::xaml::apply_properties_visitor;
    using maui::xaml::create_values_visitor;
    using maui::xaml::element_node;
    using maui::xaml::expand_markups_visitor;
    using maui::xaml::fill_resource_dictionaries_visitor;
    using maui::xaml::hydration_context;
    using maui::xaml::i_markup_extension;
    using maui::xaml::i_xaml_node;
    using maui::xaml::namescoping_visitor;
    using maui::xaml::prune_ignored_nodes_visitor;
    using maui::xaml::register_x_names_visitor;
    using maui::xaml::root_node;
    using maui::xaml::xaml_node_visitor;
    using maui::xaml::xaml_parse_exception;
    using maui::xaml::xaml_parser;
    using maui::xaml::xml_name;

    // The standard registries, populated once (the loader's register_standard_* setup).
    void ensure_standard_registrations()
    {
        static const bool registered = [] {
            maui::xaml::register_standard_xaml(maui::xaml::default_xaml_type_registry(),
                                               maui::xaml::default_xaml_property_registry(),
                                               maui::xaml::default_xaml_converter_registry());
            maui::xaml::register_standard_markup_extensions();
            return true;
        }();
        (void)registered;
    }

    [[nodiscard]] hydration_context make_context(hydration_context::exception_handler handler = nullptr)
    {
        ensure_standard_registrations();
        return hydration_context{maui::xaml::default_xaml_type_registry(), maui::xaml::default_xaml_property_registry(),
                                 maui::xaml::default_xaml_converter_registry(),
                                 maui::xaml::markup_extension_registry::instance(), std::move(handler)};
    }

    // XamlLoader.Visit's visitor sequence (the parent-setting adapter + the seven concrete passes).
    void run_pipeline(root_node& root, hydration_context& context)
    {
        xaml_node_visitor set_parents{[](i_xaml_node& node, i_xaml_node* parent) { node.set_parent(parent); }};
        root.accept(set_parents, nullptr);
        expand_markups_visitor expand{context};
        root.accept(expand, nullptr);
        prune_ignored_nodes_visitor prune;
        root.accept(prune, nullptr);
        namescoping_visitor namescope{context};
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

    // XamlLoader.Load(view, xaml)'s core: hydrate `xaml` into the caller-owned root object.
    std::shared_ptr<root_node> load_into(hydration_context& context, bindable_object& root_object,
                                         std::string_view xaml)
    {
        const std::shared_ptr<root_node> root = xaml_parser::parse(xaml);
        context.set_root_element(&root_object);
        run_pipeline(*root, context);
        return root;
    }

    // The root namescope the pipeline produced (names maui::xaml::name_scope's typed lookup).
    [[nodiscard]] maui::xaml::name_scope& root_scope(const root_node& root)
    {
        return *root.scope_ref()->scope;
    }

    [[nodiscard]] std::string parse_error_message(const std::function<void()>& action)
    {
        try
        {
            action();
        }
        catch (const xaml_parse_exception& exception)
        {
            return exception.unformatted_message();
        }
        return "(no xaml_parse_exception thrown)";
    }

    // ---- expand_markups_visitor (ExpandMarkupsVisitor + MarkupExpansionParser) --------------------

    TEST(expand_markups_visitor, property_markup_is_minted_as_an_extension)
    {
        auto context = make_context();
        const auto root = xaml_parser::parse(R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui" Text="{StaticResource foo}"/>)xml");
        xaml_node_visitor set_parents{[](i_xaml_node& node, i_xaml_node* parent) { node.set_parent(parent); }};
        root->accept(set_parents, nullptr);
        expand_markups_visitor expand{context};
        root->accept(expand, nullptr);

        const xml_name text_name{.namespace_uri = "", .local_name = "Text"};
        const std::shared_ptr<i_xaml_node> markup = root->properties().try_get(text_name);
        ASSERT_NE(markup, nullptr);
        const std::any* expansion = context.try_get_value(*markup);
        ASSERT_NE(expansion, nullptr);
        EXPECT_NE(std::any_cast<std::shared_ptr<i_markup_extension>>(expansion), nullptr);
    }

    TEST(expand_markups_visitor, unknown_extension_throws_markup_extension_not_found)
    {
        // MarkupExtensionTests.ThrowOnMarkupExtensionNotFound — raised at expand time, like C#'s
        // MarkupExpansionParser.Parse.
        auto context = make_context();
        const auto root = xaml_parser::parse(R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui" Text="{local:Missing}"/>)xml");
        xaml_node_visitor set_parents{[](i_xaml_node& node, i_xaml_node* parent) { node.set_parent(parent); }};
        root->accept(set_parents, nullptr);
        expand_markups_visitor expand{context};
        EXPECT_EQ(parse_error_message([&] { root->accept(expand, nullptr); }),
                  "MarkupExtension not found for local:Missing");
    }

    TEST(expand_markups_visitor, x_namespace_extension_resolves_through_its_prefix)
    {
        auto context = make_context();
        const auto root = xaml_parser::parse(R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
       xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml" Text="{x:Null}"/>)xml");
        xaml_node_visitor set_parents{[](i_xaml_node& node, i_xaml_node* parent) { node.set_parent(parent); }};
        root->accept(set_parents, nullptr);
        expand_markups_visitor expand{context};
        root->accept(expand, nullptr);

        const xml_name text_name{.namespace_uri = "", .local_name = "Text"};
        const std::shared_ptr<i_xaml_node> markup = root->properties().try_get(text_name);
        const std::any* expansion = context.try_get_value(*markup);
        ASSERT_NE(expansion, nullptr);
        const auto* extension = std::any_cast<std::shared_ptr<i_markup_extension>>(expansion);
        ASSERT_NE(extension, nullptr);
        const std::any provided = (*extension)->provide_value({});
        EXPECT_NE(std::any_cast<maui::xaml::xaml_null>(&provided), nullptr);
    }

    // ---- namescoping_visitor (NamescopingVisitor; NameScopeTests.cs scope-sharing shapes) ----------

    TEST(namescoping_visitor, descendants_share_the_root_scope_ref)
    {
        auto context = make_context();
        const auto root = xaml_parser::parse(R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<VerticalStackLayout>
		<Label Text="Foo"/>
	</VerticalStackLayout>
</ContentPage>)xml");
        xaml_node_visitor set_parents{[](i_xaml_node& node, i_xaml_node* parent) { node.set_parent(parent); }};
        root->accept(set_parents, nullptr);
        namescoping_visitor namescope{context};
        root->accept(namescope, nullptr);

        ASSERT_NE(root->scope_ref(), nullptr);
        const auto stack = std::dynamic_pointer_cast<element_node>(root->collection_items().front());
        ASSERT_NE(stack, nullptr);
        const auto label = std::dynamic_pointer_cast<element_node>(stack->collection_items().front());
        ASSERT_NE(label, nullptr);
        // One shared ref OBJECT (the NameScopeRef indirection), not merely equal scopes.
        EXPECT_EQ(stack->scope_ref(), root->scope_ref());
        EXPECT_EQ(label->scope_ref(), root->scope_ref());
        EXPECT_NE(root->scope_ref()->scope, nullptr);
    }

    TEST(namescoping_visitor, style_content_starts_its_own_scope)
    {
        // NamescopingVisitor.IsStyle: an element under a <Style> parent gets a fresh scope.
        auto context = make_context();
        const auto root = xaml_parser::parse(R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<Label.Style>
		<Style>
			<Setter/>
		</Style>
	</Label.Style>
</Label>)xml");
        xaml_node_visitor set_parents{[](i_xaml_node& node, i_xaml_node* parent) { node.set_parent(parent); }};
        root->accept(set_parents, nullptr);
        namescoping_visitor namescope{context};
        root->accept(namescope, nullptr);

        // Property elements are stored under (element xmlns, name-after-the-dot) — parser_tests.
        const xml_name style_name{.namespace_uri = std::string{maui::xaml::maui_uri}, .local_name = "Style"};
        const auto style = std::dynamic_pointer_cast<element_node>(root->properties().try_get(style_name));
        ASSERT_NE(style, nullptr);
        EXPECT_EQ(style->scope_ref(), root->scope_ref()); // the <Style> element itself shares
        const auto setter = std::dynamic_pointer_cast<element_node>(style->collection_items().front());
        ASSERT_NE(setter, nullptr);
        EXPECT_NE(setter->scope_ref(), root->scope_ref()); // its content does not
    }

    TEST(namescoping_visitor, data_template_content_starts_its_own_scope)
    {
        auto context = make_context();
        const auto root = xaml_parser::parse(R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<DataTemplate>
		<Label/>
	</DataTemplate>
</ContentPage>)xml");
        xaml_node_visitor set_parents{[](i_xaml_node& node, i_xaml_node* parent) { node.set_parent(parent); }};
        root->accept(set_parents, nullptr);
        namescoping_visitor namescope{context};
        root->accept(namescope, nullptr);

        const auto data_template = std::dynamic_pointer_cast<element_node>(root->collection_items().front());
        ASSERT_NE(data_template, nullptr);
        const auto content =
            std::dynamic_pointer_cast<element_node>(data_template->properties().try_get(xml_name::create_content()));
        ASSERT_NE(content, nullptr);
        EXPECT_EQ(data_template->scope_ref(), root->scope_ref());
        EXPECT_NE(content->scope_ref(), root->scope_ref()); // the _CreateContent value is isolated
    }

    // ---- create_values_visitor (CreateValuesVisitor) -----------------------------------------------

    TEST(create_values_visitor, elements_are_created_through_the_type_registry_and_owned_by_the_graph)
    {
        auto context = make_context();
        controls::content_page page;
        const auto root = load_into(context, page, R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<VerticalStackLayout>
		<Label Text="Foo"/>
	</VerticalStackLayout>
</ContentPage>)xml");

        const auto stack = std::dynamic_pointer_cast<element_node>(root->collection_items().front());
        const std::any* stack_value = context.try_get_value(*stack);
        ASSERT_NE(stack_value, nullptr);
        const auto* stack_object = std::any_cast<std::shared_ptr<bindable_object>>(stack_value);
        ASSERT_NE(stack_object, nullptr);
        EXPECT_NE(std::dynamic_pointer_cast<controls::vertical_stack_layout>(*stack_object), nullptr);
        const type_tag* stack_type = context.try_get_type(*stack);
        ASSERT_NE(stack_type, nullptr);
        EXPECT_EQ(*stack_type, type_tag::of<controls::vertical_stack_layout>());
        EXPECT_EQ(context.graph().size(), 2U); // the stack + the label; the root is caller-owned
    }

    TEST(create_values_visitor, the_root_node_takes_the_context_root_element)
    {
        auto context = make_context();
        controls::label label;
        const auto root =
            load_into(context, label, R"xml(<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui"/>)xml");
        const std::any* value = context.try_get_value(*root);
        ASSERT_NE(value, nullptr);
        const auto* object = std::any_cast<std::shared_ptr<bindable_object>>(value);
        ASSERT_NE(object, nullptr);
        EXPECT_EQ(object->get(), &label);
        const type_tag* type = context.try_get_type(*root);
        ASSERT_NE(type, nullptr);
        EXPECT_EQ(*type, type_tag::of<controls::label>());
    }

    TEST(create_values_visitor, unknown_type_throws)
    {
        // LoaderTests.TestUnknownType.
        auto context = make_context();
        controls::vertical_stack_layout stack;
        EXPECT_EQ(parse_error_message([&] {
                      (void)load_into(context, stack, R"xml(
<VerticalStackLayout xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<CustomView/>
</VerticalStackLayout>)xml");
                  }),
                  "Type CustomView not found in xmlns http://schemas.microsoft.com/dotnet/2021/maui");
    }

    TEST(create_values_visitor, x2009_language_primitives_are_parsed)
    {
        auto context = make_context();
        controls::content_page page;
        const auto root = load_into(context, page, R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml">
	<ContentPage.Resources>
		<ResourceDictionary>
			<x:String x:Key="s">hello</x:String>
			<x:Int32 x:Key="i">42</x:Int32>
			<x:Double x:Key="d">1.5</x:Double>
			<x:Boolean x:Key="b">true</x:Boolean>
			<x:Int32 x:Key="malformed">not-a-number</x:Int32>
		</ResourceDictionary>
	</ContentPage.Resources>
</ContentPage>)xml");
        (void)root;
        ASSERT_TRUE(page.is_resources_created());
        EXPECT_EQ(std::any_cast<std::string>(*page.resources().try_get("s")), "hello");
        EXPECT_EQ(std::any_cast<int>(*page.resources().try_get("i")), 42);
        EXPECT_EQ(std::any_cast<double>(*page.resources().try_get("d")), 1.5);
        EXPECT_EQ(std::any_cast<bool>(*page.resources().try_get("b")), true);
        // CreateLanguagePrimitive: a TryParse failure keeps the default value, silently.
        EXPECT_EQ(std::any_cast<int>(*page.resources().try_get("malformed")), 0);
    }

    TEST(create_values_visitor, x_arguments_and_x_factory_method_fail_loudly)
    {
        auto context = make_context();
        controls::content_page page;
        EXPECT_EQ(parse_error_message([&] {
                      (void)load_into(context, page, R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml">
	<Label>
		<x:Arguments><x:String>boom</x:String></x:Arguments>
	</Label>
</ContentPage>)xml");
                  }),
                  "x:Arguments and x:FactoryMethod are not supported by the port yet (STATUS.md M7 deferrals)");
    }

    TEST(create_values_visitor, exception_handler_collects_and_continues)
    {
        // HydrationContext.ExceptionHandler (doNotThrow): the failed node is skipped, the rest loads.
        std::vector<std::string> collected;
        auto context = make_context(
            [&collected](const xaml_parse_exception& e) { collected.emplace_back(e.unformatted_message()); });
        controls::vertical_stack_layout stack;
        (void)load_into(context, stack, R"xml(
<VerticalStackLayout xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<CustomView/>
	<Label Text="Foo"/>
</VerticalStackLayout>)xml");
        ASSERT_EQ(collected.size(), 1U);
        EXPECT_EQ(collected.front(),
                  "Type CustomView not found in xmlns http://schemas.microsoft.com/dotnet/2021/maui");
        ASSERT_EQ(stack.count(), 1); // the label still landed
        EXPECT_EQ(dynamic_cast<controls::label&>(stack.at(0)).text(), "Foo");
    }

    // ---- register_x_names_visitor (RegisterXNamesVisitor; NameScopeTests / LoaderTests names) ------

    TEST(register_x_names_visitor, x_name_registers_the_hydrated_object)
    {
        // LoaderTests.TestFindByXName.
        auto context = make_context();
        controls::vertical_stack_layout stack;
        const auto root = load_into(context, stack, R"xml(
<VerticalStackLayout xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml">
	<Label x:Name="label0" Text="Foo"/>
</VerticalStackLayout>)xml");
        ASSERT_NE(root->scope_ref(), nullptr);
        const std::shared_ptr<controls::label> label = root_scope(*root).find_by_name_as<controls::label>("label0");
        ASSERT_NE(label, nullptr);
        EXPECT_EQ(label->text(), "Foo");
    }

    TEST(register_x_names_visitor, duplicate_x_name_throws)
    {
        // NameScopeTests: "An element with the name ... already exists in this NameScope".
        auto context = make_context();
        controls::vertical_stack_layout stack;
        EXPECT_EQ(parse_error_message([&] {
                      (void)load_into(context, stack, R"xml(
<VerticalStackLayout xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml">
	<Label x:Name="dup"/>
	<Label x:Name="dup"/>
</VerticalStackLayout>)xml");
                  }),
                  "An element with the name \"dup\" already exists in this NameScope");
    }

    // ---- apply_properties_visitor: attribute + content routing (ApplyPropertiesVisitor) ------------

    TEST(apply_properties_visitor, sets_a_bindable_property_from_text)
    {
        // LoaderTests.TestSetValueToBindableProperty.
        auto context = make_context();
        controls::label label;
        (void)load_into(context, label,
                        R"xml(<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui" Text="Foo"/>)xml");
        EXPECT_EQ(label.text(), "Foo");
    }

    TEST(apply_properties_visitor, converts_text_against_the_property_value_type)
    {
        // LoaderTests.TestBoolValue.
        auto context = make_context();
        controls::image image;
        EXPECT_FALSE(image.is_opaque());
        (void)load_into(context, image,
                        R"xml(<Image xmlns="http://schemas.microsoft.com/dotnet/2021/maui" IsOpaque="true"/>)xml");
        EXPECT_TRUE(image.is_opaque());
    }

    TEST(apply_properties_visitor, unknown_property_throws_cannot_assign)
    {
        // LoaderTests.TestUnknownPropertyShouldThrow.
        auto context = make_context();
        controls::label label;
        EXPECT_EQ(parse_error_message([&] {
                      (void)load_into(context, label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui" Text="Foo" UnknownProperty="Bar"/>)xml");
                  }),
                  "Cannot assign property \"UnknownProperty\": Property does not exist, or is not assignable, or "
                  "mismatching type between value and property");
    }

    TEST(apply_properties_visitor, element_text_routes_to_the_content_property_value)
    {
        // Label carries [ContentProperty(nameof(Text))]: element text lands on Text.
        auto context = make_context();
        controls::label label;
        (void)load_into(context, label,
                        R"xml(<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui">Foo</Label>)xml");
        EXPECT_EQ(label.text(), "Foo");
    }

    TEST(apply_properties_visitor, collection_content_children_are_added_in_document_order)
    {
        // LoaderTests.TestCollectionContentProperties.
        auto context = make_context();
        controls::vertical_stack_layout stack;
        (void)load_into(context, stack, R"xml(
<VerticalStackLayout xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<Label Text="Foo"/>
	<Label Text="Bar"/>
</VerticalStackLayout>)xml");
        ASSERT_EQ(stack.count(), 2);
        EXPECT_EQ(dynamic_cast<controls::label&>(stack.at(0)).text(), "Foo");
        EXPECT_EQ(dynamic_cast<controls::label&>(stack.at(1)).text(), "Bar");
    }

    TEST(apply_properties_visitor, single_collection_content_child)
    {
        // LoaderTests.TestCollectionContentPropertiesWithSingleElement.
        auto context = make_context();
        controls::vertical_stack_layout stack;
        (void)load_into(context, stack, R"xml(
<VerticalStackLayout xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<Label Text="Foo"/>
</VerticalStackLayout>)xml");
        ASSERT_EQ(stack.count(), 1);
        EXPECT_EQ(dynamic_cast<controls::label&>(stack.at(0)).text(), "Foo");
    }

    TEST(apply_properties_visitor, children_property_element_routes_through_the_named_child_sink)
    {
        // LoaderTests.TestFindByXName's <StackLayout.Children> spelling (two children → ListNode).
        auto context = make_context();
        controls::vertical_stack_layout stack;
        (void)load_into(context, stack, R"xml(
<VerticalStackLayout xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<VerticalStackLayout.Children>
		<Label Text="Foo"/>
		<Label Text="Bar"/>
	</VerticalStackLayout.Children>
</VerticalStackLayout>)xml");
        ASSERT_EQ(stack.count(), 2);
        EXPECT_EQ(dynamic_cast<controls::label&>(stack.at(0)).text(), "Foo");
        EXPECT_EQ(dynamic_cast<controls::label&>(stack.at(1)).text(), "Bar");
    }

    TEST(apply_properties_visitor, single_child_property_element_simplifies_the_list_node)
    {
        // ApplyPropertiesVisitor's "Simplify ListNodes with single elements" route.
        auto context = make_context();
        controls::vertical_stack_layout stack;
        (void)load_into(context, stack, R"xml(
<VerticalStackLayout xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<VerticalStackLayout.Children>
		<Label Text="Foo"/>
	</VerticalStackLayout.Children>
</VerticalStackLayout>)xml");
        ASSERT_EQ(stack.count(), 1);
        EXPECT_EQ(dynamic_cast<controls::label&>(stack.at(0)).text(), "Foo");
    }

    TEST(apply_properties_visitor, content_page_content_child)
    {
        // LoaderTests.TestPropertiesWithContentProperties (the content half — the Grid.Row attached
        // property is an M7 deferral).
        auto context = make_context();
        controls::content_page page;
        (void)load_into(context, page, R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<Label Text="foo"/>
</ContentPage>)xml");
        ASSERT_NE(page.content(), nullptr);
        EXPECT_EQ(dynamic_cast<controls::label*>(page.content())->text(), "foo");
    }

    // ---- resources + {StaticResource}/{DynamicResource} (Fill pass + the applier contracts) --------

    TEST(fill_resource_dictionaries_visitor, resources_land_on_the_owning_element)
    {
        // LoaderTests.TestResources (with an x:String resource — the port's loadable v1 resource).
        auto context = make_context();
        controls::label label;
        (void)load_into(context, label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
       xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml">
	<Label.Resources>
		<ResourceDictionary>
			<x:String x:Key="greeting">hello</x:String>
		</ResourceDictionary>
	</Label.Resources>
</Label>)xml");
        ASSERT_TRUE(label.is_resources_created());
        const std::any* value = label.resources().try_get("greeting");
        ASSERT_NE(value, nullptr);
        EXPECT_EQ(std::any_cast<std::string>(*value), "hello");
    }

    TEST(fill_resource_dictionaries_visitor, implicit_resources_without_a_dictionary_element)
    {
        // The implicit form (<Label.Resources> directly holding keyed items) routes through
        // TryAddToProperty → the element's lazily-created dictionary.
        auto context = make_context();
        controls::label label;
        (void)load_into(context, label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
       xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml">
	<Label.Resources>
		<x:String x:Key="greeting">hello</x:String>
	</Label.Resources>
</Label>)xml");
        ASSERT_TRUE(label.is_resources_created());
        const std::any* value = label.resources().try_get("greeting");
        ASSERT_NE(value, nullptr);
        EXPECT_EQ(std::any_cast<std::string>(*value), "hello");
    }

    TEST(apply_properties_visitor, resource_without_x_key_throws)
    {
        // LoaderTests.TestResourceDoesRequireKey.
        auto context = make_context();
        controls::label label;
        EXPECT_EQ(parse_error_message([&] {
                      (void)load_into(context, label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
       xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml">
	<Label.Resources>
		<ResourceDictionary>
			<x:String>hello</x:String>
		</ResourceDictionary>
	</Label.Resources>
</Label>)xml");
                  }),
                  "resources in ResourceDictionary require a x:Key attribute");
    }

    TEST(apply_properties_visitor, static_resource_resolves_from_an_ancestor_dictionary)
    {
        // LoaderTests.UseResourcesOutsideOfBinding's shape, over the v1 set: the resource is consumed
        // by a SIBLING subtree before live parents exist (the load-time parent_resources walk).
        auto context = make_context();
        controls::content_page page;
        (void)load_into(context, page, R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml">
	<ContentPage.Resources>
		<ResourceDictionary>
			<x:String x:Key="bar">Foo</x:String>
		</ResourceDictionary>
	</ContentPage.Resources>
	<Label Text="{StaticResource bar}"/>
</ContentPage>)xml");
        ASSERT_NE(page.content(), nullptr);
        EXPECT_EQ(dynamic_cast<controls::label*>(page.content())->text(), "Foo");
    }

    TEST(apply_properties_visitor, missing_static_resource_throws)
    {
        // LoaderTests.MissingStaticResourceShouldThrow.
        auto context = make_context();
        controls::label label;
        EXPECT_EQ(parse_error_message([&] {
                      (void)load_into(context, label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui" Text="{StaticResource foo}"/>)xml");
                  }),
                  "StaticResource not found for key foo");
    }

    TEST(apply_properties_visitor, static_resource_falls_back_to_application_resources)
    {
        // LoaderTests.StaticResourceLookForApplicationResources: app-level fallback + local precedence.
        auto context = make_context();
        controls::application app;
        (void)app.resources().add("foo", std::any{std::string{"FOO"}});
        (void)app.resources().add("bar", std::any{std::string{"BAR"}});
        context.application = &app;
        controls::content_page page;
        const auto root = load_into(context, page, R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml">
	<ContentPage.Resources>
		<ResourceDictionary>
			<x:String x:Key="bar">BAZ</x:String>
		</ResourceDictionary>
	</ContentPage.Resources>
	<VerticalStackLayout>
		<Label x:Name="label0" Text="{StaticResource foo}"/>
		<Label x:Name="label1" Text="{StaticResource bar}"/>
	</VerticalStackLayout>
</ContentPage>)xml");
        const auto label0 = root_scope(*root).find_by_name_as<controls::label>("label0");
        const auto label1 = root_scope(*root).find_by_name_as<controls::label>("label1");
        ASSERT_NE(label0, nullptr);
        ASSERT_NE(label1, nullptr);
        EXPECT_EQ(label0->text(), "FOO"); // from app resources
        EXPECT_EQ(label1->text(), "BAZ"); // local resources have precedence
    }

    TEST(apply_properties_visitor, dynamic_resource_routes_to_set_dynamic_resource)
    {
        // The {DynamicResource} applier contract: the marker becomes set_dynamic_resource, so a later
        // resource change re-applies (DynamicResource.cs / TrySetDynamicResource).
        auto context = make_context();
        controls::content_page page;
        (void)page.resources().add("greeting", std::any{std::string{"hello"}});
        (void)load_into(context, page, R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<Label Text="{DynamicResource greeting}"/>
</ContentPage>)xml");
        auto* label = dynamic_cast<controls::label*>(page.content());
        ASSERT_NE(label, nullptr);
        EXPECT_EQ(label->text(), "hello");
        page.resources().set("greeting", std::any{std::string{"goodbye"}});
        EXPECT_EQ(label->text(), "goodbye");
    }

    // ---- the markup-extension appliers ({x:Static}/{OnPlatform}/{AppThemeBinding}/{Binding}/{x:Null}) ----

    TEST(apply_properties_visitor, x_static_applies_the_member_value_typed)
    {
        auto context = make_context();
        controls::label label;
        (void)load_into(context, label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
       xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml" TextColor="{x:Static Colors.Red}"/>)xml");
        EXPECT_EQ(label.text_color(), maui::graphics::colors::red);
    }

    TEST(apply_properties_visitor, on_platform_picks_the_runtime_platform_and_converts_late)
    {
        const maui::xaml::xaml_runtime_environment saved = maui::xaml::xaml_runtime_environment::current();
        maui::xaml::xaml_runtime_environment::set_current(
            {.platform = maui::xaml::device_platform::ios, .idiom = maui::xaml::device_idiom::phone});
        auto context = make_context();
        controls::label label;
        (void)load_into(context, label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
       Opacity="{OnPlatform iOS=0.5, Default=0.8}"/>)xml");
        EXPECT_EQ(label.opacity(), 0.5);
        maui::xaml::xaml_runtime_environment::set_current(saved);
    }

    TEST(apply_properties_visitor, on_platform_with_no_matching_value_skips_the_assignment)
    {
        const maui::xaml::xaml_runtime_environment saved = maui::xaml::xaml_runtime_environment::current();
        maui::xaml::xaml_runtime_environment::set_current(
            {.platform = maui::xaml::device_platform::android, .idiom = maui::xaml::device_idiom::phone});
        auto context = make_context();
        controls::label label;
        const double default_opacity = label.opacity();
        (void)load_into(context, label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui" Opacity="{OnPlatform iOS=0.5}"/>)xml");
        EXPECT_EQ(label.opacity(), default_opacity); // the documented skip (markup_extensions.hpp)
        maui::xaml::xaml_runtime_environment::set_current(saved);
    }

    TEST(apply_properties_visitor, app_theme_binding_applies_and_reapplies_on_theme_change)
    {
        // AppThemeBinding.Apply/ApplyCore: the picked slot now, re-applied on RequestedThemeChanged.
        // The app outlives the context: the context's theme subscriptions are non-owning borrows
        // that must disconnect while the application's event is still alive (the same teardown
        // order xaml_load_result pins by member order) — ASan-verified.
        controls::application app;
        auto context = make_context();
        context.application = &app;
        controls::label label;
        (void)load_into(context, label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
       Text="{AppThemeBinding Light=day, Dark=night}"/>)xml");
        EXPECT_EQ(label.text(), "day"); // unspecified theme defaults to the light slot
        app.set_user_app_theme(app_theme::dark);
        EXPECT_EQ(label.text(), "night");
        // The re-apply subscription is owned by the load (the context's accumulator).
        EXPECT_EQ(context.subscriptions().size(), 1U);
    }

    TEST(apply_properties_visitor, binding_markup_fails_loudly_without_a_binding_applier)
    {
        // The {Binding} applier contract: until register_runtime_bindings() installs the real
        // applier (loader_tests cover that), the DEFAULT one REJECTS with a clear error instead of
        // dropping the binding.
        auto context = make_context();
        controls::label label;
        const std::string message = parse_error_message([&] {
            (void)load_into(context, label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui" Text="{Binding Path=labeltext}"/>)xml");
        });
        EXPECT_TRUE(message.contains("no binding applier is registered")) << message;
    }

    TEST(apply_properties_visitor, x_null_fails_loudly)
    {
        // The {x:Null} value-form contract: no typed-null channel yet — a reported load failure.
        auto context = make_context();
        controls::label label;
        const std::string message = parse_error_message([&] {
            (void)load_into(context, label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
       xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml" Text="{x:Null}"/>)xml");
        });
        EXPECT_TRUE(message.contains("{x:Null}")) << message;
    }

    // ---- prune_ignored_nodes_visitor (PruneIgnoredNodesVisitor; mc:Ignorable) ----------------------

    TEST(prune_ignored_nodes_visitor, ignorable_prefixed_attributes_are_dropped)
    {
        auto context = make_context();
        controls::label label;
        (void)load_into(context, label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
       xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006"
       xmlns:d="http://schemas.microsoft.com/dotnet/2021/maui/design"
       mc:Ignorable="d"
       d:Text="design-only" Text="runtime"/>)xml");
        EXPECT_EQ(label.text(), "runtime"); // the d:Text attribute (AND mc:Ignorable) never applied
    }
} // namespace
