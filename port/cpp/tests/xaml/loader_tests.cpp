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

#include "maui/core/text_alignment.hpp"
#include "maui/core/text_decorations.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"

#include "maui/controls/absolute_layout.hpp" // W10: AbsoluteLayout attached props
#include "maui/controls/application.hpp"
#include "maui/controls/bindings/i_value_converter.hpp"
#include "maui/controls/border.hpp"                        // W9: Border.StrokeShape element form
#include "maui/controls/box_view.hpp"                      // W7: element-form gradient brush target
#include "maui/controls/brushes/gradient_stop.hpp"         // W7
#include "maui/controls/brushes/linear_gradient_brush.hpp" // W7
#include "maui/controls/button.hpp"                        // W15: Button.ImageSource
#include "maui/controls/content_page.hpp"
#include "maui/controls/content_view.hpp"      // W16: ContentView.ControlTemplate
#include "maui/controls/flex_layout.hpp"       // W11: FlexLayout attached props
#include "maui/controls/font_image_source.hpp" // W17: <FontImageSource> element form
#include "maui/controls/formatted_string.hpp"  // W8: Label.FormattedText element form
#include "maui/controls/grid.hpp"
#include "maui/controls/image.hpp"                   // W17: Image.Source element form
#include "maui/controls/items/collection_view.hpp"   // W4: ItemTemplate inflation target
#include "maui/controls/items/grid_items_layout.hpp" // W14: ItemsLayout="VerticalGrid,N"
#include "maui/controls/label.hpp"
#include "maui/controls/picker.hpp" // W12: <Picker.Items> x:String child sink
#include "maui/controls/resource_dictionary.hpp"
#include "maui/controls/span.hpp"                       // W8
#include "maui/controls/templates/control_template.hpp" // W16: <ControlTemplate> element form
#include "maui/controls/templates/data_template.hpp"    // W4: data_template::create_content()
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/font_attributes.hpp" // W8
#include "maui/core/property.hpp"
#include "maui/core/type_tag.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/gradient_paint.hpp"         // W7: the bridged paint reflects the stops
#include "maui/graphics/rect.hpp"                   // W10: AbsoluteLayout.LayoutBounds
#include "maui/graphics/shapes/round_rectangle.hpp" // W9
#include "maui/layouts/absolute_layout_flags.hpp"   // W10: AbsoluteLayout.LayoutFlags
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

    TEST(xaml_loader, converted_literal_attributes_apply)
    {
        // The M7 converter-parity unit end-to-end: color / [Flags] enum / enum / thickness literals
        // convert through the now-registered default converter table during a real load.
        controls::label label;
        (void)xaml_loader::load_into(label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
       TextColor="Red" HorizontalTextAlignment="Center" Padding="1,2,3,4"
       TextDecorations="Underline,Strikethrough"/>)xml");
        EXPECT_EQ(label.text_color(), maui::graphics::colors::red);
        EXPECT_EQ(label.horizontal_text_alignment(), maui::core::text_alignment::center);
        EXPECT_EQ(label.padding(), (maui::core::thickness{1, 2, 3, 4}));
        EXPECT_EQ(std::to_underlying(label.text_decorations()),
                  std::to_underlying(maui::core::text_decorations::underline) |
                      std::to_underlying(maui::core::text_decorations::strikethrough));
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
        // LoaderTests.StyleWithoutTargetTypeThrows — now that <Style>/<Setter> are loadable (W3), a
        // <Style> with no TargetType throws just as C#'s Style ctor does (a Style MUST have a TargetType).
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
        EXPECT_EQ(message, "Style requires a TargetType");
    }

    TEST(xaml_loader, style_setters_apply_to_the_target)
    {
        // W3 positive case: an inline <Label.Style> with a <Style TargetType="Label"> whose <Setter>
        // resolves Property against the TargetType and APPLIES through the existing style runtime —
        // proving the loader wiring end to end (mint Style, thread TargetType to the Setter, set_style).
        controls::label label;
        const std::string message = parse_error_message([&] {
            (void)xaml_loader::load_into(label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<Label.Style>
		<Style TargetType="Label">
			<Setter Property="Text" Value="Styled" />
		</Style>
	</Label.Style>
</Label>)xml");
        });
        EXPECT_EQ(message, "(no xaml_parse_exception thrown)") << message;
        EXPECT_EQ(label.text(), "Styled");
    }

    // ---- W4: DataTemplate inflation + CollectionView.ItemTemplate ----------------------------------

    TEST(xaml_loader, collection_view_item_template_inflates)
    {
        // W4 positive case: an inline <CollectionView.ItemTemplate><DataTemplate><Label Text="cell"/>
        // wires the minted data_template onto the CollectionView, and create_content() inflates a fresh
        // Label carrying the body's static text — proving mint (create_values) + set_template (apply)
        // + per-item inflation (inflate_template_body) end to end.
        controls::collection_view view;
        const std::string message = parse_error_message([&] {
            (void)xaml_loader::load_into(view, R"xml(
<CollectionView xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<CollectionView.ItemTemplate>
		<DataTemplate>
			<Label Text="cell" />
		</DataTemplate>
	</CollectionView.ItemTemplate>
</CollectionView>)xml");
        });
        EXPECT_EQ(message, "(no xaml_parse_exception thrown)") << message;

        const std::shared_ptr<controls::data_template> tmpl = view.item_template();
        ASSERT_NE(tmpl, nullptr);
        EXPECT_TRUE(tmpl->has_load_template());

        const std::shared_ptr<bindable_object> content = tmpl->create_content();
        ASSERT_NE(content, nullptr);
        const auto cell = std::dynamic_pointer_cast<controls::label>(content);
        ASSERT_NE(cell, nullptr);
        EXPECT_EQ(cell->text(), "cell");
    }

    TEST(xaml_loader, grid_element_form_row_and_column_definitions)
    {
        // W2 positive case: the ELEMENT form <Grid.RowDefinitions><RowDefinition Height="Auto"/>…
        // (the markup shape real MAUI pages use when a definition carries per-row properties). The
        // <RowDefinition>/<ColumnDefinition> items mint plain value types (create_values special-case)
        // that push straight onto the grid's vectors (try_add_grid_definition), in document order —
        // the multi-child list path here, the single-child property path below. Equivalent to the
        // string form RowDefinitions="Auto,2*,48" which routes through the converter twin.
        controls::grid grid;
        const std::string message = parse_error_message([&] {
            (void)xaml_loader::load_into(grid, R"xml(
<Grid xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<Grid.RowDefinitions>
		<RowDefinition Height="Auto" />
		<RowDefinition Height="2*" />
		<RowDefinition Height="48" />
	</Grid.RowDefinitions>
	<Grid.ColumnDefinitions>
		<ColumnDefinition Width="*" />
	</Grid.ColumnDefinitions>
</Grid>)xml");
        });
        EXPECT_EQ(message, "(no xaml_parse_exception thrown)") << message;

        ASSERT_EQ(grid.row_definitions().size(), 3U);
        EXPECT_TRUE(grid.row_definitions()[0].height().is_auto());
        EXPECT_TRUE(grid.row_definitions()[1].height().is_star());
        EXPECT_DOUBLE_EQ(grid.row_definitions()[1].height().value(), 2.0);
        EXPECT_TRUE(grid.row_definitions()[2].height().is_absolute());
        EXPECT_DOUBLE_EQ(grid.row_definitions()[2].height().value(), 48.0);

        ASSERT_EQ(grid.column_definitions().size(), 1U); // single-child property path
        EXPECT_TRUE(grid.column_definitions()[0].width().is_star());
    }

    TEST(xaml_loader, picker_items_element_form_x_string_children)
    {
        // W12 positive case: the ELEMENT form <Picker.Items><x:String>…</x:String></Picker.Items>
        // (the markup shape the C# gallery PickerPage.xaml uses). Each <x:String> mints a plain
        // std::string value (the x:String primitive route), which bypasses both the registered-property
        // surface and the bindable child sink and pushes straight onto the picker's Items face
        // (try_add_picker_item), in document order. (A SelectedIndex attribute would coerce against the
        // still-empty Items — attributes apply before property-element children — so it is set in code
        // here, after load, mirroring the gallery's markup_picker.)
        controls::picker picker;
        const std::string message = parse_error_message([&] {
            (void)xaml_loader::load_into(picker, R"xml(
<Picker xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
        xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml"
        Title="Select an item">
	<Picker.Items>
		<x:String>Item 1</x:String>
		<x:String>Item 2</x:String>
		<x:String>Item 3</x:String>
	</Picker.Items>
</Picker>)xml");
        });
        EXPECT_EQ(message, "(no xaml_parse_exception thrown)") << message;

        ASSERT_EQ(picker.items().count(), 3);
        EXPECT_EQ(picker.items().at(0), "Item 1");
        EXPECT_EQ(picker.items().at(1), "Item 2");
        EXPECT_EQ(picker.items().at(2), "Item 3"); // document order preserved
        EXPECT_EQ(picker.title(), "Select an item");

        picker.set_selected_index(1); // now coerces against the populated Items (Clamp(-1, Count-1))
        EXPECT_EQ(picker.selected_index(), 1);
    }

    TEST(xaml_loader, collection_view_items_source_x_array_static_strings)
    {
        // W13: element-form <CollectionView.ItemsSource><x:Array Type="{x:Type x:String}"><x:String>…
        // populates a fixed-snapshot string ItemsSource with NO binding context — the inline-items shape
        // the gallery's items / collectionview pages need (the C# array-ItemsSource). The <x:Array>
        // create-pass gathers its already-created <x:String> children into an xaml_array;
        // try_set_items_source_from_array builds the item_collection and calls set_items_source. The
        // Type="{x:Type …}" attribute is consumed/ignored at create (the create-time-consumed pattern).
        controls::collection_view view;
        const std::string message = parse_error_message([&] {
            (void)xaml_loader::load_into(view, R"xml(
<CollectionView xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
                xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
	<CollectionView.ItemsSource>
		<x:Array Type="{x:Type x:String}">
			<x:String>Water the plants</x:String>
			<x:String>Review the port</x:String>
			<x:String>Ship wave 2</x:String>
		</x:Array>
	</CollectionView.ItemsSource>
</CollectionView>)xml");
        });
        EXPECT_EQ(message, "(no xaml_parse_exception thrown)") << message;

        const auto source = view.items_source();
        ASSERT_NE(source, nullptr);
        ASSERT_EQ(source->count(), 3U);
        EXPECT_EQ(source->at(0).text(), "Water the plants");
        EXPECT_EQ(source->at(1).text(), "Review the port");
        EXPECT_EQ(source->at(2).text(), "Ship wave 2"); // document order preserved
    }

    TEST(xaml_loader, collection_view_items_layout_string_vertical_grid)
    {
        // W14: ItemsLayout="VerticalGrid, 3" (the MAUI ItemsLayoutTypeConverter string form) sets a
        // GridItemsLayout(span=3, Vertical) — the 3-across grid the collectionview gallery page uses.
        // (The element form <GridItemsLayout Orientation=… Span=…> needs [Parameter] ctor-arg reflection
        // the port lacks, so the string form is the supported route.)
        controls::collection_view view;
        const std::string message = parse_error_message([&] {
            (void)xaml_loader::load_into(view, R"xml(
<CollectionView xmlns="http://schemas.microsoft.com/dotnet/2021/maui" ItemsLayout="VerticalGrid, 3" />)xml");
        });
        EXPECT_EQ(message, "(no xaml_parse_exception thrown)") << message;

        const auto layout = view.items_layout();
        ASSERT_NE(layout, nullptr);
        const auto grid = std::dynamic_pointer_cast<controls::grid_items_layout>(layout);
        ASSERT_NE(grid, nullptr) << "ItemsLayout did not resolve to a grid_items_layout";
        EXPECT_EQ(grid->span(), 3);
        EXPECT_EQ(grid->orientation(), controls::items_layout_orientation::vertical);
    }

    TEST(xaml_loader, button_image_source_from_file)
    {
        // W15: Button.ImageSource="settings.png" routes through convert_image_source (the i_image_source
        // converter shared with Image/ImageButton) — the icon buttons on the gallery button page.
        controls::button button;
        const std::string message = parse_error_message([&] {
            (void)xaml_loader::load_into(button, R"xml(
<Button xmlns="http://schemas.microsoft.com/dotnet/2021/maui" Text="settings" ImageSource="settings.png" />)xml");
        });
        EXPECT_EQ(message, "(no xaml_parse_exception thrown)") << message;
        EXPECT_EQ(button.text(), "settings");
        EXPECT_NE(button.image_source(), nullptr) << "ImageSource did not resolve to an i_image_source";
    }

    TEST(xaml_loader, font_image_source_element_form)
    {
        // W17: <Image.Source><FontImageSource Glyph=… FontFamily=… Size=… Color=…> — the element form the
        // real C# ImagePage uses. The ctor-only font_image_source is consumed at create (like
        // <RoundRectangle>) + boxed as shared_ptr<i_image_source> for Image.Source's exact type. FontFamily
        // + Size compose onto the font. The Image co-owns the source via set_source, so source() stays valid.
        controls::image img;
        const std::string message = parse_error_message([&] {
            (void)xaml_loader::load_into(img, R"xml(
<Image xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<Image.Source>
		<FontImageSource Glyph="&#xf30c;" FontFamily="Ionicons" Size="20" Color="White" />
	</Image.Source>
</Image>)xml");
        });
        EXPECT_EQ(message, "(no xaml_parse_exception thrown)") << message;

        const auto* fis = dynamic_cast<const controls::font_image_source*>(img.source());
        ASSERT_NE(fis, nullptr) << "Image.Source did not resolve to a font_image_source";
        EXPECT_FALSE(fis->glyph().empty()) << "Glyph entity (&#xf30c;) did not decode onto the source";
        EXPECT_EQ(fis->font().family(), "Ionicons");
        EXPECT_DOUBLE_EQ(fis->font().size(), 20.0);
    }

    TEST(xaml_loader, control_template_element_form_mints_and_inflates)
    {
        // W16: <ContentView.ControlTemplate><ControlTemplate>… is minted as a control_template (the
        // DataTemplate sibling), routed to set_control_template, and its body loader inflates the template
        // root on create_content(). <ContentPresenter/> inside the body is a registered view. This is the
        // loader path that backs the radio_template_from_style page (ControlTemplate-from-Style).
        //
        // OWNERSHIP (PROFILE §8): the load result OWNS the inflated graph incl. the developer Content that
        // the stamped ContentPresenter references, so it must OUTLIVE the root `view` — hence `result` is
        // declared FIRST (destructs last), exactly as the gallery's xaml_twin_holder orders result_/page_.
        // (A result destructed before the root frees that content under the live presenter → use-after-free.)
        xaml_load_result result;
        controls::content_view view;
        const std::string message = parse_error_message([&] {
            result = xaml_loader::load_into(view, R"xml(
<ContentView xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
	<ContentView.ControlTemplate>
		<ControlTemplate>
			<Border>
				<Grid>
					<Label Text="tile" />
					<ContentPresenter />
				</Grid>
			</Border>
		</ControlTemplate>
	</ContentView.ControlTemplate>
	<Label Text="A" />
</ContentView>)xml");
        });
        EXPECT_EQ(message, "(no xaml_parse_exception thrown)") << message;

        const std::shared_ptr<controls::control_template> tmpl = view.control_template();
        ASSERT_NE(tmpl, nullptr) << "ControlTemplate was not minted/assigned";
        // The body loader inflates the template root (the <Border>) on demand.
        const auto root = std::dynamic_pointer_cast<controls::border>(tmpl->create_content());
        ASSERT_NE(root, nullptr) << "ControlTemplate body did not inflate to its Border root";
    }

    TEST(xaml_loader, font_size_numeric_and_named_compose_onto_the_font)
    {
        // W6: FontSize is registered on every set_font control, parsing BOTH a numeric literal and a
        // NamedSize alias, composing onto the port's single font value via font().with_size().
        controls::label numeric;
        (void)xaml_loader::load_into(numeric, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui" Text="A" FontSize="22" />)xml");
        EXPECT_DOUBLE_EQ(numeric.font().size(), 22.0);

        controls::label named; // a NamedSize alias must resolve through the same entry (not just numbers)
        const std::string message = parse_error_message([&] {
            (void)xaml_loader::load_into(named, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui" Text="A" FontSize="Large" />)xml");
        });
        EXPECT_EQ(message, "(no xaml_parse_exception thrown)") << message;
        EXPECT_GT(named.font().size(), 0.0);
    }

    TEST(xaml_loader, gallery_controls_stack_authored_as_plain_content_page)
    {
        // Phase-4 pilot: a real gallery page (controls_stack — the maui-compare reference demo) authored
        // as a PLAIN ContentPage on the now-supported loader surface. This is the achievable path to
        // gallery parity (the raw sample files are views:BasePage-rooted and cannot load as-is). It
        // exercises 13 controls in one tree plus FontSize + FontAttributes, end to end.
        controls::content_page page;
        // Hold the load result: it OWNS the created object graph (PROFILE §8 — the tree-wiring APIs are
        // non-owning), so discarding it would free the whole content tree before we inspect it.
        const xaml_load_result result = xaml_loader::load_into(page, R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml" Title="Controls">
    <VerticalStackLayout Spacing="12" Padding="16">
        <Label Text="Controls" FontSize="22" FontAttributes="Bold" />
        <Button Text="A Button" />
        <Entry Placeholder="An Entry" />
        <Editor Placeholder="An Editor" HeightRequest="60" />
        <SearchBar Placeholder="A SearchBar" />
        <HorizontalStackLayout Spacing="12">
            <CheckBox IsChecked="True" />
            <Switch IsToggled="True" />
            <ActivityIndicator IsRunning="True" />
        </HorizontalStackLayout>
        <Slider Minimum="0" Maximum="100" Value="40" />
        <Stepper Minimum="0" Maximum="10" Value="3" />
        <ProgressBar Progress="0.6" />
    </VerticalStackLayout>
</ContentPage>)xml");
        EXPECT_TRUE(result.warnings.empty());

        auto* stack = dynamic_cast<controls::vertical_stack_layout*>(page.content());
        ASSERT_NE(stack, nullptr);
        EXPECT_EQ(stack->count(),
                  9); // header + button + entry + editor + searchbar + row + slider + stepper + progress

        // The header's FontAttributes="Bold" folded onto the font's weight (end-to-end W6).
        auto* header = dynamic_cast<controls::label*>(&stack->at(0));
        ASSERT_NE(header, nullptr);
        EXPECT_EQ(header->text(), "Controls");
        EXPECT_DOUBLE_EQ(header->font().size(), 22.0);
        EXPECT_EQ(header->font().weight(), maui::core::font_weight::bold);
    }

    TEST(xaml_loader, element_form_linear_gradient_brush_background)
    {
        // W7: the ELEMENT form <BoxView.Background><LinearGradientBrush><GradientStop…/></…> — the brush
        // is created (register_type), its EndPoint + GradientStop children apply, and the created brush
        // reaches the shared_ptr<brush> Background property via the apply object-coercion.
        controls::box_view box;
        const xaml_load_result result = xaml_loader::load_into(box, R"xml(
<BoxView xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
    <BoxView.Background>
        <LinearGradientBrush EndPoint="1,0">
            <GradientStop Color="Yellow" Offset="0.1" />
            <GradientStop Color="Green" Offset="1.0" />
        </LinearGradientBrush>
    </BoxView.Background>
</BoxView>)xml");

        const auto brush = std::dynamic_pointer_cast<controls::linear_gradient_brush>(box.background_brush());
        ASSERT_NE(brush, nullptr) << "Background did not resolve to a LinearGradientBrush";
        EXPECT_EQ(brush->end_point(), (maui::graphics::point{1.0, 0.0}));
        ASSERT_EQ(brush->gradient_stops().count(), 2U);
        EXPECT_EQ(brush->gradient_stops()[0]->color(), maui::graphics::colors::yellow);
        EXPECT_FLOAT_EQ(brush->gradient_stops()[0]->offset(), 0.1F);
        EXPECT_EQ(brush->gradient_stops()[1]->color(), maui::graphics::colors::green);

        // The bridged PAINT (what the native layer renders) must reflect the stops — proving the view's
        // re-derive-on-invalidate fix: the loader adds the GradientStops AFTER set_background_brush cached
        // the paint, so without the subscription this paint would be empty (the iOS black-render bug).
        const auto* paint = dynamic_cast<const maui::graphics::gradient_paint*>(box.background());
        ASSERT_NE(paint, nullptr) << "Background paint is not a gradient (stops not bridged)";
        EXPECT_EQ(paint->gradient_stops().size(), 2U);
    }

    TEST(xaml_loader, element_form_formatted_string)
    {
        // W8: <Label.FormattedText><FormattedString><Span …/> — FormattedString + Span are created, the
        // spans apply their attributes, the spans child-sink fills the collection, and the created
        // formatted_string reaches Label.FormattedText via the object-coercion. (label::set_formatted_text
        // subscribes to the formatted_string's changed signal, so loader-added spans rebuild the text.)
        controls::label label;
        const xaml_load_result result = xaml_loader::load_into(label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
    <Label.FormattedText>
        <FormattedString>
            <Span Text="Bold red" TextColor="Red" FontAttributes="Bold" />
            <Span Text=" underlined" TextDecorations="Underline" />
        </FormattedString>
    </Label.FormattedText>
</Label>)xml");

        const std::shared_ptr<controls::formatted_string> formatted = label.formatted_text();
        ASSERT_NE(formatted, nullptr) << "FormattedText did not resolve to a FormattedString";
        ASSERT_EQ(formatted->spans().size(), 2U);
        EXPECT_EQ(formatted->spans()[0]->text(), "Bold red");
        EXPECT_EQ(formatted->spans()[0]->text_color(), maui::graphics::colors::red);
        EXPECT_EQ(formatted->spans()[0]->font_attributes(), maui::core::font_attributes::bold);
        EXPECT_EQ(formatted->spans()[1]->text(), " underlined");
        EXPECT_EQ(formatted->spans()[1]->text_decorations(), maui::core::text_decorations::underline);
    }

    TEST(xaml_loader, flex_layout_attached_grow_basis)
    {
        // W11: FlexLayout.Grow / Basis per-child attached props — set on the child, deferred until parented
        // into the FlexLayout, then applied via set_grow/set_basis (the W10 playbook for layout-distribution).
        controls::content_page page;
        const xaml_load_result result = xaml_loader::load_into(page, R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
    <FlexLayout>
        <Label Text="A" FlexLayout.Grow="1" />
        <Label Text="B" FlexLayout.Basis="50" />
    </FlexLayout>
</ContentPage>)xml");
        auto* flex = dynamic_cast<controls::flex_layout*>(page.content());
        ASSERT_NE(flex, nullptr);
        ASSERT_EQ(flex->count(), 2);
        EXPECT_FLOAT_EQ(flex->get_grow(flex->at(0)), 1.0F);
        EXPECT_FLOAT_EQ(flex->get_grow(flex->at(1)), 0.0F); // B has no Grow → per-child, not global
        EXPECT_FLOAT_EQ(flex->get_basis(flex->at(1)).length(), 50.0F);
    }

    TEST(xaml_loader, absolute_layout_attached_bounds_and_flags)
    {
        // W10: AbsoluteLayout.LayoutBounds (Rect) + LayoutFlags ([Flags] enum) attached properties — set on
        // the child, deferred until it's parented into the AbsoluteLayout, then applied via set_layout_bounds/
        // set_layout_flags (mirrors the Grid.Row deferred-placement path).
        controls::content_page page;
        const xaml_load_result result = xaml_loader::load_into(page, R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
    <AbsoluteLayout>
        <BoxView Color="Blue" AbsoluteLayout.LayoutBounds="0.5,0,100,25"
                 AbsoluteLayout.LayoutFlags="PositionProportional" />
    </AbsoluteLayout>
</ContentPage>)xml");
        auto* layout = dynamic_cast<controls::absolute_layout*>(page.content());
        ASSERT_NE(layout, nullptr);
        ASSERT_EQ(layout->count(), 1);
        const maui::core::i_view& child = layout->at(0);
        EXPECT_EQ(layout->get_layout_bounds(child), (maui::graphics::rect{0.5, 0.0, 100.0, 25.0}));
        EXPECT_EQ(layout->get_layout_flags(child), maui::layouts::absolute_layout_flags::position_proportional);
    }

    TEST(xaml_loader, element_form_border_stroke_shape)
    {
        // W9: <Border.StrokeShape> in element form. RoundRectangle is minted from CornerRadius (no controls
        // equivalent → graphics::round_rectangle, boxed as i_shape); Ellipse is a register_type'd controls
        // shape that reaches the shared_ptr<i_shape> StrokeShape via the apply object-coercion.
        controls::border rounded;
        (void)xaml_loader::load_into(rounded, R"xml(
<Border xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
    <Border.StrokeShape><RoundRectangle CornerRadius="20" /></Border.StrokeShape>
</Border>)xml");
        const auto round_rect =
            std::dynamic_pointer_cast<maui::graphics::shapes::round_rectangle>(rounded.stroke_shape());
        ASSERT_NE(round_rect, nullptr) << "RoundRectangle StrokeShape not minted";

        controls::border circle;
        const std::string message = parse_error_message([&] {
            (void)xaml_loader::load_into(circle, R"xml(
<Border xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
    <Border.StrokeShape><Ellipse /></Border.StrokeShape>
</Border>)xml");
        });
        EXPECT_EQ(message, "(no xaml_parse_exception thrown)") << message;
        EXPECT_NE(circle.stroke_shape(), nullptr) << "Ellipse StrokeShape not coerced to i_shape";
    }

    TEST(xaml_loader, collection_view_structured_and_grouping_properties)
    {
        // W6 positive case: the common CollectionView surface beyond ItemTemplate — Header/Footer string
        // literals (boxed into the items machinery's object stand-in), IsGrouped (groupable_items_view),
        // and ItemSizingStrategy (the new enum converter). These cleared the "Cannot assign property"
        // bucket for the HeaderFooter / Grouping / ItemSizing gallery pages.
        controls::collection_view view;
        const std::string message = parse_error_message([&] {
            (void)xaml_loader::load_into(view, R"xml(
<CollectionView xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
                Header="Top" Footer="Bottom" IsGrouped="true" ItemSizingStrategy="MeasureFirstItem" />)xml");
        });
        EXPECT_EQ(message, "(no xaml_parse_exception thrown)") << message;
        EXPECT_EQ(view.header().text(), "Top");
        EXPECT_EQ(view.footer().text(), "Bottom");
        EXPECT_TRUE(view.is_grouped());
        EXPECT_EQ(view.item_sizing_strategy(), controls::item_sizing_strategy::measure_first_item);
    }

    TEST(xaml_loader, collection_view_empty_view_string_form)
    {
        // EmptyView string form: the attribute literal boxes into the items machinery's object
        // stand-in (C# assigns the string to the object-typed ItemsView.EmptyView; the handler
        // renders its ToString). Mirrors the Header/Footer boxed-string route above.
        controls::collection_view view;
        const std::string message = parse_error_message([&] {
            (void)xaml_loader::load_into(view, R"xml(
<CollectionView xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
                EmptyView="No items match your filter." />)xml");
        });
        EXPECT_EQ(message, "(no xaml_parse_exception thrown)") << message;
        ASSERT_TRUE(view.empty_view().has_value());
        EXPECT_EQ(view.empty_view().text(), "No items match your filter.");
        EXPECT_EQ(view.empty_view().as_bindable(), nullptr); // a string, not a hosted view
    }

    TEST(xaml_loader, collection_view_empty_view_element_text_form)
    {
        // EmptyView property-element TEXT form — the spelling the original C# galleries use
        // (<CollectionView.EmptyView>Items loading simulation...</CollectionView.EmptyView>): the
        // element text reaches the same registration as the attribute literal.
        controls::collection_view view;
        const std::string message = parse_error_message([&] {
            (void)xaml_loader::load_into(view, R"xml(
<CollectionView xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<CollectionView.EmptyView>Items loading simulation...</CollectionView.EmptyView>
</CollectionView>)xml");
        });
        EXPECT_EQ(message, "(no xaml_parse_exception thrown)") << message;
        ASSERT_TRUE(view.empty_view().has_value());
        EXPECT_EQ(view.empty_view().text(), "Items loading simulation...");
        EXPECT_EQ(view.empty_view().as_bindable(), nullptr);
    }

    TEST(xaml_loader, collection_view_empty_view_element_form)
    {
        // EmptyView element form: the created element coerces to a view and boxes with reference
        // semantics — C#'s `EmptyView is View` direct-hosting path (the handler hosts the instance).
        controls::collection_view view;
        const std::string message = parse_error_message([&] {
            (void)xaml_loader::load_into(view, R"xml(
<CollectionView xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<CollectionView.EmptyView>
		<Label Text="No results matched your filter." />
	</CollectionView.EmptyView>
</CollectionView>)xml");
        });
        EXPECT_EQ(message, "(no xaml_parse_exception thrown)") << message;
        ASSERT_TRUE(view.empty_view().has_value());
        const std::shared_ptr<maui::core::bindable_object> hosted = view.empty_view().as_bindable();
        ASSERT_NE(hosted, nullptr);
        const auto label = std::dynamic_pointer_cast<controls::label>(hosted);
        ASSERT_NE(label, nullptr);
        EXPECT_EQ(label->text(), "No results matched your filter.");
    }

    TEST(xaml_loader, data_template_create_content_yields_distinct_stamps)
    {
        // Per-stamp independence (W4 risk #1/#4): the loader re-clones + re-inflates per call, so two
        // create_content() calls return DISTINCT subtree roots (different pointers), each a live Label.
        controls::collection_view view;
        (void)xaml_loader::load_into(view, R"xml(
<CollectionView xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<CollectionView.ItemTemplate>
		<DataTemplate>
			<Label Text="cell" />
		</DataTemplate>
	</CollectionView.ItemTemplate>
</CollectionView>)xml");

        const std::shared_ptr<controls::data_template> tmpl = view.item_template();
        ASSERT_NE(tmpl, nullptr);
        const std::shared_ptr<bindable_object> first = tmpl->create_content();
        const std::shared_ptr<bindable_object> second = tmpl->create_content();
        ASSERT_NE(first, nullptr);
        ASSERT_NE(second, nullptr);
        EXPECT_NE(first.get(), second.get()); // two independent stamps
        // Both outlive the inflate call (the aliasing shared_ptr owns each stamp's graph): the text is
        // still readable after the second stamp exists (no use-after-free / premature teardown).
        EXPECT_EQ(std::dynamic_pointer_cast<controls::label>(first)->text(), "cell");
        EXPECT_EQ(std::dynamic_pointer_cast<controls::label>(second)->text(), "cell");
    }

    TEST(xaml_loader, empty_data_template_falls_back_to_label)
    {
        // C# LoaderTests.TestEmptyTemplate: a <DataTemplate/> with NO body leaves the loader unset, so
        // create_content() returns the element_template Label fallback (non-null) rather than throwing.
        controls::collection_view view;
        const std::string message = parse_error_message([&] {
            (void)xaml_loader::load_into(view, R"xml(
<CollectionView xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<CollectionView.ItemTemplate>
		<DataTemplate />
	</CollectionView.ItemTemplate>
</CollectionView>)xml");
        });
        EXPECT_EQ(message, "(no xaml_parse_exception thrown)") << message;

        const std::shared_ptr<controls::data_template> tmpl = view.item_template();
        ASSERT_NE(tmpl, nullptr);
        EXPECT_FALSE(tmpl->has_load_template());
        EXPECT_NE(tmpl->create_content(), nullptr); // the Label fallback
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

    TEST(xaml_loader, item_template_body_binding_resolves_per_stamp)
    {
        // Per-item binding (W4): a {Binding} inside the DataTemplate body re-evaluates against each
        // stamp's own BindingContext — set the realized root's context to a VM and the bound Text
        // reflects it, exactly the seam the CollectionView consumer uses to push the data item into a
        // cell. (Placed after the runtime-binding helpers so labeltext_view_model is in scope.)
        const runtime_bindings_guard guard;
        controls::collection_view view;
        (void)xaml_loader::load_into(view, R"xml(
<CollectionView xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<CollectionView.ItemTemplate>
		<DataTemplate>
			<Label Text="{Binding Path=labeltext}" />
		</DataTemplate>
	</CollectionView.ItemTemplate>
</CollectionView>)xml");

        const std::shared_ptr<controls::data_template> tmpl = view.item_template();
        ASSERT_NE(tmpl, nullptr);
        const auto cell = std::dynamic_pointer_cast<controls::label>(tmpl->create_content());
        ASSERT_NE(cell, nullptr);
        EXPECT_EQ(cell->text(), ""); // Label.TextProperty default until a context arrives

        auto context = std::make_shared<labeltext_view_model>();
        context->labeltext.set("Item 1");
        cell->set_binding_context(context);
        EXPECT_EQ(cell->text(), "Item 1");
    }

    TEST(xaml_loader, item_template_self_binding_resolves_string_item)
    {
        // W13 render path: a {Binding .} (self path) cell template binds to a STRING item — the shape an
        // x:Array string ItemsSource needs (the gallery items/collectionview pages model each item AS its
        // caption string). The collection_view handler pushes each item via set_binding_context_box(item
        // .context_box()); here we drive that seam directly with a boxed string and assert the self-binding
        // resolves current.boxed → Label.Text. This is the per-cell primitive the binding render path relies
        // on; verifying it before the gallery/corpus register_runtime_bindings integration.
        const runtime_bindings_guard guard;
        controls::collection_view view;
        (void)xaml_loader::load_into(view, R"xml(
<CollectionView xmlns="http://schemas.microsoft.com/dotnet/2021/maui">
	<CollectionView.ItemTemplate>
		<DataTemplate>
			<Label Text="{Binding .}" />
		</DataTemplate>
	</CollectionView.ItemTemplate>
</CollectionView>)xml");

        const std::shared_ptr<controls::data_template> tmpl = view.item_template();
        ASSERT_NE(tmpl, nullptr);
        const auto cell = std::dynamic_pointer_cast<controls::label>(tmpl->create_content());
        ASSERT_NE(cell, nullptr);
        EXPECT_EQ(cell->text(), ""); // default until a context arrives

        // The handler's per-stamp seam: BindingContext = the (boxed) string item.
        cell->set_binding_context_box(controls::boxed_item::of(std::string{"Water the plants"}).context_box());
        EXPECT_EQ(cell->text(), "Water the plants");
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

    TEST(xaml_loader, attached_property_outside_a_grid_is_accepted_and_ignored)
    {
        // LoaderTests.TestAttachedBP — a Grid.* attached property is valid on ANY element (Grid.SetColumn).
        // When the element is NOT a child of a Grid there is nowhere to store the placement (PROFILE §7 keeps
        // it in the parent grid's per-child side-map, not a per-element bag), so the port parses the attribute
        // and then ignores it — the load SUCCEEDS, and the rest of the markup still applies. (Placement inside
        // a real Grid is covered by attached_properties_place_grid_children below + the codegen parity tests.)
        controls::label label;
        const std::string message = parse_error_message([&] {
            (void)xaml_loader::load_into(label, R"xml(
<Label xmlns="http://schemas.microsoft.com/dotnet/2021/maui" Grid.Column="1" Text="Foo"/>)xml");
        });
        EXPECT_EQ(message, "(no xaml_parse_exception thrown)") << message;
        EXPECT_EQ(label.text(), "Foo");
    }

    TEST(xaml_loader, attached_properties_place_grid_children)
    {
        // Grid.Row/Grid.Column on a grid's children place them in the grid's per-child side-map. The value is
        // parsed at the child's line but the PLACEMENT is deferred until the apply pass has parented every
        // child (a child's attached attribute is applied before add() parents it) — the loader drains the
        // deferred closures, then get_row/get_column read back the placement.
        xaml_load_result result = xaml_loader::load(R"xml(
<Grid xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
      xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
    <Label Grid.Row="0" Grid.Column="0" Text="a"/>
    <Label Grid.Row="1" Grid.Column="2" Text="b"/>
</Grid>)xml");
        const std::shared_ptr<controls::grid> grid = result.root_as<controls::grid>();
        ASSERT_NE(grid, nullptr);
        ASSERT_EQ(grid->count(), 2);
        EXPECT_EQ(grid->get_row(grid->at(0)), 0);
        EXPECT_EQ(grid->get_column(grid->at(0)), 0);
        EXPECT_EQ(grid->get_row(grid->at(1)), 1);
        EXPECT_EQ(grid->get_column(grid->at(1)), 2);
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
