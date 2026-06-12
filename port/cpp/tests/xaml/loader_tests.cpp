// Tests for maui::xaml::xaml_loader (M7 wave 2) — the e2e port of
// src/Controls/tests/Xaml.UnitTests/LoaderTests.cs (LoadFromXaml drives XamlLoader.Load ==
// load_into; XamlLoader.Create == load) plus the MarkupExtensionTests.cs cases that run through a
// full load (TestInXaml / TestDocumentationCode, over port-registered custom extensions — the C#
// originals resolve their extension CLASSES by reflection from the test assembly).
//
// The binding cases — TestSetBindingToBindableProperty / TestBindingPath /
// TestBindingModeAndConverter / TestSetBindingToNonBindablePropertyShouldThrow — run LIVE against
// the runtime binding engine via register_runtime_bindings() (the hook's rejecting DEFAULT is still
// asserted separately, plus the hook seam itself).
//
// Documented deviations (the C# cases that need machinery the port defers; each fails LOUDLY):
//   - TestAttachedBP / TestAttachedBPWithDifferentNS / TestBindOnAttachedBP: attached-property
//     dotted names are an M7 deferral (the cannot-assign error is asserted below);
//   - TestEvent / TestFailingEvent / TestConnectingEventOnMethodWithWrongSignature /
//     TestEventWithCustomEventArgs: event wiring needs the rootElement method lookup C# does by
//     reflection (cannot-assign, asserted below);
//   - TestEmptyTemplate (DataTemplate), StyleWithoutTargetTypeThrows (Style), ParseEnum/ParseFlags
//     (enum converters are the U4 unit), CreateNewChildrenCollection (custom collection types),
//     LoadFromXamlResource / ThrowOnMissingXamlResource (embedded resources / assemblies),
//     TestRootName's x:Class half (no class generation): not portable as-is; Style stays a loud
//     unknown-type failure (asserted below).
#include "maui/xaml/xaml_loader.hpp"

#include <any>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/application.hpp"
#include "maui/controls/bindings/i_value_converter.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/resource_dictionary.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/property.hpp"
#include "maui/core/type_tag.hpp"
#include "maui/graphics/color.hpp"
#include "maui/xaml/i_markup_extension.hpp"
#include "maui/xaml/markup_extensions.hpp"
#include "maui/xaml/xaml_binding_applier.hpp"
#include "maui/xaml/xaml_parse_exception.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_runtime_bindings.hpp"
#include <gtest/gtest.h>

namespace
{
    namespace controls = maui::controls;
    using maui::core::bindable_object;
    using maui::core::type_tag;
    using maui::xaml::binding_request;
    using maui::xaml::i_markup_extension;
    using maui::xaml::markup_extension_arguments;
    using maui::xaml::markup_extension_registry;
    using maui::xaml::set_xaml_binding_applier;
    using maui::xaml::xaml_binding_applier;
    using maui::xaml::xaml_load_result;
    using maui::xaml::xaml_loader;
    using maui::xaml::xaml_parse_exception;
    using maui::xaml::xaml_property_registry;
    using maui::xaml::xaml_service_provider;

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

    // ---- load_into (XamlLoader.Load / the LoadFromXaml extension) ----------------------------------

    TEST(xaml_loader, test_root_name)
    {
        // LoaderTests.TestRootName: x:Name on the root registers the ROOT OBJECT itself.
        controls::label view;
        const xaml_load_result result = xaml_loader::load_into(view, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
       xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml"
       x:Name="customView"/>)xml");
        const std::shared_ptr<controls::label> found = result.find_by_name<controls::label>("customView");
        ASSERT_NE(found, nullptr);
        EXPECT_EQ(found.get(), &view); // Assert.Same(view, FindByName("customView"))
    }

    TEST(xaml_loader, test_find_by_x_name)
    {
        // LoaderTests.TestFindByXName (over the port's registered stack layout).
        controls::vertical_stack_layout stacklayout;
        const xaml_load_result result = xaml_loader::load_into(stacklayout, R"xml(
<VerticalStackLayout xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
	<VerticalStackLayout.Children>
		<Label x:Name="label0" Text="Foo"/>
	</VerticalStackLayout.Children>
</VerticalStackLayout>)xml");
        const std::shared_ptr<controls::label> label = result.find_by_name<controls::label>("label0");
        ASSERT_NE(label, nullptr);
        EXPECT_EQ(label->text(), "Foo");
    }

    TEST(xaml_loader, test_unknown_property_should_throw)
    {
        controls::label label;
        EXPECT_EQ(parse_error_message([&] {
                      (void)xaml_loader::load_into(label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui" Text="Foo" UnknownProperty="Bar"/>)xml");
                  }),
                  "Cannot assign property \"UnknownProperty\": Property does not exist, or is not assignable, or "
                  "mismatching type between value and property");
    }

    TEST(xaml_loader, test_set_value_to_bindable_property)
    {
        controls::label label;
        (void)xaml_loader::load_into(label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui" Text="Foo"/>)xml");
        EXPECT_EQ(label.text(), "Foo");
    }

    TEST(xaml_loader, test_non_empty_collection_members)
    {
        // LoaderTests.TestNonEmptyCollectionMembers (Grid children named via x:Name).
        controls::vertical_stack_layout stacklayout;
        const xaml_load_result result = xaml_loader::load_into(stacklayout, R"xml(
<VerticalStackLayout xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
	<VerticalStackLayout.Children>
		<Grid x:Name="grid0">
		</Grid>
		<Grid x:Name="grid1">
		</Grid>
	</VerticalStackLayout.Children>
</VerticalStackLayout>)xml");
        EXPECT_EQ(stacklayout.count(), 2);
        EXPECT_NE(result.find_by_name<bindable_object>("grid0"), nullptr);
        EXPECT_NE(result.find_by_name<bindable_object>("grid1"), nullptr);
    }

    TEST(xaml_loader, test_unknown_type)
    {
        controls::vertical_stack_layout stacklayout;
        EXPECT_EQ(parse_error_message([&] {
                      (void)xaml_loader::load_into(stacklayout, R"xml(
<VerticalStackLayout xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
	<VerticalStackLayout.Children>
		<CustomView />
	</VerticalStackLayout.Children>
</VerticalStackLayout>)xml");
                  }),
                  "Type CustomView not found in xmlns http://schemas.microsoft.com/dotnet/2021/maui");
    }

    TEST(xaml_loader, test_resources)
    {
        // LoaderTests.TestResources (an x:String stands in for the unregistered local converter).
        controls::label label;
        (void)xaml_loader::load_into(label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
       xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
	<Label.Resources>
		<ResourceDictionary>
			<x:String x:Key="reverseConverter">marker</x:String>
		</ResourceDictionary>
	</Label.Resources>
</Label>)xml");
        ASSERT_TRUE(label.is_resources_created());
        EXPECT_NE(label.resources().try_get("reverseConverter"), nullptr);
    }

    TEST(xaml_loader, test_resource_does_require_key)
    {
        controls::label label;
        EXPECT_EQ(parse_error_message([&] {
                      (void)xaml_loader::load_into(label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
       xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
	<Label.Resources>
		<ResourceDictionary>
			<x:String>keyless</x:String>
		</ResourceDictionary>
	</Label.Resources>
</Label>)xml");
                  }),
                  "resources in ResourceDictionary require a x:Key attribute");
    }

    TEST(xaml_loader, use_resources_outside_of_binding)
    {
        // LoaderTests.UseResourcesOutsideOfBinding: a CONTROL kept in a resource dictionary is
        // consumed by Content="{StaticResource bar}" (the child-sink route).
        controls::content_page page;
        const xaml_load_result result = xaml_loader::load_into(page, R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml"
             Content="{StaticResource bar}">
	<ContentPage.Resources>
		<ResourceDictionary>
			<Label Text="Foo" x:Key="bar"/>
		</ResourceDictionary>
	</ContentPage.Resources>
</ContentPage>)xml");
        (void)result;
        auto* label = dynamic_cast<controls::label*>(page.content());
        ASSERT_NE(label, nullptr);
        EXPECT_EQ(label->text(), "Foo");
    }

    TEST(xaml_loader, missing_static_resource_should_throw)
    {
        controls::label label;
        EXPECT_EQ(parse_error_message([&] {
                      (void)xaml_loader::load_into(label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui" Text="{StaticResource foo}"/>)xml");
                  }),
                  "StaticResource not found for key foo");
    }

    TEST(xaml_loader, static_resource_looks_for_application_resources)
    {
        // LoaderTests.StaticResourceLookForApplicationResources, threaded through options
        // .application (the port's Application.Current stand-in).
        controls::application app;
        (void)app.resources().add("foo", std::any{std::string{"FOO"}});
        (void)app.resources().add("bar", std::any{std::string{"BAR"}});
        controls::content_page page;
        const xaml_load_result result = xaml_loader::load_into(page, R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
	<ContentPage.Resources>
		<ResourceDictionary>
			<x:String x:Key="bar">BAZ</x:String>
		</ResourceDictionary>
	</ContentPage.Resources>
	<VerticalStackLayout>
		<Label x:Name="label0" Text="{StaticResource foo}"/>
		<Label x:Name="label1" Text="{StaticResource bar}"/>
	</VerticalStackLayout>
</ContentPage>)xml",
                                                               {.application = &app});
        EXPECT_EQ(result.find_by_name<controls::label>("label0")->text(), "FOO"); // from App.Resources
        EXPECT_EQ(result.find_by_name<controls::label>("label1")->text(), "BAZ"); // local precedence
    }

    TEST(xaml_loader, test_content_properties)
    {
        // LoaderTests.TestContentProperties: the content child is the named object.
        controls::content_page page;
        const xaml_load_result result = xaml_loader::load_into(page, R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
	<Label x:Name="contentview"/>
</ContentPage>)xml");
        ASSERT_NE(page.content(), nullptr);
        EXPECT_EQ(result.find_by_name<controls::label>("contentview").get(), page.content());
    }

    TEST(xaml_loader, find_resource_by_name)
    {
        // LoaderTests.FindResourceByName: a keyed resource is ALSO x:Name-registered.
        controls::content_page page;
        const xaml_load_result result = xaml_loader::load_into(page, R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
	<ContentPage.Resources>
		<ResourceDictionary>
			<Button x:Key="buttonKey" x:Name="buttonName"/>
		</ResourceDictionary>
	</ContentPage.Resources>
	<Label x:Name="label"/>
</ContentPage>)xml");
        ASSERT_TRUE(page.is_resources_created());
        EXPECT_NE(page.resources().try_get("buttonKey"), nullptr);
        EXPECT_NE(result.find_by_name<bindable_object>("buttonName"), nullptr);
    }

    TEST(xaml_loader, style_without_target_type_throws)
    {
        // LoaderTests.StyleWithoutTargetTypeThrows — the port fails earlier (Style is not a
        // registered loadable type; styles-in-XAML are an M7 deferral) but just as loudly.
        controls::label label;
        const std::string message = parse_error_message([&] {
            (void)xaml_loader::load_into(label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<Label.Style>
		<Style>
			<Setter Property="Text" Value="Foo" />
		</Style>
	</Label.Style>
</Label>)xml");
        });
        // The create pass is bottom-up, so the INNER unregistered type fails first (C# creates its
        // real Setter and fails on the missing TargetType later — same loud channel).
        EXPECT_EQ(message, "Type Setter not found in xmlns http://schemas.microsoft.com/dotnet/2021/maui");
    }

    // ---- the documented loud deferrals ({Binding} default, attached properties, events) ------------

    TEST(xaml_loader, test_set_binding_without_an_applier_throws)
    {
        // LoaderTests.TestSetBindingToBindableProperty's markup, against the DEFAULT hook: the
        // runtime-binding engine is a parallel unit, so the load fails loudly instead of binding.
        controls::label label;
        const std::string message = parse_error_message([&] {
            (void)xaml_loader::load_into(label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui" Text="{Binding Path=labeltext}"/>)xml");
        });
        EXPECT_TRUE(message.contains("no binding applier is registered")) << message;
    }

    TEST(xaml_loader, registered_binding_applier_receives_the_request)
    {
        // The xaml_binding_applier hook seam: a registered applier gets the parsed request and the
        // SetBinding-relevant target metadata (the runtime-binding unit's integration point).
        struct captured_request
        {
            std::string xaml_name;
            std::string path;
            maui::core::binding_mode mode = maui::core::binding_mode::default_mode;
            std::string bindable_name;
        };
        auto captured = std::make_shared<std::vector<captured_request>>();
        set_xaml_binding_applier([captured](const xaml_property_registry& properties, bindable_object& /*target*/,
                                            type_tag target_type, const std::string& xaml_name,
                                            const binding_request& request, int /*line*/, int /*position*/) {
            const xaml_property_registry::property_entry* entry = properties.find(target_type, xaml_name);
            captured->push_back({.xaml_name = xaml_name,
                                 .path = request.path,
                                 .mode = request.mode,
                                 .bindable_name = entry != nullptr ? std::string{entry->bindable_name} : ""});
        });

        controls::label label;
        (void)xaml_loader::load_into(label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui" Text="{Binding Path=labeltext, Mode=TwoWay}"/>)xml");
        set_xaml_binding_applier(nullptr); // restore the rejecting default

        ASSERT_EQ(captured->size(), 1U);
        EXPECT_EQ(captured->front().xaml_name, "Text");
        EXPECT_EQ(captured->front().path, "labeltext");
        EXPECT_EQ(captured->front().mode, maui::core::binding_mode::two_way);
        EXPECT_EQ(captured->front().bindable_name, "text"); // the SetBinding routing key
    }

    // ---- the runtime binding bridge (register_runtime_bindings; the C# cases that needed it) ------

    // RAII pin of the real applier: register_runtime_bindings() for the test body, then restore the
    // rejecting default so the deferral tests above stay order-independent.
    class runtime_bindings_guard
    {
    public:
        runtime_bindings_guard()
        {
            maui::xaml::register_runtime_bindings();
        }
        runtime_bindings_guard(const runtime_bindings_guard&) = delete;
        runtime_bindings_guard(runtime_bindings_guard&&) = delete;
        runtime_bindings_guard& operator=(const runtime_bindings_guard&) = delete;
        runtime_bindings_guard& operator=(runtime_bindings_guard&&) = delete;
        ~runtime_bindings_guard()
        {
            set_xaml_binding_applier(nullptr);
        }
    };

    // The C# tests' binding contexts: `new { labeltext = "Foo" }` / the ViewModel { Text } poco
    // become bindable_objects with a named property<T> (the port's INotifyPropertyChanged analog).
    const maui::core::bindable_property<std::string>& vm_labeltext_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"labeltext"};
        return descriptor;
    }
    struct labeltext_view_model : bindable_object
    {
        maui::core::property<std::string> labeltext{*this, vm_labeltext_property()};
    };
    const maui::core::bindable_property<std::string>& vm_text_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"text"};
        return descriptor;
    }
    struct text_view_model : bindable_object
    {
        maui::core::property<std::string> text{*this, vm_text_property()};
    };

    // LoaderTests.ReverseConverter: Convert/ConvertBack reverse the bound string.
    class reverse_converter final : public controls::i_value_converter
    {
    public:
        [[nodiscard]] std::any convert(const std::any& value, type_tag /*target_type*/,
                                       const std::any& /*parameter*/) override
        {
            return reversed(value);
        }
        [[nodiscard]] std::any convert_back(const std::any& value, type_tag /*target_type*/,
                                            const std::any& /*parameter*/) override
        {
            return reversed(value);
        }

    private:
        [[nodiscard]] static std::any reversed(const std::any& value)
        {
            const auto* text = std::any_cast<std::string>(&value);
            if (text == nullptr)
            {
                return value;
            }
            return std::any{std::string{text->rbegin(), text->rend()}};
        }
    };

    TEST(xaml_loader, test_set_binding_to_bindable_property)
    {
        // LoaderTests.TestSetBindingToBindableProperty: the {Binding} binds end-to-end once the
        // runtime applier is registered — default until a context arrives, then the path value.
        const runtime_bindings_guard guard;
        controls::label label;
        (void)xaml_loader::load_into(label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui" Text="{Binding Path=labeltext}"/>)xml");
        EXPECT_EQ(label.text(), ""); // Label.TextProperty.DefaultValue

        auto context = std::make_shared<labeltext_view_model>();
        context->labeltext.set("Foo");
        label.set_binding_context(context);
        EXPECT_EQ(label.text(), "Foo");
    }

    TEST(xaml_loader, test_binding_path)
    {
        // LoaderTests.TestBindingPath: the positional and the named Path spell the same binding;
        // the context set on the root inherits down to both labels.
        const runtime_bindings_guard guard;
        controls::vertical_stack_layout stacklayout;
        const xaml_load_result result = xaml_loader::load_into(stacklayout, R"xml(
<VerticalStackLayout xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
	<VerticalStackLayout.Children>
		<Label x:Name="label0" Text="{Binding text}"/>
		<Label x:Name="label1" Text="{Binding Path=text}"/>
	</VerticalStackLayout.Children>
</VerticalStackLayout>)xml");
        const std::shared_ptr<controls::label> label0 = result.find_by_name<controls::label>("label0");
        const std::shared_ptr<controls::label> label1 = result.find_by_name<controls::label>("label1");
        ASSERT_NE(label0, nullptr);
        ASSERT_NE(label1, nullptr);
        EXPECT_EQ(label0->text(), ""); // Label.TextProperty.DefaultValue
        EXPECT_EQ(label1->text(), "");

        auto context = std::make_shared<text_view_model>();
        context->text.set("Foo");
        stacklayout.set_binding_context(context);
        EXPECT_EQ(label0->text(), "Foo");
        EXPECT_EQ(label1->text(), "Foo");
    }

    TEST(xaml_loader, test_binding_mode_and_converter)
    {
        // LoaderTests.TestBindingModeAndConverter. C# hydrates <local:ReverseConverter x:Key=…/>
        // from markup; custom non-element types are not XAML-creatable in the port, so the resource
        // is code-seeded on the root (the same {StaticResource} live-chain lookup).
        const runtime_bindings_guard guard;
        controls::content_page content_page;
        content_page.resources().set("reverseConverter", std::any{std::static_pointer_cast<controls::i_value_converter>(
                                                             std::make_shared<reverse_converter>())});
        const xaml_load_result result = xaml_loader::load_into(content_page, R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
	<ContentPage.Content>
		<VerticalStackLayout>
			<Label x:Name="label0" Text="{Binding text, Converter={StaticResource reverseConverter}}"/>
			<Label x:Name="label1" Text="{Binding text, Mode=TwoWay}"/>
		</VerticalStackLayout>
	</ContentPage.Content>
</ContentPage>)xml");

        auto context = std::make_shared<text_view_model>();
        context->text.set("foobar");
        content_page.set_binding_context(context);

        const std::shared_ptr<controls::label> label0 = result.find_by_name<controls::label>("label0");
        const std::shared_ptr<controls::label> label1 = result.find_by_name<controls::label>("label1");
        ASSERT_NE(label0, nullptr);
        ASSERT_NE(label1, nullptr);
        EXPECT_EQ(label0->text(), "raboof"); // through ReverseConverter

        label1->set_text("baz"); // TwoWay pushes target -> source
        EXPECT_EQ(context->text.get(), "baz");
    }

    TEST(xaml_loader, test_set_binding_to_non_bindable_property_should_throw)
    {
        // LoaderTests.TestSetBindingToNonBindablePropertyShouldThrow — Window.Title is the port's
        // registered NON-bindable property; SetBinding on it is TrySetPropertyValue's catch-all.
        const runtime_bindings_guard guard;
        controls::window window;
        const std::string message = parse_error_message([&] {
            (void)xaml_loader::load_into(window, R"xml(
<Window xmlns="http://schemas.microsoft.com/dotnet/2021/maui" Title="{Binding text}"/>)xml");
        });
        EXPECT_EQ(message, "Cannot assign property \"Title\": Property does not exist, or is not assignable, or "
                           "mismatching type between value and property");
    }

    TEST(xaml_loader, attached_property_is_a_loud_deferral)
    {
        // LoaderTests.TestAttachedBP — dotted attached names are an M7 deferral; the load fails
        // with TrySetPropertyValue's catch-all instead of silently dropping Grid.Column.
        controls::label label;
        const std::string message = parse_error_message([&] {
            (void)xaml_loader::load_into(label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui" Grid.Column="1"/>)xml");
        });
        EXPECT_TRUE(message.contains("Cannot assign property \"Grid.Column\"")) << message;
    }

    TEST(xaml_loader, event_wiring_is_a_loud_deferral)
    {
        // LoaderTests.TestEvent — reflective event-handler lookup has no port yet; loud failure.
        controls::label label;
        const std::string message = parse_error_message([&] {
            (void)xaml_loader::load_into(label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui" Clicked="onButtonClicked"/>)xml");
        });
        EXPECT_TRUE(message.contains("Cannot assign property \"Clicked\"")) << message;
    }

    // ---- load (XamlLoader.Create): the root is minted from markup ---------------------------------

    TEST(xaml_loader, load_creates_the_root_from_markup)
    {
        const xaml_load_result result = xaml_loader::load(R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui" Text="Foo"/>)xml");
        const std::shared_ptr<controls::label> label = result.root_as<controls::label>();
        ASSERT_NE(label, nullptr);
        EXPECT_EQ(label->text(), "Foo");
    }

    TEST(xaml_loader, load_hydrates_a_whole_tree_owned_by_the_result)
    {
        xaml_load_result result = xaml_loader::load(R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
	<VerticalStackLayout>
		<Label x:Name="label0" Text="Foo"/>
		<Label x:Name="label1" Text="Bar"/>
	</VerticalStackLayout>
</ContentPage>)xml");
        const std::shared_ptr<controls::content_page> page = result.root_as<controls::content_page>();
        ASSERT_NE(page, nullptr);
        EXPECT_EQ(result.graph.size(), 4U); // page + stack + two labels
        const std::shared_ptr<controls::label> label0 = result.find_by_name<controls::label>("label0");
        ASSERT_NE(label0, nullptr);
        EXPECT_EQ(label0->text(), "Foo");
        // The result is the single owner: the weak handle dies with it (graph teardown).
        const std::weak_ptr<controls::label> weak_label = label0;
        result = xaml_load_result{};
        EXPECT_TRUE(weak_label.expired() || label0.use_count() == 1);
    }

    TEST(xaml_loader, load_rejects_a_non_control_root_loudly)
    {
        // A root that hydrates to a non-control payload (C# Create would return the
        // ResourceDictionary object; the port's control-typed result defers that loudly).
        EXPECT_EQ(parse_error_message([&] {
                      (void)xaml_loader::load(
                          R"xml(<ResourceDictionary xmlns="http://schemas.microsoft.com/dotnet/2021/maui"/>)xml");
                  }),
                  "Loading a non-control root element (ResourceDictionary) is not supported by the port yet "
                  "(STATUS.md M7 deferrals)");
    }

    TEST(xaml_loader, load_unknown_root_type_throws)
    {
        EXPECT_EQ(parse_error_message([&] {
                      (void)xaml_loader::load(
                          R"xml(<CustomView xmlns="http://schemas.microsoft.com/dotnet/2021/maui"/>)xml");
                  }),
                  "Type CustomView not found in xmlns http://schemas.microsoft.com/dotnet/2021/maui");
    }

    TEST(xaml_loader, load_with_exception_handler_returns_an_empty_result)
    {
        // XamlLoader.Create(xaml, doNotThrow: true) returns null on a failed root creation.
        std::vector<std::string> collected;
        const xaml_load_result result =
            xaml_loader::load(R"xml(<CustomView xmlns="http://schemas.microsoft.com/dotnet/2021/maui"/>)xml",
                              {.exception_handler = [&collected](const xaml_parse_exception& error) {
                                  collected.emplace_back(error.unformatted_message());
                              }});
        EXPECT_EQ(result.graph.root(), nullptr);
        ASSERT_EQ(collected.size(), 1U);
        EXPECT_EQ(collected.front(),
                  "Type CustomView not found in xmlns http://schemas.microsoft.com/dotnet/2021/maui");
    }

    // ---- MarkupExtensionTests e2e (custom extensions registered the port way) ----------------------

    // MarkupExtensionTests.AppendMarkupExtension, registered explicitly instead of resolved by
    // reflection from the test assembly.
    class append_markup_extension final : public i_markup_extension
    {
    public:
        append_markup_extension(std::string value0, std::string value1)
            : value0_(std::move(value0)), value1_(std::move(value1))
        {
        }

        [[nodiscard]] std::any provide_value(const xaml_service_provider& /*services*/) override
        {
            return std::any{value0_ + value1_};
        }

    private:
        std::string value0_;
        std::string value1_;
    };

    void register_append_markup_extension()
    {
        static const bool registered = [] {
            markup_extension_registry::instance().register_extension(
                "AppendMarkup", [](const markup_extension_arguments& args) {
                    const auto attribute = [&args](const char* name) {
                        const auto found = args.attributes.find(name);
                        return found != args.attributes.end() ? found->second : std::string{};
                    };
                    return std::make_unique<append_markup_extension>(attribute("Value0"), attribute("Value1"));
                });
            return true;
        }();
        (void)registered;
    }

    TEST(xaml_loader, markup_extension_test_in_xaml)
    {
        // MarkupExtensionTests.TestInXaml.
        register_append_markup_extension();
        controls::label label;
        (void)xaml_loader::load_into(label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
       Text="{AppendMarkup Value0=Foo, Value1=Bar}"/>)xml");
        EXPECT_EQ(label.text(), "FooBar");
    }

    TEST(xaml_loader, markup_extension_extension_suffix_spelling_resolves)
    {
        // MarkupExtensionTests.TestLookupWithSuffix's registry tolerance, through a full load.
        register_append_markup_extension();
        controls::label label;
        (void)xaml_loader::load_into(label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
       Text="{AppendMarkupExtension Value0=Fuu, Value1=Baa}"/>)xml");
        EXPECT_EQ(label.text(), "FuuBaa");
    }

    TEST(xaml_loader, markup_extension_documentation_code)
    {
        // MarkupExtensionTests.TestDocumentationCode (ColorMarkup R/G/B -> TextColor), exercising a
        // typed (non-string) provide_value result through the property registry.
        static const bool registered = [] {
            markup_extension_registry::instance().register_extension(
                "ColorMarkup", [](const markup_extension_arguments& args) {
                    struct color_markup final : i_markup_extension
                    {
                        explicit color_markup(maui::graphics::color value) : value_(value)
                        {
                        }
                        [[nodiscard]] std::any provide_value(const xaml_service_provider& /*services*/) override
                        {
                            return std::any{value_};
                        }
                        maui::graphics::color value_;
                    };
                    const auto channel = [&args](const char* name) {
                        const auto found = args.attributes.find(name);
                        return found != args.attributes.end() ? std::stoi(found->second) : 0;
                    };
                    return std::make_unique<color_markup>(
                        maui::graphics::color::from_rgb(channel("R"), channel("G"), channel("B")));
                });
            return true;
        }();
        (void)registered;
        controls::label label;
        (void)xaml_loader::load_into(label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
       TextColor="{ColorMarkup R=100, G=80, B=60}"/>)xml");
        EXPECT_EQ(label.text_color(), maui::graphics::color::from_rgb(100, 80, 60));
    }

    TEST(xaml_loader, throw_on_markup_extension_not_found)
    {
        // MarkupExtensionTests.ThrowOnMarkupExtensionNotFound, through a full load.
        controls::label label;
        EXPECT_EQ(parse_error_message([&] {
                      (void)xaml_loader::load_into(label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui" Text="{local:Missing}"/>)xml");
                  }),
                  "MarkupExtension not found for local:Missing");
    }
} // namespace
