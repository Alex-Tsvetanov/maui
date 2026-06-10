// Tests for maui::xaml::xaml_parser + the xaml_node tree (M7 wave 1).
//
// The C# Xaml.UnitTests exercise the parser through hydration (wave 2 here), so these cases assert
// the PARSE-TREE SHAPES those tests rely on, derived from XamlParser.cs/XamlNode.cs and the
// fixtures' markup: FindByName.xaml (x:Name/x:Class + nesting), StringLiterals.xaml ("{}" escapes),
// DataTemplate.xaml (_CreateContent + "{Binding}" markup detection), GenericsTests.xaml
// (x:TypeArguments incl. nested generics), plus the visitor-ordering knobs of XamlNodeVisitor.cs
// and the MarkupExpressionParser.cs tokenization primitives.
#include "maui/xaml/xaml_parser.hpp"

#include "maui/xaml/xaml_parse_exception.hpp"

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/xaml/xaml_node.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::xaml::element_node;
    using maui::xaml::i_xaml_node;
    using maui::xaml::list_node;
    using maui::xaml::markup_node;
    using maui::xaml::maui_uri;
    using maui::xaml::parse_options;
    using maui::xaml::root_node;
    using maui::xaml::tree_visiting_mode;
    using maui::xaml::value_node;
    using maui::xaml::x2009_uri;
    using maui::xaml::xaml_node_visitor;
    using maui::xaml::xaml_parse_exception;
    using maui::xaml::xaml_parser;
    using maui::xaml::xml_name;
    using maui::xaml::xml_namespace_resolver;
    using maui::xaml::xml_type;

    [[nodiscard]] xml_name name_of(std::string_view namespace_uri, std::string_view local_name)
    {
        return xml_name{.namespace_uri = std::string(namespace_uri), .local_name = std::string(local_name)};
    }

    template <class TNode> [[nodiscard]] std::shared_ptr<TNode> as(const std::shared_ptr<i_xaml_node>& node)
    {
        return std::dynamic_pointer_cast<TNode>(node);
    }

    // The FindByName.xaml shape.
    constexpr std::string_view find_by_name_xaml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
<ContentPage
	xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
	xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml"
	x:Class="Microsoft.Maui.Controls.Xaml.UnitTests.FindByName"
	x:Name="root">
	<StackLayout>
		<Label Text="Foo" x:Name="label0"/>
	</StackLayout>
</ContentPage>)xml";

    // ---- root element: type + namespace (XamlLoader root-node construction) ----

    TEST(xaml_parser, root_element_type_and_namespace)
    {
        const auto root = xaml_parser::parse(find_by_name_xaml);
        ASSERT_NE(root, nullptr);
        EXPECT_EQ(root->type().name(), "ContentPage");
        EXPECT_EQ(root->type().namespace_uri(), maui_uri);
        EXPECT_EQ(root->namespace_uri(), maui_uri);
    }

    TEST(xaml_parser, root_keeps_the_qualified_name_inner_elements_use_the_local_name)
    {
        // XamlLoader: new XmlType(reader.NamespaceURI, reader.Name, …) — the root's Name is the
        // QUALIFIED reader.Name; inner elements (ReadNode) use reader.LocalName.
        const auto root = xaml_parser::parse(R"xml(
<m:ContentPage xmlns:m="http://schemas.microsoft.com/dotnet/2021/maui">
	<m:Label/>
</m:ContentPage>)xml");
        EXPECT_EQ(root->type().name(), "m:ContentPage");
        EXPECT_EQ(root->type().namespace_uri(), maui_uri);
        ASSERT_EQ(root->collection_items().size(), 1U);
        const auto label = as<element_node>(root->collection_items().front());
        ASSERT_NE(label, nullptr);
        EXPECT_EQ(label->type().name(), "Label");
        EXPECT_EQ(label->type().namespace_uri(), maui_uri);
    }

    // ---- nesting: children land in collection_items ----

    TEST(xaml_parser, nested_children_go_into_collection_items)
    {
        const auto root = xaml_parser::parse(find_by_name_xaml);
        ASSERT_EQ(root->collection_items().size(), 1U);

        const auto stack = as<element_node>(root->collection_items().front());
        ASSERT_NE(stack, nullptr);
        EXPECT_EQ(stack->type().name(), "StackLayout");
        EXPECT_EQ(stack->type().namespace_uri(), maui_uri);

        ASSERT_EQ(stack->collection_items().size(), 1U);
        const auto label = as<element_node>(stack->collection_items().front());
        ASSERT_NE(label, nullptr);
        EXPECT_EQ(label->type().name(), "Label");
    }

    // ---- attributes: plain values, x:Name / x:Class directives ----

    TEST(xaml_parser, attribute_becomes_a_value_node_property)
    {
        const auto root = xaml_parser::parse(find_by_name_xaml);
        const auto stack = as<element_node>(root->collection_items().front());
        const auto label = as<element_node>(stack->collection_items().front());

        // Unprefixed, undotted attributes carry NO namespace (XmlReader semantics).
        const auto text = as<value_node>(label->properties().try_get(name_of("", "Text")));
        ASSERT_NE(text, nullptr);
        EXPECT_EQ(text->value(), "Foo");
        EXPECT_FALSE(text->is_escaped());
    }

    TEST(xaml_parser, x_name_and_x_class_are_recognized_as_directives)
    {
        const auto root = xaml_parser::parse(find_by_name_xaml);

        const auto klass = as<value_node>(root->properties().try_get(xml_name::x_class()));
        ASSERT_NE(klass, nullptr);
        EXPECT_EQ(klass->value(), "Microsoft.Maui.Controls.Xaml.UnitTests.FindByName");

        const auto root_name = as<value_node>(root->properties().try_get(xml_name::x_name()));
        ASSERT_NE(root_name, nullptr);
        EXPECT_EQ(root_name->value(), "root");

        const auto stack = as<element_node>(root->collection_items().front());
        const auto label = as<element_node>(stack->collection_items().front());
        const auto label_name = as<value_node>(label->properties().try_get(xml_name::x_name()));
        ASSERT_NE(label_name, nullptr);
        EXPECT_EQ(label_name->value(), "label0");
    }

    TEST(xaml_parser, x_key_is_recognized_and_unknown_x_attributes_are_skipped)
    {
        const auto root = xaml_parser::parse(R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml">
	<Color x:Key="riGHTcolor" x:Whatever="ignored">#ff0000</Color>
</ContentPage>)xml");
        const auto color = as<element_node>(root->collection_items().front());
        ASSERT_NE(color, nullptr);

        const auto key = as<value_node>(color->properties().try_get(xml_name::x_key()));
        ASSERT_NE(key, nullptr);
        EXPECT_EQ(key->value(), "riGHTcolor");
        // ParsePropertyName maps unknown x-namespace attributes to XmlName.Empty → skipped.
        EXPECT_EQ(color->properties().size(), 1U);
    }

    TEST(xaml_parser, x2006_namespace_directives_map_like_x2009)
    {
        const auto root = xaml_parser::parse(R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml"
             x:Name="legacy" x:FieldModifier="public"/>)xml");
        const auto root_name = as<value_node>(root->properties().try_get(xml_name::x_name()));
        ASSERT_NE(root_name, nullptr);
        EXPECT_EQ(root_name->value(), "legacy");
        EXPECT_NE(root->properties().try_get(xml_name::x_field_modifier()), nullptr);
    }

    TEST(xaml_parser, mc_ignorable_attribute_passes_through_with_its_namespace)
    {
        const auto root = xaml_parser::parse(R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006"
             mc:Ignorable="d"/>)xml");
        const auto ignorable = as<value_node>(root->properties().try_get(xml_name::mc_ignorable()));
        ASSERT_NE(ignorable, nullptr);
        EXPECT_EQ(ignorable->value(), "d");
    }

    // ---- property-element syntax ----

    TEST(xaml_parser, property_element_syntax_fills_properties)
    {
        const auto root = xaml_parser::parse(R"xml(
<Button xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<Button.Text>Click Me</Button.Text>
</Button>)xml");
        // Property ELEMENTS get the default xmlns (unlike attributes).
        const auto text = as<value_node>(root->properties().try_get(name_of(maui_uri, "Text")));
        ASSERT_NE(text, nullptr);
        EXPECT_EQ(text->value(), "Click Me"); // trimmed
        EXPECT_TRUE(text->has_line_info());
    }

    TEST(xaml_parser, empty_property_element_adds_no_property)
    {
        const auto root = xaml_parser::parse(R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<Label.Text></Label.Text>
</Label>)xml");
        EXPECT_TRUE(root->properties().empty());
    }

    TEST(xaml_parser, duplicate_property_element_throws)
    {
        try
        {
            (void)xaml_parser::parse(R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<Label.Text>a</Label.Text>
	<Label.Text>b</Label.Text>
</Label>)xml");
            FAIL() << "expected xaml_parse_exception";
        }
        catch (const xaml_parse_exception& exception)
        {
            EXPECT_EQ(exception.unformatted_message(), "'Label.Text' is a duplicate property name.");
            EXPECT_TRUE(exception.has_line_info());
        }
    }

    TEST(xaml_parser, property_element_attributes_are_warned_about_and_ignored)
    {
        const auto root = xaml_parser::parse(R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<Label.Text Bogus="1">Foo</Label.Text>
</Label>)xml");
        ASSERT_EQ(root->warnings().size(), 1U);
        EXPECT_EQ(root->warnings().front().message,
                  "Property element 'Text' cannot have attributes. Attribute 'Bogus' will be ignored.");
        const auto text = as<value_node>(root->properties().try_get(name_of(maui_uri, "Text")));
        ASSERT_NE(text, nullptr);
        EXPECT_EQ(text->value(), "Foo");
    }

    TEST(xaml_parser, attached_property_element_keeps_its_dotted_local_name)
    {
        const auto root = xaml_parser::parse(R"xml(
<Button xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<Grid.Row>1</Grid.Row>
</Button>)xml");
        const auto row = as<value_node>(root->properties().try_get(name_of(maui_uri, "Grid.Row")));
        ASSERT_NE(row, nullptr);
        EXPECT_EQ(row->value(), "1");
    }

    TEST(xaml_parser, attached_property_attribute_rebinds_to_the_default_namespace)
    {
        const auto root = xaml_parser::parse(R"xml(
<Grid xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<Button Grid.Row="1" Text="a"/>
</Grid>)xml");
        const auto button = as<element_node>(root->collection_items().front());
        ASSERT_NE(button, nullptr);
        // dotted + unprefixed attribute → C# rebinds "" to LookupNamespace("").
        EXPECT_NE(button->properties().try_get(name_of(maui_uri, "Grid.Row")), nullptr);
        // …while the plain attribute stays namespace-less.
        EXPECT_NE(button->properties().try_get(name_of("", "Text")), nullptr);
    }

    TEST(xaml_parser, multiple_property_element_children_become_a_list_node)
    {
        const auto root = xaml_parser::parse(R"xml(
<Grid xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<Grid.RowDefinitions>
		<RowDefinition/>
		<RowDefinition/>
	</Grid.RowDefinitions>
</Grid>)xml");
        const auto rows = as<list_node>(root->properties().try_get(name_of(maui_uri, "RowDefinitions")));
        ASSERT_NE(rows, nullptr);
        ASSERT_EQ(rows->collection_items().size(), 2U);
        for (const auto& item : rows->collection_items())
        {
            const auto definition = as<element_node>(item);
            ASSERT_NE(definition, nullptr);
            EXPECT_EQ(definition->type().name(), "RowDefinition");
        }
    }

    TEST(xaml_parser, single_property_element_child_is_the_node_itself)
    {
        const auto root = xaml_parser::parse(R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<ContentPage.Content>
		<Label/>
	</ContentPage.Content>
</ContentPage>)xml");
        const auto content = as<element_node>(root->properties().try_get(name_of(maui_uri, "Content")));
        ASSERT_NE(content, nullptr);
        EXPECT_EQ(content->type().name(), "Label");
    }

    // ---- markup detection (XamlParser.GetValueNode) ----

    TEST(xaml_parser, brace_attribute_values_become_markup_nodes)
    {
        const auto root = xaml_parser::parse(R"xml(
<ListView xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
          ItemTemplate="{StaticResource celltemplate}"/>)xml");
        const auto markup = as<markup_node>(root->properties().try_get(name_of("", "ItemTemplate")));
        ASSERT_NE(markup, nullptr);
        EXPECT_EQ(markup->markup_string(), "{StaticResource celltemplate}");
    }

    TEST(xaml_parser, markup_attribute_values_are_trimmed)
    {
        const auto root = xaml_parser::parse(R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui" Text="  {Binding Name}  "/>)xml");
        const auto markup = as<markup_node>(root->properties().try_get(name_of("", "Text")));
        ASSERT_NE(markup, nullptr);
        EXPECT_EQ(markup->markup_string(), "{Binding Name}");
    }

    // StringLiterals.xaml: Text="{}Foo" / Text="{}{Foo}" are literals, not markup.
    TEST(xaml_parser, escaped_attribute_values_become_escaped_value_nodes)
    {
        const auto root = xaml_parser::parse(R"xml(
<StackLayout xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml">
	<Label x:Name="label0" Text="{}Foo" />
	<Label x:Name="label1" Text="{}{Foo}" />
</StackLayout>)xml");
        const auto label0 = as<element_node>(root->collection_items()[0]);
        const auto text0 = as<value_node>(label0->properties().try_get(name_of("", "Text")));
        ASSERT_NE(text0, nullptr);
        EXPECT_EQ(text0->value(), "Foo");
        EXPECT_TRUE(text0->is_escaped());

        const auto label1 = as<element_node>(root->collection_items()[1]);
        const auto text1 = as<value_node>(label1->properties().try_get(name_of("", "Text")));
        ASSERT_NE(text1, nullptr);
        EXPECT_EQ(text1->value(), "{Foo}");
        EXPECT_TRUE(text1->is_escaped());
    }

    // ---- text content ----

    TEST(xaml_parser, text_content_becomes_a_trimmed_value_node_collection_item)
    {
        const auto root = xaml_parser::parse(R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	Foo
</Label>)xml");
        ASSERT_EQ(root->collection_items().size(), 1U);
        const auto text = as<value_node>(root->collection_items().front());
        ASSERT_NE(text, nullptr);
        EXPECT_EQ(text->value(), "Foo");
    }

    // StringLiterals.xaml label2: the "{}" escape is NOT processed for text content at parse time
    // (GetValueNode only runs for attributes; the strip happens during hydration).
    TEST(xaml_parser, text_content_is_not_markup_or_escape_processed)
    {
        const auto root = xaml_parser::parse(R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	{}Foo
</Label>)xml");
        const auto text = as<value_node>(root->collection_items().front());
        ASSERT_NE(text, nullptr);
        EXPECT_EQ(text->value(), "{}Foo");
        EXPECT_FALSE(text->is_escaped());
    }

    TEST(xaml_parser, adjacent_text_and_cdata_merge_into_one_value_node)
    {
        const auto root = xaml_parser::parse(R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui">foo<![CDATA[ {bar} ]]></Label>)xml");
        ASSERT_EQ(root->collection_items().size(), 1U);
        const auto text = as<value_node>(root->collection_items().front());
        ASSERT_NE(text, nullptr);
        EXPECT_EQ(text->value(), "foo{bar}"); // each run is trimmed, then appended
    }

    TEST(xaml_parser, comments_are_ignored)
    {
        const auto root = xaml_parser::parse(R"xml(
<StackLayout xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<!-- a comment -->
	<Label/>
</StackLayout>)xml");
        ASSERT_EQ(root->collection_items().size(), 1U);
        EXPECT_NE(as<element_node>(root->collection_items().front()), nullptr);
    }

    // ---- x:Arguments + DataTemplate/_CreateContent ----

    TEST(xaml_parser, x_arguments_single_child_is_stored_under_the_directive_name)
    {
        const auto root = xaml_parser::parse(R"xml(
<Color xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
       xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml">
	<x:Arguments>
		<x:String>bar</x:String>
	</x:Arguments>
</Color>)xml");
        const auto argument = as<element_node>(root->properties().try_get(xml_name::x_arguments()));
        ASSERT_NE(argument, nullptr);
        EXPECT_EQ(argument->type().name(), "String");
        EXPECT_EQ(argument->type().namespace_uri(), x2009_uri);
    }

    TEST(xaml_parser, x_arguments_multiple_children_become_a_list_node)
    {
        const auto root = xaml_parser::parse(R"xml(
<Color xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
       xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml">
	<x:Arguments>
		<x:Double>0.5</x:Double>
		<x:Double>0.25</x:Double>
	</x:Arguments>
</Color>)xml");
        const auto arguments = as<list_node>(root->properties().try_get(xml_name::x_arguments()));
        ASSERT_NE(arguments, nullptr);
        EXPECT_EQ(arguments->collection_items().size(), 2U);
    }

    TEST(xaml_parser, duplicate_x_arguments_throws)
    {
        try
        {
            (void)xaml_parser::parse(R"xml(
<Color xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
       xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml">
	<x:Arguments><x:Double>1</x:Double></x:Arguments>
	<x:Arguments><x:Double>2</x:Double></x:Arguments>
</Color>)xml");
            FAIL() << "expected xaml_parse_exception";
        }
        catch (const xaml_parse_exception& exception)
        {
            EXPECT_EQ(exception.unformatted_message(), "'x:Arguments' is a duplicate directive name.");
        }
    }

    TEST(xaml_parser, data_template_child_becomes_create_content)
    {
        // The DataTemplate.xaml ListView shape.
        const auto root = xaml_parser::parse(R"xml(
<ListView xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<ListView.ItemTemplate>
		<DataTemplate>
			<TextCell Text="{Binding}"/>
		</DataTemplate>
	</ListView.ItemTemplate>
</ListView>)xml");
        const auto data_template = as<element_node>(root->properties().try_get(name_of(maui_uri, "ItemTemplate")));
        ASSERT_NE(data_template, nullptr);
        EXPECT_EQ(data_template->type().name(), "DataTemplate");

        const auto content = as<element_node>(data_template->properties().try_get(xml_name::create_content()));
        ASSERT_NE(content, nullptr);
        EXPECT_EQ(content->type().name(), "TextCell");
        EXPECT_TRUE(data_template->collection_items().empty());

        const auto binding = as<markup_node>(content->properties().try_get(name_of("", "Text")));
        ASSERT_NE(binding, nullptr);
        EXPECT_EQ(binding->markup_string(), "{Binding}");
    }

    TEST(xaml_parser, multiple_data_template_children_throw)
    {
        try
        {
            (void)xaml_parser::parse(R"xml(
<DataTemplate xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<TextCell/>
	<TextCell/>
</DataTemplate>)xml");
            FAIL() << "expected xaml_parse_exception";
        }
        catch (const xaml_parse_exception& exception)
        {
            EXPECT_EQ(exception.unformatted_message(), "Multiple child elements in DataTemplate");
        }
    }

    // ---- x:TypeArguments (GenericsTests.xaml shapes) ----

    TEST(xaml_parser, x_type_arguments_flow_into_the_element_xml_type)
    {
        const auto root = xaml_parser::parse(R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml"
             xmlns:scg="clr-namespace:System.Collections.Generic;assembly=mscorlib">
	<scg:List x:TypeArguments="x:String" x:Key="stringList"/>
</ContentPage>)xml");
        const auto list = as<element_node>(root->collection_items().front());
        ASSERT_NE(list, nullptr);
        EXPECT_EQ(list->type().name(), "List");
        EXPECT_EQ(list->type().namespace_uri(), "clr-namespace:System.Collections.Generic;assembly=mscorlib");
        ASSERT_EQ(list->type().type_arguments().size(), 1U);
        EXPECT_EQ(list->type().type_arguments().front(), xml_type(std::string(x2009_uri), "String"));

        // The textual expression is still a property (C# stores the parsed list in the node).
        const auto raw = as<value_node>(list->properties().try_get(xml_name::x_type_arguments()));
        ASSERT_NE(raw, nullptr);
        EXPECT_EQ(raw->value(), "x:String");
    }

    TEST(xaml_parser, x_type_arguments_parse_lists_and_nested_generics)
    {
        const auto root = xaml_parser::parse(R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml"
             xmlns:sys="clr-namespace:System;assembly=mscorlib"
             xmlns:scg="clr-namespace:System.Collections.Generic;assembly=mscorlib">
	<scg:Dictionary x:TypeArguments="sys:String, sys:String"/>
	<scg:List x:TypeArguments="scg:KeyValuePair(sys:String,sys:String)"/>
</ContentPage>)xml");
        constexpr std::string_view sys_ns = "clr-namespace:System;assembly=mscorlib";
        constexpr std::string_view scg_ns = "clr-namespace:System.Collections.Generic;assembly=mscorlib";

        const auto dictionary = as<element_node>(root->collection_items()[0]);
        ASSERT_EQ(dictionary->type().type_arguments().size(), 2U);
        EXPECT_EQ(dictionary->type().type_arguments()[0], xml_type(std::string(sys_ns), "String"));
        EXPECT_EQ(dictionary->type().type_arguments()[1], xml_type(std::string(sys_ns), "String"));

        const auto squared = as<element_node>(root->collection_items()[1]);
        ASSERT_EQ(squared->type().type_arguments().size(), 1U);
        const xml_type expected(std::string(scg_ns), "KeyValuePair",
                                {xml_type(std::string(sys_ns), "String"), xml_type(std::string(sys_ns), "String")});
        EXPECT_EQ(squared->type().type_arguments().front(), expected);
    }

    TEST(xaml_parser, x_type_arguments_with_an_undeclared_prefix_throws)
    {
        try
        {
            (void)xaml_parser::parse(R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml"
             xmlns:scg="clr-namespace:System.Collections.Generic;assembly=mscorlib">
	<scg:List x:TypeArguments="nope:String"/>
</ContentPage>)xml");
            FAIL() << "expected xaml_parse_exception";
        }
        catch (const xaml_parse_exception& exception)
        {
            EXPECT_EQ(exception.unformatted_message(), "No xmlns declaration for prefix 'nope'.");
        }
    }

    // ---- error paths: malformed XML, undeclared prefixes, duplicate attributes ----

    TEST(xaml_parser, malformed_xml_throws_with_line_info)
    {
        try
        {
            (void)xaml_parser::parse(R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<Label>
</ContentPage>)xml");
            FAIL() << "expected xaml_parse_exception";
        }
        catch (const xaml_parse_exception& exception)
        {
            EXPECT_TRUE(exception.has_line_info());
            EXPECT_NE(exception.unformatted_message(), "");
        }
    }

    TEST(xaml_parser, empty_input_throws)
    {
        EXPECT_THROW((void)xaml_parser::parse(""), xaml_parse_exception);
        EXPECT_THROW((void)xaml_parser::parse("<!-- only a comment -->"), xaml_parse_exception);
    }

    TEST(xaml_parser, undeclared_element_prefix_throws)
    {
        try
        {
            (void)xaml_parser::parse(R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<local:MockView/>
</ContentPage>)xml");
            FAIL() << "expected xaml_parse_exception";
        }
        catch (const xaml_parse_exception& exception)
        {
            EXPECT_EQ(exception.unformatted_message(), "'local' is an undeclared prefix.");
        }
    }

    TEST(xaml_parser, duplicate_attribute_throws)
    {
        try
        {
            (void)xaml_parser::parse(R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui" Text="a" Text="b"/>)xml");
            FAIL() << "expected xaml_parse_exception";
        }
        catch (const xaml_parse_exception& exception)
        {
            EXPECT_EQ(exception.unformatted_message(), "'Text' is a duplicate attribute name.");
        }
    }

    TEST(xaml_parser, exception_what_is_position_formatted)
    {
        const xaml_parse_exception exception("Boom", 3, 7);
        EXPECT_STREQ(exception.what(), "Position 3:7. Boom");
        EXPECT_EQ(exception.unformatted_message(), "Boom");

        const xaml_parse_exception no_info("Boom");
        EXPECT_STREQ(no_info.what(), "Boom");
        EXPECT_FALSE(no_info.has_line_info());
    }

    // ---- line info (IXmlLineInfo: 1-based, element positions point at the name) ----

    TEST(xaml_parser, line_info_is_recorded_on_elements)
    {
        const auto root = xaml_parser::parse("<ContentPage xmlns=\"http://schemas.microsoft.com/dotnet/2021/maui\">\n"
                                             "  <Label Text=\"Foo\" />\n"
                                             "</ContentPage>");
        EXPECT_TRUE(root->has_line_info());
        EXPECT_EQ(root->line_number(), 1);
        EXPECT_EQ(root->line_position(), 2); // the name, just past '<'

        const auto label = as<element_node>(root->collection_items().front());
        ASSERT_NE(label, nullptr);
        EXPECT_EQ(label->line_number(), 2);
        EXPECT_EQ(label->line_position(), 4);
    }

    // ---- ignorable prefixes (PrefixesToIgnore + targetPlatform) ----

    constexpr std::string_view target_platform_xaml = R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:ios="clr-namespace:Foo;assembly=Bar;targetPlatform=iOS">
	<Label/>
</ContentPage>)xml";

    TEST(xaml_parser, foreign_target_platform_prefixes_are_ignorable)
    {
        parse_options options;
        options.target_platform = "Android";
        const auto root = xaml_parser::parse(target_platform_xaml, options);
        ASSERT_EQ(root->ignorable_prefixes().size(), 1U);
        EXPECT_EQ(root->ignorable_prefixes().front(), "ios");
    }

    TEST(xaml_parser, matching_target_platform_prefixes_are_kept)
    {
        parse_options options;
        options.target_platform = "iOS";
        const auto root = xaml_parser::parse(target_platform_xaml, options);
        EXPECT_TRUE(root->ignorable_prefixes().empty());
    }

    TEST(xaml_parser, without_a_platform_every_target_platform_prefix_is_ignorable)
    {
        const auto root = xaml_parser::parse(target_platform_xaml);
        ASSERT_EQ(root->ignorable_prefixes().size(), 1U);
        EXPECT_EQ(root->ignorable_prefixes().front(), "ios");
    }

    TEST(xaml_parser, windows_target_platform_matches_winui)
    {
        parse_options options;
        options.target_platform = "WinUI";
        const auto root = xaml_parser::parse(R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:win="clr-namespace:Foo;assembly=Bar;targetPlatform=Windows"/>)xml",
                                             options);
        EXPECT_TRUE(root->ignorable_prefixes().empty());
    }

    TEST(xaml_node, skip_prefix_walks_up_through_parents)
    {
        const auto root = xaml_parser::parse(target_platform_xaml);
        // Assign parents the way XamlLoader does: a top-down delegate visitor.
        xaml_node_visitor parenting([](i_xaml_node& node, i_xaml_node* parent) { node.set_parent(parent); });
        root->accept(parenting, nullptr);

        const auto label = as<element_node>(root->collection_items().front());
        ASSERT_NE(label, nullptr);
        EXPECT_EQ(label->parent(), root.get());
        EXPECT_TRUE(maui::xaml::skip_prefix(*label, "ios")); // inherited from the root
        EXPECT_FALSE(maui::xaml::skip_prefix(*label, "android"));
    }

    // ---- the visitor seam: ordering knobs + data-template stops ----

    struct recording_visitor final : maui::xaml::i_xaml_node_visitor
    {
        std::vector<std::string> visited;
        tree_visiting_mode mode = tree_visiting_mode::top_down;
        bool stop_on_data_template_value = false;
        bool visit_node_on_data_template_value = true;

        [[nodiscard]] tree_visiting_mode visiting_mode() const override
        {
            return mode;
        }
        [[nodiscard]] bool stop_on_data_template() const override
        {
            return stop_on_data_template_value;
        }
        [[nodiscard]] bool visit_node_on_data_template() const override
        {
            return visit_node_on_data_template_value;
        }
        [[nodiscard]] bool stop_on_resource_dictionary() const override
        {
            return false;
        }

        void visit(value_node& node, i_xaml_node* /*parent*/) override
        {
            visited.push_back("value:" + node.value());
        }
        void visit(markup_node& node, i_xaml_node* /*parent*/) override
        {
            visited.push_back("markup:" + node.markup_string());
        }
        void visit(element_node& node, i_xaml_node* /*parent*/) override
        {
            visited.push_back(node.type().name());
        }
        void visit(root_node& node, i_xaml_node* /*parent*/) override
        {
            visited.push_back("root:" + node.type().name());
        }
        void visit(list_node& /*node*/, i_xaml_node* /*parent*/) override
        {
            visited.emplace_back("list");
        }
        [[nodiscard]] bool skip_children(i_xaml_node& /*node*/, i_xaml_node* /*parent*/) override
        {
            return false;
        }
        [[nodiscard]] bool is_resource_dictionary(element_node& /*node*/) override
        {
            return false;
        }
    };

    constexpr std::string_view small_tree_xaml = R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<StackLayout>
		<Label Text="Foo"/>
	</StackLayout>
</ContentPage>)xml";

    TEST(xaml_node, top_down_visits_parents_before_children_and_properties_before_items)
    {
        const auto root = xaml_parser::parse(small_tree_xaml);
        recording_visitor visitor;
        root->accept(visitor, nullptr);
        const std::vector<std::string> expected{"root:ContentPage", "StackLayout", "Label", "value:Foo"};
        EXPECT_EQ(visitor.visited, expected);
    }

    TEST(xaml_node, bottom_up_visits_children_before_parents)
    {
        const auto root = xaml_parser::parse(small_tree_xaml);
        recording_visitor visitor;
        visitor.mode = tree_visiting_mode::bottom_up;
        root->accept(visitor, nullptr);
        const std::vector<std::string> expected{"value:Foo", "Label", "StackLayout", "root:ContentPage"};
        EXPECT_EQ(visitor.visited, expected);
    }

    constexpr std::string_view data_template_tree_xaml = R"xml(
<ListView xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<ListView.ItemTemplate>
		<DataTemplate>
			<TextCell Text="{Binding}"/>
		</DataTemplate>
	</ListView.ItemTemplate>
</ListView>)xml";

    TEST(xaml_node, stop_on_data_template_skips_the_template_content_children)
    {
        const auto root = xaml_parser::parse(data_template_tree_xaml);
        recording_visitor visitor;
        visitor.stop_on_data_template_value = true;
        root->accept(visitor, nullptr);
        // TextCell (the _CreateContent node) is still visited, but its children are not.
        const std::vector<std::string> expected{"root:ListView", "DataTemplate", "TextCell"};
        EXPECT_EQ(visitor.visited, expected);
    }

    TEST(xaml_node, visit_node_on_data_template_false_skips_only_the_content_node)
    {
        const auto root = xaml_parser::parse(data_template_tree_xaml);
        recording_visitor visitor;
        visitor.visit_node_on_data_template_value = false;
        root->accept(visitor, nullptr);
        // TextCell itself is not visited; its children still are.
        const std::vector<std::string> expected{"root:ListView", "DataTemplate", "markup:{Binding}"};
        EXPECT_EQ(visitor.visited, expected);
    }

    TEST(xaml_node, list_node_children_are_visited_through_the_list)
    {
        const auto root = xaml_parser::parse(R"xml(
<Grid xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<Grid.RowDefinitions>
		<RowDefinition/>
		<RowDefinition/>
	</Grid.RowDefinitions>
</Grid>)xml");
        recording_visitor visitor;
        root->accept(visitor, nullptr);
        const std::vector<std::string> expected{"root:Grid", "list", "RowDefinition", "RowDefinition"};
        EXPECT_EQ(visitor.visited, expected);
    }

    // ---- clone ----

    TEST(xaml_node, clone_deep_copies_the_tree)
    {
        const auto root = xaml_parser::parse(small_tree_xaml);
        const auto clone = as<element_node>(root->clone());
        ASSERT_NE(clone, nullptr);
        // Like C#: RootNode has no Clone override, so the copy is a plain element_node.
        EXPECT_EQ(as<root_node>(root->clone()), nullptr);

        EXPECT_EQ(clone->type().name(), "ContentPage");
        ASSERT_EQ(clone->collection_items().size(), 1U);
        const auto stack_copy = as<element_node>(clone->collection_items().front());
        ASSERT_NE(stack_copy, nullptr);
        EXPECT_NE(stack_copy, root->collection_items().front()); // deep, not shared
        const auto label_copy = as<element_node>(stack_copy->collection_items().front());
        const auto text_copy = as<value_node>(label_copy->properties().try_get(name_of("", "Text")));
        ASSERT_NE(text_copy, nullptr);
        EXPECT_EQ(text_copy->value(), "Foo");
    }

    // ---- xml_name_map local-name lookup (XmlNameExtensions.TryGetValue) ----

    TEST(xaml_node, properties_can_be_found_by_local_name)
    {
        const auto root = xaml_parser::parse(R"xml(
<Button xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<Button.Text>Click Me</Button.Text>
</Button>)xml");
        xml_name matched;
        const auto text = as<value_node>(root->properties().try_get("Text", matched));
        ASSERT_NE(text, nullptr);
        EXPECT_EQ(matched, name_of(maui_uri, "Text"));
        EXPECT_EQ(as<value_node>(text)->value(), "Click Me");
        EXPECT_EQ(root->properties().try_get("Nope", matched), nullptr);
    }

    // ---- namespace resolver snapshots ----

    TEST(xaml_node, nodes_resolve_the_prefixes_in_scope_where_they_appeared)
    {
        const auto root = xaml_parser::parse(R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml">
	<Label xmlns:inner="clr-namespace:Inner" Text="a"/>
	<Button Text="b"/>
</ContentPage>)xml");
        EXPECT_EQ(root->namespace_resolver()->lookup_namespace("x"), std::string(x2009_uri));
        EXPECT_EQ(root->namespace_resolver()->lookup_namespace(""), std::string(maui_uri));
        EXPECT_EQ(root->namespace_resolver()->lookup_namespace("inner"), std::nullopt);

        const auto label = as<element_node>(root->collection_items()[0]);
        EXPECT_EQ(label->namespace_resolver()->lookup_namespace("inner"), "clr-namespace:Inner");

        const auto button = as<element_node>(root->collection_items()[1]);
        EXPECT_EQ(button->namespace_resolver()->lookup_namespace("inner"), std::nullopt);
        EXPECT_EQ(button->namespace_resolver()->lookup_prefix(x2009_uri), "x");
    }

    // ---- the markup tokenization primitives (MarkupExpressionParser) ----

    TEST(markup_tokenizer, match_markup_extracts_the_extension_name)
    {
        const auto match = maui::xaml::match_markup("{Binding Foo}");
        EXPECT_TRUE(match.matched);
        EXPECT_EQ(match.match, "Binding");
        EXPECT_EQ(match.end, 8U); // just past "Binding"

        const auto braces_only = maui::xaml::match_markup("{StaticResource}");
        EXPECT_TRUE(braces_only.matched);
        EXPECT_EQ(braces_only.match, "StaticResource");

        const auto spaced = maui::xaml::match_markup("{  Binding }");
        EXPECT_TRUE(spaced.matched);
        EXPECT_EQ(spaced.match, "Binding");
    }

    TEST(markup_tokenizer, match_markup_failure_codes_mirror_the_source)
    {
        const auto too_short = maui::xaml::match_markup("{");
        EXPECT_FALSE(too_short.matched);
        EXPECT_EQ(too_short.end, 1U);

        const auto no_brace = maui::xaml::match_markup("Binding}");
        EXPECT_FALSE(no_brace.matched);
        EXPECT_EQ(no_brace.end, 2U);

        const auto all_blank = maui::xaml::match_markup("{    ");
        EXPECT_FALSE(all_blank.matched);
        EXPECT_EQ(all_blank.end, 3U);

        const auto unterminated = maui::xaml::match_markup("{Binding");
        EXPECT_FALSE(unterminated.matched);
        EXPECT_EQ(unterminated.end, 6U);
    }

    // Leading whitespace is the CALLER's job (C# ParseProperty TrimStart()s `remaining` before
    // every GetNextPiece call); the tokenizer itself only trims trailing whitespace.
    [[nodiscard]] std::string_view trim_start(std::string_view text)
    {
        const auto first = text.find_first_not_of(' ');
        return first == std::string_view::npos ? std::string_view{} : text.substr(first);
    }

    TEST(markup_tokenizer, get_next_piece_splits_on_markup_delimiters)
    {
        const std::string_view expression = "Path=foo, Mode=TwoWay}";

        const auto name = maui::xaml::get_next_piece(expression);
        EXPECT_EQ(name.piece, "Path");
        EXPECT_EQ(name.next, '=');

        const auto value = maui::xaml::get_next_piece(name.remaining);
        EXPECT_EQ(value.piece, "foo");
        EXPECT_EQ(value.next, ',');

        // Without the caller-side TrimStart the leading space stays in the piece, like C#.
        EXPECT_EQ(maui::xaml::get_next_piece(value.remaining).piece, " Mode");

        const auto mode_name = maui::xaml::get_next_piece(trim_start(value.remaining));
        EXPECT_EQ(mode_name.piece, "Mode");
        EXPECT_EQ(mode_name.next, '=');

        const auto mode_value = maui::xaml::get_next_piece(mode_name.remaining);
        EXPECT_EQ(mode_value.piece, "TwoWay");
        EXPECT_EQ(mode_value.next, '}');
        EXPECT_TRUE(mode_value.remaining.empty());
    }

    TEST(markup_tokenizer, get_next_piece_honors_quotes_and_escapes)
    {
        const auto quoted = maui::xaml::get_next_piece("'a, b=c', X}");
        EXPECT_EQ(quoted.piece, "a, b=c");
        EXPECT_EQ(quoted.next, ',');

        const auto double_quoted = maui::xaml::get_next_piece("\"hi there\"}");
        EXPECT_EQ(double_quoted.piece, "hi there");
        EXPECT_EQ(double_quoted.next, '}');

        const auto escaped = maui::xaml::get_next_piece("a\\,b}");
        EXPECT_EQ(escaped.piece, "a,b");
        EXPECT_EQ(escaped.next, '}');

        const auto trimmed = maui::xaml::get_next_piece("foo   }");
        EXPECT_EQ(trimmed.piece, "foo"); // trailing whitespace trimmed
    }

    TEST(markup_tokenizer, get_next_piece_error_paths)
    {
        EXPECT_THROW((void)maui::xaml::get_next_piece("'unterminated"), xaml_parse_exception);
        EXPECT_THROW((void)maui::xaml::get_next_piece("}rest"), xaml_parse_exception); // empty piece
        EXPECT_THROW((void)maui::xaml::get_next_piece("no delimiter"), xaml_parse_exception);
    }

    TEST(markup_tokenizer, parse_markup_name_splits_prefixes)
    {
        EXPECT_EQ(maui::xaml::parse_markup_name("x:Static"), (std::pair<std::string, std::string>("x", "Static")));
        EXPECT_EQ(maui::xaml::parse_markup_name("Binding"), (std::pair<std::string, std::string>("", "Binding")));
        EXPECT_THROW((void)maui::xaml::parse_markup_name("a:b:c"), std::invalid_argument);
    }

    // XmlNamespaceManager pre-binds the empty prefix to "", so an unprefixed type name with no
    // default xmlns resolves to the empty namespace (C# does not throw).
    TEST(xaml_node, empty_prefix_is_pre_bound_like_xml_namespace_manager)
    {
        EXPECT_EQ(xml_namespace_resolver::built_in()->lookup_namespace(""), "");
    }

    TEST(xaml_parser, unprefixed_type_arguments_resolve_against_the_default_namespace)
    {
        // GenericsTests.xaml: <scg:List x:TypeArguments="Button" …/> under the maui default xmlns.
        const auto root = xaml_parser::parse(R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml"
             xmlns:scg="clr-namespace:System.Collections.Generic;assembly=mscorlib">
	<scg:List x:TypeArguments="Button"/>
</ContentPage>)xml");
        const auto list = as<element_node>(root->collection_items().front());
        ASSERT_EQ(list->type().type_arguments().size(), 1U);
        EXPECT_EQ(list->type().type_arguments().front(), xml_type(std::string(maui_uri), "Button"));
    }

    TEST(xaml_parser, duplicate_attribute_on_a_property_element_throws)
    {
        EXPECT_THROW((void)xaml_parser::parse(R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<Label.Text Bogus="1" Bogus="2">Foo</Label.Text>
</Label>)xml"),
                     xaml_parse_exception);
    }

    // ---- type_arguments_parser directly ----

    TEST(type_arguments, parse_single_rejects_lists)
    {
        const auto resolver = xml_namespace_resolver::built_in()->extend(
            {{"x", std::string(x2009_uri)}, {"sys", "clr-namespace:System;assembly=mscorlib"}});

        const xml_type single = maui::xaml::type_arguments_parser::parse_single("x:String", *resolver);
        EXPECT_EQ(single, xml_type(std::string(x2009_uri), "String"));

        EXPECT_THROW((void)maui::xaml::type_arguments_parser::parse_single("x:String, x:Int32", *resolver),
                     xaml_parse_exception);
    }
} // namespace
