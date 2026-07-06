// Tests for the M7 wave-1 XAML registries (maui::xaml) — the reflection-free replacements for
// XamlParser.GetElementType + Activator.CreateInstance (type registry), ApplyPropertiesVisitor's
// property lookup/SetValue + [ContentProperty] (property registry), and TypeConversionExtensions'
// built-in conversions (converter registry), plus the xaml_object_graph ownership container.
// Behavior derived from src/Controls/src/Xaml/{XamlParser,CreateValuesVisitor,ApplyPropertiesVisitor}
// .cs and src/Controls/src/Core/Xaml/TypeConversionExtensions.cs.
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_object_graph.hpp"
#include "maui/xaml/xaml_parse_exception.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_standard_types.hpp"
#include "maui/xaml/xaml_type_registry.hpp"

#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "maui/controls/border.hpp" // StrokeShape — a still-deferred (no-converter) property example
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/navigation_page.hpp"
#include "maui/controls/setter.hpp"
#include "maui/controls/style.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/type_tag.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/i_shape.hpp" // Border.StrokeShape value type — a still-deferred no-converter case
#include <gtest/gtest.h>

namespace
{
    using maui::core::type_tag;
    using maui::xaml::xaml_converter_registry;
    using maui::xaml::xaml_namespace;
    using maui::xaml::xaml_object_graph;
    using maui::xaml::xaml_parse_exception;
    using maui::xaml::xaml_property_registry;
    using maui::xaml::xaml_type_registry;
    namespace controls = maui::controls;

    // A fully-registered registry trio, as the M7 loader will hold them.
    struct registries
    {
        xaml_type_registry types;
        xaml_property_registry properties;
        xaml_converter_registry converters;

        registries()
        {
            maui::xaml::register_standard_xaml(types, properties, converters);
        }
    };

    // ---- xaml_type_registry -------------------------------------------------------------------------

    TEST(xaml_type_registry, create_by_name_returns_a_working_control)
    {
        const registries reg;

        const auto object = reg.types.create("Button");
        ASSERT_NE(object, nullptr);

        // The created object IS a button (the registration's type_tag agrees), and it works.
        const auto button = std::dynamic_pointer_cast<controls::button>(object);
        ASSERT_NE(button, nullptr);
        button->set_text("Hi");
        EXPECT_EQ(button->text(), "Hi");
    }

    TEST(xaml_type_registry, find_exposes_the_concrete_type_tag)
    {
        const registries reg;

        const auto* entry = reg.types.find("Label");
        ASSERT_NE(entry, nullptr);
        EXPECT_EQ(entry->type, type_tag::of<controls::label>());
        EXPECT_NE(entry->type, type_tag::of<controls::button>());
    }

    TEST(xaml_type_registry, the_v1_control_set_is_registered)
    {
        const registries reg;
        for (const auto* name : {"Button", "Label", "Entry", "Image", "VerticalStackLayout", "HorizontalStackLayout",
                                 "Grid", "ContentPage", "NavigationPage", "Window"})
        {
            EXPECT_TRUE(reg.types.is_registered(name)) << name;
            EXPECT_NE(reg.types.create(name), nullptr) << name;
        }
    }

    // Tables: the TableView content hierarchy + renderable cell family all register.
    TEST(xaml_type_registry, the_table_view_hierarchy_is_registered)
    {
        const registries reg;
        for (const auto* name :
             {"TableView", "TableRoot", "TableSection", "TextCell", "EntryCell", "SwitchCell", "ImageCell", "ViewCell"})
        {
            EXPECT_TRUE(reg.types.is_registered(name)) << name;
            EXPECT_NE(reg.types.create(name), nullptr) << name;
        }
    }

    // XamlParser.GetElementType convention: a miss is null + an out exception, never a throw — the
    // LOADER throws. The registry is the throw-free half.
    TEST(xaml_type_registry, unknown_type_returns_null_without_throwing)
    {
        const registries reg;
        // A name that is NOT a registered control element (Slider/etc. are now registered — see the
        // register_xaml_<group> TUs — so use a guaranteed-absent type for the "unknown" assertion).
        EXPECT_EQ(reg.types.find("NotARegisteredControl"), nullptr);
        EXPECT_EQ(reg.types.create("NotARegisteredControl"), nullptr);
        EXPECT_FALSE(reg.types.is_registered("NotARegisteredControl"));
        // Namespaces are separate keyspaces: "Button" lives in the default maui xmlns, not in x.
        EXPECT_EQ(reg.types.find("Button", xaml_namespace::x), nullptr);
    }

    // ---- self-registration (the MAUI_REGISTER_HANDLER pattern) --------------------------------------

    class probe_control : public maui::core::bindable_object
    {
    };

    TEST(xaml_type_registry, macro_self_registers_into_the_default_registry)
    {
        auto& registry = maui::xaml::default_xaml_type_registry();
        ASSERT_TRUE(registry.is_registered("XamlRegistryProbe"));
        const auto object = registry.create("XamlRegistryProbe");
        ASSERT_NE(object, nullptr);
        EXPECT_NE(std::dynamic_pointer_cast<probe_control>(object), nullptr);
    }

    // ---- xaml_property_registry: the apply_setter (bindable) route -----------------------------------

    TEST(xaml_property_registry, sets_text_on_a_button_through_the_registry)
    {
        const registries reg;
        controls::button target;

        EXPECT_TRUE(reg.properties.try_set(type_tag::of<controls::button>(), target, "Text",
                                           std::any{std::string{"Click me"}}));
        EXPECT_EQ(target.text(), "Click me");
    }

    // C# TrySetValue routes through the plain BindableObject.SetValue — ManualValueSetter — so a
    // XAML-set value outranks a style and clearing the manual slot falls back to the style beneath.
    TEST(xaml_property_registry, bindable_route_applies_at_manual_specificity)
    {
        const registries reg;
        controls::button target;

        controls::style sheet = controls::style::of<controls::button>();
        sheet.add(controls::setter::of(controls::button::text_property(), std::string{"Styled"}));
        sheet.apply(target);
        EXPECT_EQ(target.text(), "Styled");

        ASSERT_TRUE(reg.properties.try_set(type_tag::of<controls::button>(), target, "Text",
                                           std::any{std::string{"From XAML"}}));
        EXPECT_EQ(target.text(), "From XAML"); // manual outranks the style…

        target.clear_setter("text", maui::core::setter_specificity::manual_value_setter);
        EXPECT_EQ(target.text(), "Styled"); // …and the style value still sits beneath it
    }

    TEST(xaml_property_registry, unknown_property_returns_false_without_throwing)
    {
        const registries reg;
        controls::button target;

        EXPECT_FALSE(reg.properties.try_set(type_tag::of<controls::button>(), target, "NoSuchProperty",
                                            std::any{std::string{"x"}}));
        EXPECT_EQ(reg.properties.find(type_tag::of<controls::button>(), "NoSuchProperty"), nullptr);
    }

    TEST(xaml_property_registry, unknown_type_returns_false_without_throwing)
    {
        const registries reg;
        controls::button target;

        EXPECT_FALSE(reg.properties.try_set(type_tag::of<int>(), target, "Text", std::any{std::string{"x"}}));
        EXPECT_EQ(reg.properties.content_property(type_tag::of<int>()), nullptr);
    }

    // C# TrySetValue pre-checks ReturnType.IsInstanceOfType and reports false on a mismatch instead of
    // throwing; the port's setters do the same for the boxed std::any.
    TEST(xaml_property_registry, mismatched_value_type_returns_false_without_throwing)
    {
        const registries reg;
        controls::button target;
        target.set_text("untouched");

        EXPECT_FALSE(reg.properties.try_set(type_tag::of<controls::button>(), target, "Text", std::any{42}));
        EXPECT_EQ(target.text(), "untouched");
    }

    // ---- xaml_property_registry: the explicit (non-bindable) route -----------------------------------

    TEST(xaml_property_registry, sets_a_non_bindable_member_through_a_typed_lambda)
    {
        const registries reg;
        controls::window host;

        EXPECT_TRUE(
            reg.properties.try_set(type_tag::of<controls::window>(), host, "Title", std::any{std::string{"Main"}}));
        EXPECT_EQ(host.title(), "Main");

        // The bindable geometry quartet rides the apply_setter route on the same control.
        EXPECT_TRUE(reg.properties.try_set(type_tag::of<controls::window>(), host, "Width", std::any{640.0}));
        EXPECT_DOUBLE_EQ(host.width(), 640.0);
    }

    TEST(xaml_property_registry, typed_route_rejects_a_wrong_target_object)
    {
        const registries reg;
        controls::button not_a_window;

        // "Title" is registered for window with a window&-typed lambda; a button target downcast fails.
        EXPECT_FALSE(reg.properties.try_set(type_tag::of<controls::window>(), not_a_window, "Title",
                                            std::any{std::string{"x"}}));
    }

    // The generic view surface is flattened into every control's registration (C# finds inherited
    // bindables via FlattenHierarchy; the explicit port registers them per concrete type).
    TEST(xaml_property_registry, shared_view_surface_is_registered_per_control)
    {
        const registries reg;
        controls::entry target;

        EXPECT_TRUE(reg.properties.try_set(type_tag::of<controls::entry>(), target, "IsEnabled", std::any{false}));
        EXPECT_FALSE(target.is_enabled());
        EXPECT_TRUE(reg.properties.try_set(type_tag::of<controls::entry>(), target, "Opacity", std::any{0.25}));
        EXPECT_DOUBLE_EQ(target.opacity(), 0.25);
        // IsVisible maps per VisibilityExtensions.ToVisibility: false → Collapsed.
        EXPECT_TRUE(reg.properties.try_set(type_tag::of<controls::entry>(), target, "IsVisible", std::any{false}));
        EXPECT_EQ(target.visibility(), maui::core::visibility::collapsed);
    }

    // ---- [ContentProperty] metadata -------------------------------------------------------------------

    TEST(xaml_property_registry, label_names_text_as_its_value_content_property)
    {
        const registries reg;

        const std::string* name = reg.properties.content_property(type_tag::of<controls::label>());
        ASSERT_NE(name, nullptr);
        EXPECT_EQ(*name, "Text"); // Label.cs [ContentProperty(nameof(Text))]
        // Button carries no [ContentProperty] in C# — null, like GetContentPropertyName.
        EXPECT_EQ(reg.properties.content_property(type_tag::of<controls::button>()), nullptr);
    }

    TEST(xaml_property_registry, add_child_hosts_content_on_a_content_page)
    {
        const registries reg;
        controls::content_page page;
        controls::button child;

        ASSERT_TRUE(reg.properties.try_add_child(type_tag::of<controls::content_page>(), page, child));
        EXPECT_EQ(page.content(), &child);
    }

    TEST(xaml_property_registry, add_child_appends_to_a_stack_layout)
    {
        const registries reg;
        controls::vertical_stack_layout stack;
        controls::button first;
        controls::label second;

        ASSERT_TRUE(reg.properties.try_add_child(type_tag::of<controls::vertical_stack_layout>(), stack, first));
        ASSERT_TRUE(reg.properties.try_add_child(type_tag::of<controls::vertical_stack_layout>(), stack, second));
        ASSERT_EQ(stack.count(), 2);
        EXPECT_EQ(&stack.at(0), static_cast<maui::core::i_view*>(&first));
        EXPECT_EQ(&stack.at(1), static_cast<maui::core::i_view*>(&second));
    }

    TEST(xaml_property_registry, add_child_pushes_the_navigation_root)
    {
        const registries reg;
        controls::content_page root; // outlives the nav whose internal tracker subscribes to it (§8)
        controls::navigation_page navigation;

        ASSERT_TRUE(reg.properties.try_add_child(type_tag::of<controls::navigation_page>(), navigation, root));
        EXPECT_EQ(navigation.current_page(), &root);
        EXPECT_TRUE(root.has_appeared());
    }

    TEST(xaml_property_registry, add_child_hosts_a_page_on_a_window)
    {
        const registries reg;
        controls::content_page page; // outlives the hosting window's chrome subscriptions (§8)
        controls::window host;

        ASSERT_TRUE(reg.properties.try_add_child(type_tag::of<controls::window>(), host, page));
        EXPECT_EQ(host.content_element(), &page);
    }

    TEST(xaml_property_registry, add_child_rejects_a_child_the_container_cannot_host)
    {
        const registries reg;
        controls::content_page page;
        controls::window not_a_view; // a window is a bindable_object but not an i_view

        EXPECT_FALSE(reg.properties.try_add_child(type_tag::of<controls::content_page>(), page, not_a_view));
        EXPECT_EQ(page.content(), nullptr);

        // NavigationPage only hosts pages — a button child is rejected.
        controls::navigation_page navigation;
        controls::button not_a_page;
        EXPECT_FALSE(reg.properties.try_add_child(type_tag::of<controls::navigation_page>(), navigation, not_a_page));
        EXPECT_EQ(navigation.current_page(), nullptr);

        // A leaf control has no child sink at all.
        controls::button leaf;
        controls::label child;
        EXPECT_FALSE(reg.properties.try_add_child(type_tag::of<controls::button>(), leaf, child));
    }

    // ---- xaml_converter_registry: the built-ins (TypeConversionExtensions invariant behavior) --------

    TEST(xaml_converter_registry, built_in_conversions_follow_the_invariant_parse_rules)
    {
        const registries reg;

        // Int32.Parse(InvariantCulture): trimmed, one optional sign.
        EXPECT_EQ(std::any_cast<int>(reg.converters.convert(type_tag::of<int>(), "42")), 42);
        EXPECT_EQ(std::any_cast<int>(reg.converters.convert(type_tag::of<int>(), " -7 ")), -7);
        EXPECT_EQ(std::any_cast<int>(reg.converters.convert(type_tag::of<int>(), "+3")), 3);

        // Double.Parse(InvariantCulture): '.' decimal separator, exponent form accepted.
        EXPECT_DOUBLE_EQ(std::any_cast<double>(reg.converters.convert(type_tag::of<double>(), "3.14")), 3.14);
        EXPECT_DOUBLE_EQ(std::any_cast<double>(reg.converters.convert(type_tag::of<double>(), "1e3")), 1000.0);
        EXPECT_DOUBLE_EQ(std::any_cast<double>(reg.converters.convert(type_tag::of<double>(), " -0.5 ")), -0.5);

        // Boolean.Parse: trimmed + case-insensitive TrueString/FalseString.
        EXPECT_TRUE(std::any_cast<bool>(reg.converters.convert(type_tag::of<bool>(), "True")));
        EXPECT_TRUE(std::any_cast<bool>(reg.converters.convert(type_tag::of<bool>(), "true")));
        EXPECT_FALSE(std::any_cast<bool>(reg.converters.convert(type_tag::of<bool>(), " FALSE ")));

        // String: the literal as-is, except the "{}" markup-escape prefix is stripped.
        EXPECT_EQ(std::any_cast<std::string>(reg.converters.convert(type_tag::of<std::string>(), "plain")), "plain");
        EXPECT_EQ(std::any_cast<std::string>(reg.converters.convert(type_tag::of<std::string>(), "{}{Binding}")),
                  "{Binding}");
    }

    // A registered converter rejecting a malformed literal throws xaml_parse_exception — the net
    // behavior of C#'s Parse → FormatException → XamlParseException; it derives std::runtime_error.
    TEST(xaml_converter_registry, malformed_literal_throws_xaml_parse_exception)
    {
        const registries reg;

        EXPECT_THROW((void)reg.converters.convert(type_tag::of<int>(), "abc"), xaml_parse_exception);
        EXPECT_THROW((void)reg.converters.convert(type_tag::of<int>(), "1.5"), xaml_parse_exception);
        EXPECT_THROW((void)reg.converters.convert(type_tag::of<int>(), "+-5"), xaml_parse_exception);
        EXPECT_THROW((void)reg.converters.convert(type_tag::of<double>(), ""), xaml_parse_exception);
        EXPECT_THROW((void)reg.converters.convert(type_tag::of<double>(), "100#"), xaml_parse_exception);
        EXPECT_THROW((void)reg.converters.convert(type_tag::of<bool>(), "yes"), xaml_parse_exception);
        EXPECT_THROW((void)reg.converters.convert(type_tag::of<bool>(), "abc"), std::runtime_error);
    }

    // A lookup MISS is throw-free (an empty any) — only the loader escalates it. Image sources are
    // the documented converter-less value type (color & friends register since the M7
    // converter-parity unit).
    TEST(xaml_converter_registry, missing_converter_returns_an_empty_any_without_throwing)
    {
        const registries reg;

        EXPECT_TRUE(reg.converters.has_converter(type_tag::of<maui::graphics::color>()));
        // Border.StrokeShape (shared_ptr<i_shape>) is a still-deferred type — no convert_stroke_shape yet.
        // (Image.Source's i_image_source converter landed in W6, so that is no longer the example here.)
        EXPECT_FALSE(reg.converters.has_converter(type_tag::of<std::shared_ptr<maui::graphics::i_shape>>()));
        EXPECT_FALSE(reg.converters.convert(type_tag::of<std::shared_ptr<maui::graphics::i_shape>>(), "RoundRectangle")
                         .has_value());
    }

    // ---- the converter ⇄ property seam (converters named implicitly by the value type T) -------------

    TEST(xaml_property_registry, try_set_from_text_converts_then_applies)
    {
        const registries reg;
        controls::button target;

        EXPECT_TRUE(reg.properties.try_set_from_text(type_tag::of<controls::button>(), target, "Opacity", "0.5",
                                                     reg.converters));
        EXPECT_DOUBLE_EQ(target.opacity(), 0.5);
        EXPECT_TRUE(reg.properties.try_set_from_text(type_tag::of<controls::button>(), target, "CornerRadius", "7",
                                                     reg.converters));
        EXPECT_EQ(target.corner_radius(), 7);
        // The M7 converter-parity unit: a color-typed attribute converts from markup text.
        EXPECT_TRUE(reg.properties.try_set_from_text(type_tag::of<controls::button>(), target, "TextColor", "Red",
                                                     reg.converters));
        EXPECT_EQ(target.text_color(), maui::graphics::colors::red);

        // Unknown property → false; a known property whose value type has no converter yet → false.
        EXPECT_FALSE(
            reg.properties.try_set_from_text(type_tag::of<controls::button>(), target, "NoSuch", "1", reg.converters));
        // Border.StrokeShape (shared_ptr<i_shape>) is registered but its converter is still deferred →
        // false. (Image.Source became text-settable in W6, so it is no longer the no-converter example.)
        controls::border border_target;
        EXPECT_FALSE(reg.properties.try_set_from_text(type_tag::of<controls::border>(), border_target, "StrokeShape",
                                                      "RoundRectangle", reg.converters));
    }

    // The U4 seam, exercised with a fake: registering a color converter makes the already-registered
    // color-typed properties text-settable, with no change to the property registrations.
    TEST(xaml_converter_registry, a_fake_color_converter_completes_the_seam_for_color_properties)
    {
        registries reg;
        reg.converters.register_converter<maui::graphics::color>([](const std::string& text) {
            if (text == "red")
            {
                return maui::graphics::color{1.0F, 0.0F, 0.0F};
            }
            throw xaml_parse_exception("Cannot convert \"" + text + "\" into color (fake)");
        });

        controls::button target;
        EXPECT_TRUE(reg.properties.try_set_from_text(type_tag::of<controls::button>(), target, "TextColor", "red",
                                                     reg.converters));
        EXPECT_EQ(target.text_color(), (maui::graphics::color{1.0F, 0.0F, 0.0F}));
        EXPECT_THROW((void)reg.properties.try_set_from_text(type_tag::of<controls::button>(), target, "TextColor",
                                                            "mauve", reg.converters),
                     xaml_parse_exception);
    }

    // ---- xaml_object_graph (the loader-side ownership decision) --------------------------------------

    TEST(xaml_object_graph, owns_the_loaded_tree_and_keeps_it_alive)
    {
        std::weak_ptr<maui::core::bindable_object> watch;
        {
            const registries reg;
            xaml_object_graph graph;

            auto stack = reg.types.create("VerticalStackLayout");
            auto child = reg.types.create("Button");
            ASSERT_NE(stack, nullptr);
            ASSERT_NE(child, nullptr);
            watch = child;

            // Wire the borrowed reference, then hand BOTH owning handles to the graph.
            ASSERT_TRUE(reg.properties.try_add_child(reg.types.find("VerticalStackLayout")->type, *stack, *child));
            graph.set_root(stack);
            graph.add(std::move(stack));
            graph.add(std::move(child));
            EXPECT_EQ(graph.size(), 2U);

            // The locals are gone; the graph alone keeps the tree alive (the layout only borrows).
            EXPECT_FALSE(watch.expired());
            const auto root = graph.root_as<controls::vertical_stack_layout>();
            ASSERT_NE(root, nullptr);
            EXPECT_EQ(root->count(), 1);
        }
        // Destroying the graph destroys the tree.
        EXPECT_TRUE(watch.expired());
    }

    TEST(xaml_object_graph, root_as_is_type_checked)
    {
        const registries reg;
        xaml_object_graph graph;
        auto page = reg.types.create("ContentPage");
        graph.set_root(page);
        graph.add(std::move(page));

        EXPECT_NE(graph.root_as<controls::content_page>(), nullptr);
        EXPECT_EQ(graph.root_as<controls::button>(), nullptr);
    }

    // ---- the loader dry-run: compose a small page entirely through the registries --------------------

    TEST(xaml_registries, compose_a_page_from_names_and_text_like_the_loader_will)
    {
        const registries reg;
        xaml_object_graph graph;

        // <ContentPage Title="Home"><VerticalStackLayout Spacing="12">
        //   <Label Text="Welcome"/><Button Text="Go" IsEnabled="false"/>
        // </VerticalStackLayout></ContentPage>
        auto page = reg.types.create("ContentPage");
        auto stack = reg.types.create("VerticalStackLayout");
        auto label = reg.types.create("Label");
        auto button = reg.types.create("Button");
        ASSERT_TRUE(page && stack && label && button);

        const auto page_tag = reg.types.find("ContentPage")->type;
        const auto stack_tag = reg.types.find("VerticalStackLayout")->type;
        const auto label_tag = reg.types.find("Label")->type;
        const auto button_tag = reg.types.find("Button")->type;

        EXPECT_TRUE(reg.properties.try_set_from_text(page_tag, *page, "Title", "Home", reg.converters));
        EXPECT_TRUE(reg.properties.try_set_from_text(stack_tag, *stack, "Spacing", "12", reg.converters));
        // Label element text routes through the [ContentProperty] name.
        const std::string* label_content = reg.properties.content_property(label_tag);
        ASSERT_NE(label_content, nullptr);
        EXPECT_TRUE(reg.properties.try_set_from_text(label_tag, *label, *label_content, "Welcome", reg.converters));
        EXPECT_TRUE(reg.properties.try_set_from_text(button_tag, *button, "Text", "Go", reg.converters));
        EXPECT_TRUE(reg.properties.try_set_from_text(button_tag, *button, "IsEnabled", "false", reg.converters));

        ASSERT_TRUE(reg.properties.try_add_child(stack_tag, *stack, *label));
        ASSERT_TRUE(reg.properties.try_add_child(stack_tag, *stack, *button));
        ASSERT_TRUE(reg.properties.try_add_child(page_tag, *page, *stack));

        graph.set_root(page);
        graph.add(std::move(page));
        graph.add(std::move(stack));
        graph.add(std::move(label));
        graph.add(std::move(button));

        const auto root = graph.root_as<controls::content_page>();
        ASSERT_NE(root, nullptr);
        EXPECT_EQ(root->title(), "Home");
        auto* content = dynamic_cast<controls::vertical_stack_layout*>(root->content());
        ASSERT_NE(content, nullptr);
        EXPECT_DOUBLE_EQ(content->spacing(), 12.0);
        ASSERT_EQ(content->count(), 2);
        const auto* first = dynamic_cast<controls::label*>(&content->at(0));
        const auto* second = dynamic_cast<controls::button*>(&content->at(1));
        ASSERT_NE(first, nullptr);
        ASSERT_NE(second, nullptr);
        EXPECT_EQ(first->text(), "Welcome");
        EXPECT_EQ(second->text(), "Go");
        EXPECT_FALSE(second->is_enabled());
    }
} // namespace

// Self-registration sugar under test (file scope, like a control TU would use it). Lives after the
// anonymous namespace so the macro's own anonymous-namespace registrar can reference probe_control.
MAUI_XAML_REGISTER_TYPE("XamlRegistryProbe", probe_control)
