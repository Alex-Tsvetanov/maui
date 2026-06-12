// Tests for the M7 wave-2 v1 markup extensions (maui::xaml markup_extensions.hpp) — each factory is
// driven DIRECTLY (mint via markup_extension_registry, call provide_value with a hand-built
// xaml_service_provider), the way C#'s MarkupExtensionTests drives MarkupExtensionParser without a
// full XAML load. Behavior derived from src/Controls/src/Xaml/MarkupExtensions/*.cs and the C# tests:
// MarkupExtensionTests.cs (registry/suffix semantics), XStatic.xaml.cs ({x:Static} member kinds),
// TypeExtensionTests/XmlTypeTests ({x:Type}), NullExtensionTests ({x:Null}), OnPlatformTests +
// OnIdiomTests (platform/idiom selection incl. the UWP/WinUI aliasing), OnAppThemeTests
// ({AppThemeBinding} light/dark/default picks), and StaticResource/DynamicResource per their sources.
#include "maui/xaml/markup_extensions.hpp"

#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/application.hpp"
#include "maui/controls/bindings/binding.hpp"
#include "maui/controls/bindings/i_value_converter.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/dynamic_resource.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/app_theme.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/type_tag.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/xaml/i_markup_extension.hpp"
#include "maui/xaml/xaml_parse_exception.hpp"
#include "maui/xaml/xaml_runtime_environment.hpp"
#include "maui/xaml/xaml_standard_types.hpp"
#include "maui/xaml/xaml_static_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::dynamic_resource;
    using maui::core::app_theme;
    using maui::core::binding_mode;
    using maui::core::type_tag;
    using maui::graphics::color;
    using maui::xaml::app_theme_binding;
    using maui::xaml::binding_request;
    using maui::xaml::device_idiom;
    using maui::xaml::device_platform;
    using maui::xaml::markup_extension_arguments;
    using maui::xaml::markup_extension_factory;
    using maui::xaml::markup_extension_registry;
    using maui::xaml::xaml_null;
    using maui::xaml::xaml_parse_exception;
    using maui::xaml::xaml_runtime_environment;
    using maui::xaml::xaml_service_provider;

    // The factory for `name` from the process registry, with the standard set registered once.
    const markup_extension_factory& factory_for(std::string_view name)
    {
        static const bool registered = [] {
            maui::xaml::register_standard_markup_extensions();
            return true;
        }();
        (void)registered;
        const markup_extension_factory* factory = markup_extension_registry::instance().find(name);
        if (factory == nullptr)
        {
            throw std::runtime_error("markup extension not registered: " + std::string{name});
        }
        return *factory;
    }

    // Mint + ProvideValue in one go (the MarkupExtensionParser.Parse tail).
    std::any provide(std::string_view name, const markup_extension_arguments& args,
                     const xaml_service_provider& services = {})
    {
        return factory_for(name)(args)->provide_value(services);
    }

    // Run `action` and return the thrown xaml_parse_exception's message (FAIL-free assert helper).
    template <class F> std::string parse_error_message(F&& action)
    {
        try
        {
            (void)std::forward<F>(action)();
        }
        catch (const xaml_parse_exception& exception)
        {
            return exception.unformatted_message();
        }
        return "(no xaml_parse_exception thrown)";
    }

    // DeviceInfo.SetCurrent(mock)-style RAII pin of the runtime environment.
    class environment_guard
    {
    public:
        environment_guard(device_platform platform, device_idiom idiom) : saved_(xaml_runtime_environment::current())
        {
            xaml_runtime_environment::set_current({.platform = platform, .idiom = idiom});
        }
        environment_guard(const environment_guard&) = delete;
        environment_guard(environment_guard&&) = delete;
        environment_guard& operator=(const environment_guard&) = delete;
        environment_guard& operator=(environment_guard&&) = delete;
        ~environment_guard()
        {
            xaml_runtime_environment::set_current(saved_);
        }

    private:
        xaml_runtime_environment saved_;
    };

    // ---- the registry seam (MarkupExtensionTests.TestLookupWithSuffix / ThrowOnMarkupExtensionNotFound) ----

    TEST(markup_extension_registry_lookup, extension_suffix_spelling_resolves)
    {
        // C# tolerates both "{local:Baa}" and the Extension-suffixed class spelling.
        (void)factory_for("StaticResource");
        EXPECT_NE(markup_extension_registry::instance().find("StaticResourceExtension"), nullptr);
        EXPECT_NE(markup_extension_registry::instance().find("BindingExtension"), nullptr);
    }

    TEST(markup_extension_registry_lookup, unknown_extension_is_a_throw_free_miss)
    {
        // The registry miss is nullptr; the LOADER throws ("MarkupExtension not found for {match}").
        (void)factory_for("StaticResource");
        EXPECT_EQ(markup_extension_registry::instance().find("Missing"), nullptr);
    }

    // ---- {StaticResource} (StaticResourceExtension.cs) ----

    TEST(static_resource_extension, resolves_from_the_nearest_scope)
    {
        maui::controls::label label;
        label.resources().set("accent", std::any{maui::graphics::colors::fuchsia});
        xaml_service_provider services;
        services.resource_scope = &label;

        const std::any result = provide("StaticResource", {.attributes = {{"Key", "accent"}}}, services);
        ASSERT_NE(std::any_cast<color>(&result), nullptr);
        EXPECT_EQ(std::any_cast<color>(result), maui::graphics::colors::fuchsia);
    }

    TEST(static_resource_extension, walks_the_parent_chain)
    {
        maui::controls::vertical_stack_layout layout;
        maui::controls::label label;
        layout.resources().set("greeting", std::any{std::string{"hello"}});
        layout.add(label); // attaches label as a logical child -> the chain reaches the layout

        xaml_service_provider services;
        services.resource_scope = &label;
        const std::any result = provide("StaticResource", {.attributes = {{"Key", "greeting"}}}, services);
        ASSERT_NE(std::any_cast<std::string>(&result), nullptr);
        EXPECT_EQ(std::any_cast<std::string>(result), "hello");
    }

    TEST(static_resource_extension, the_closer_scope_wins)
    {
        // ResourcesExtensions.TryGetResource walks self -> ancestors; the first hit wins.
        maui::controls::vertical_stack_layout layout;
        maui::controls::label label;
        layout.resources().set("accent", std::any{std::string{"outer"}});
        label.resources().set("accent", std::any{std::string{"inner"}});
        layout.add(label);

        xaml_service_provider services;
        services.resource_scope = &label;
        const std::any result = provide("StaticResource", {.attributes = {{"Key", "accent"}}}, services);
        EXPECT_EQ(std::any_cast<std::string>(result), "inner");
    }

    TEST(static_resource_extension, falls_back_to_application_resources)
    {
        // StaticResourceExtension.TryGetApplicationLevelResource (Application.Current.Resources).
        maui::controls::application app;
        app.resources().set("app-wide", std::any{42});
        maui::controls::label label; // NOT under the app — the chain misses
        xaml_service_provider services;
        services.resource_scope = &label;
        services.application = &app;

        const std::any result = provide("StaticResource", {.attributes = {{"Key", "app-wide"}}}, services);
        EXPECT_EQ(std::any_cast<int>(result), 42);
    }

    TEST(static_resource_extension, application_without_created_resources_is_a_miss)
    {
        maui::controls::application app; // resources never touched -> IsResourcesCreated false
        xaml_service_provider services;
        services.application = &app;
        EXPECT_EQ(
            parse_error_message([&] { return provide("StaticResource", {.attributes = {{"Key", "k"}}}, services); }),
            "StaticResource not found for key k");
        EXPECT_FALSE(app.is_resources_created()); // the lookup must not lazily create the dictionary
    }

    TEST(static_resource_extension, missing_key_throws)
    {
        // C#: "StaticResource not found for key {Key}".
        maui::controls::label label;
        xaml_service_provider services;
        services.resource_scope = &label;
        EXPECT_EQ(
            parse_error_message([&] { return provide("StaticResource", {.attributes = {{"Key", "nope"}}}, services); }),
            "StaticResource not found for key nope");
    }

    TEST(static_resource_extension, missing_key_attribute_throws)
    {
        // C#: "you must specify a key in {StaticResource}".
        EXPECT_EQ(parse_error_message([&] { return provide("StaticResource", {}); }),
                  "you must specify a key in {StaticResource}");
    }

    TEST(static_resource_extension, positional_key_is_the_content_property)
    {
        // [ContentProperty(nameof(Key))]: "{StaticResource accent}".
        maui::controls::label label;
        label.resources().set("accent", std::any{1.5});
        xaml_service_provider services;
        services.resource_scope = &label;
        const std::any result = provide("StaticResource", {.attributes = {{"", "accent"}}}, services);
        EXPECT_EQ(std::any_cast<double>(result), 1.5);
    }

    TEST(static_resource_extension, pre_resolved_key_value_wins_over_raw_text)
    {
        // A values entry takes precedence over the attribute string (i_markup_extension.hpp).
        maui::controls::label label;
        label.resources().set("right", std::any{std::string{"resolved"}});
        xaml_service_provider services;
        services.resource_scope = &label;
        const markup_extension_arguments args{.attributes = {{"Key", "wrong"}},
                                              .values = {{"Key", std::any{std::string{"right"}}}}};
        EXPECT_EQ(std::any_cast<std::string>(provide("StaticResource", args, services)), "resolved");
    }

    TEST(static_resource_extension, unknown_attribute_throws)
    {
        EXPECT_EQ(parse_error_message([&] { return provide("StaticResource", {.attributes = {{"Frobnicate", "x"}}}); }),
                  "Markup extension StaticResource has no property 'Frobnicate'");
    }

    // ---- {DynamicResource} (DynamicResourceExtension.cs) ----

    TEST(dynamic_resource_extension, returns_the_key_marker)
    {
        // C# returns the DynamicResource OBJECT — the applier turns it into SetDynamicResource.
        const std::any named = provide("DynamicResource", {.attributes = {{"Key", "accent"}}});
        ASSERT_NE(std::any_cast<dynamic_resource>(&named), nullptr);
        EXPECT_EQ(std::any_cast<dynamic_resource>(&named)->key(), "accent");

        const std::any positional = provide("DynamicResource", {.attributes = {{"", "accent"}}});
        EXPECT_EQ(std::any_cast<dynamic_resource>(&positional)->key(), "accent");
    }

    TEST(dynamic_resource_extension, missing_key_throws)
    {
        // C# (sic): "DynamicResource markup require a Key".
        EXPECT_EQ(parse_error_message([&] { return provide("DynamicResource", {}); }),
                  "DynamicResource markup require a Key");
    }

    // ---- {x:Static} (StaticExtension.cs + XStatic.xaml.cs) ----

    TEST(x_static_extension, named_colors_are_registered)
    {
        // XStatic.xaml's FieldColor case uses Colors.Fuchsia; AliceBlue covers the multi-word names.
        const std::any fuchsia = provide("x:Static", {.attributes = {{"Member", "Colors.Fuchsia"}}});
        EXPECT_EQ(std::any_cast<color>(fuchsia), maui::graphics::colors::fuchsia);

        const std::any alice = provide("x:Static", {.attributes = {{"Member", "Colors.AliceBlue"}}});
        EXPECT_EQ(std::any_cast<color>(alice), maui::graphics::colors::alice_blue);
    }

    TEST(x_static_extension, member_attribute_name_is_optional)
    {
        // XStatic.xaml.cs MemberOptional: "{x:Static local:MockxStatic.MockStaticProperty}".
        const std::any result = provide("x:Static", {.attributes = {{"", "Colors.Black"}}});
        EXPECT_EQ(std::any_cast<color>(result), maui::graphics::colors::black);
    }

    TEST(x_static_extension, custom_member_resolves_through_the_explicit_registry)
    {
        // The reflection-free analog of XStatic.xaml.cs StaticProperty: the app registers its static.
        maui::xaml::xaml_static_registry::instance().register_member("MockxStatic.MockStaticProperty",
                                                                     [] { return std::any{std::string{"Property"}}; });
        const std::any result = provide("x:Static", {.attributes = {{"Member", "MockxStatic.MockStaticProperty"}}});
        EXPECT_EQ(std::any_cast<std::string>(result), "Property");
    }

    TEST(x_static_extension, an_xmlns_prefix_is_stripped_for_lookup)
    {
        // C# markup spells "{x:Static local:MockxStatic.MockStaticProperty}" — registrations are
        // keyed by the bare "Type.Member", so the prefix is retried away.
        maui::xaml::xaml_static_registry::instance().register_member("MockxStatic.MockField",
                                                                     [] { return std::any{std::string{"Field"}}; });
        const std::any result = provide("x:Static", {.attributes = {{"Member", "local:MockxStatic.MockField"}}});
        EXPECT_EQ(std::any_cast<std::string>(result), "Field");
    }

    TEST(x_static_extension, member_without_a_dot_is_a_syntax_error)
    {
        // C#: "Syntax for x:Static is [Member=][prefix:]typeName.staticMemberName".
        const std::string expected = "Syntax for x:Static is [Member=][prefix:]typeName.staticMemberName";
        EXPECT_EQ(parse_error_message([&] { return provide("x:Static", {.attributes = {{"Member", "NoDot"}}}); }),
                  expected);
        EXPECT_EQ(parse_error_message([&] { return provide("x:Static", {}); }), expected); // Member missing
    }

    TEST(x_static_extension, unknown_member_throws)
    {
        // C#: "No static member found for {Member}".
        EXPECT_EQ(
            parse_error_message([&] { return provide("x:Static", {.attributes = {{"Member", "Nope.Nothing"}}}); }),
            "No static member found for Nope.Nothing");
    }

    // ---- {x:Type} (TypeExtension.cs) ----

    TEST(x_type_extension, resolves_a_registered_type_to_its_tag)
    {
        maui::xaml::xaml_type_registry types;
        maui::xaml::register_standard_xaml_types(types);
        xaml_service_provider services;
        services.type_registry = &types;

        const std::any named = provide("x:Type", {.attributes = {{"TypeName", "Button"}}}, services);
        EXPECT_EQ(std::any_cast<type_tag>(named), type_tag::of<maui::controls::button>());

        const std::any positional = provide("x:Type", {.attributes = {{"", "Label"}}}, services);
        EXPECT_EQ(std::any_cast<type_tag>(positional), type_tag::of<maui::controls::label>());
    }

    TEST(x_type_extension, the_x_prefix_selects_the_x_namespace)
    {
        maui::xaml::xaml_type_registry types;
        types.register_type<maui::controls::label>("Probe", maui::xaml::xaml_namespace::x);
        xaml_service_provider services;
        services.type_registry = &types;
        const std::any result = provide("x:Type", {.attributes = {{"TypeName", "x:Probe"}}}, services);
        EXPECT_EQ(std::any_cast<type_tag>(result), type_tag::of<maui::controls::label>());
    }

    TEST(x_type_extension, missing_type_name_throws)
    {
        // C#: "TypeName isn't set." (missing AND empty).
        EXPECT_EQ(parse_error_message([&] { return provide("x:Type", {}); }), "TypeName isn't set.");
        EXPECT_EQ(parse_error_message([&] { return provide("x:Type", {.attributes = {{"TypeName", ""}}}); }),
                  "TypeName isn't set.");
    }

    TEST(x_type_extension, unknown_type_and_prefix_throw)
    {
        const maui::xaml::xaml_type_registry types; // empty
        xaml_service_provider services;
        services.type_registry = &types;
        // XamlParser.GetElementType's message shape.
        EXPECT_EQ(
            parse_error_message([&] { return provide("x:Type", {.attributes = {{"TypeName", "Bogus"}}}, services); }),
            "Type Bogus not found in xmlns http://schemas.microsoft.com/dotnet/2021/maui");
        EXPECT_EQ(
            parse_error_message([&] { return provide("x:Type", {.attributes = {{"TypeName", "foo:Bar"}}}, services); }),
            "No xmlns declaration for prefix 'foo'.");
    }

    // ---- {x:Null} (NullExtension.cs + NullExtensionTests.cs) ----

    TEST(x_null_extension, returns_the_null_marker)
    {
        // C# TestxNull asserts the parsed result is null; the port's typed-null is the marker.
        const std::any result = provide("x:Null", {});
        EXPECT_NE(std::any_cast<xaml_null>(&result), nullptr);
    }

    TEST(x_null_extension, any_attribute_throws)
    {
        EXPECT_EQ(parse_error_message([&] { return provide("x:Null", {.attributes = {{"Key", "x"}}}); }),
                  "Markup extension x:Null has no property 'Key'");
    }

    // ---- {OnPlatform} (OnPlatformExtension.cs + OnPlatformTests.cs) ----

    TEST(on_platform_extension, selects_the_running_platform)
    {
        const environment_guard guard{device_platform::ios, device_idiom::unknown};
        const std::any result =
            provide("OnPlatform", {.attributes = {{"iOS", "ios-value"}, {"Android", "android-value"}}});
        EXPECT_EQ(std::any_cast<std::string>(result), "ios-value"); // raw text — the applier converts late
    }

    TEST(on_platform_extension, falls_back_to_default)
    {
        const environment_guard guard{device_platform::android, device_idiom::unknown};
        const std::any result =
            provide("OnPlatform", {.attributes = {{"iOS", "ios-value"}, {"Default", "default-value"}}});
        EXPECT_EQ(std::any_cast<std::string>(result), "default-value");
    }

    TEST(on_platform_extension, no_value_for_the_platform_yields_an_empty_any)
    {
        // C# would assign the target property's default; the port's empty any = "skip the assignment"
        // (the documented deviation in markup_extensions.hpp).
        const environment_guard guard{device_platform::tizen, device_idiom::unknown};
        const std::any result = provide("OnPlatform", {.attributes = {{"iOS", "ios-value"}}});
        EXPECT_FALSE(result.has_value());
    }

    TEST(on_platform_extension, all_unset_throws)
    {
        EXPECT_EQ(parse_error_message([&] { return provide("OnPlatform", {}); }),
                  "OnPlatformExtension requires a value to be specified for at least one platform or Default.");
    }

    TEST(on_platform_extension, uwp_matches_the_winui_platform_and_winui_is_preferred)
    {
        // DevicePlatform.UWP == WinUI in C# (OnPlatformTests.UWPisWinUI / ChecksPreferWinUI).
        const environment_guard guard{device_platform::win_ui, device_idiom::unknown};
        const std::any uwp_only = provide("OnPlatform", {.attributes = {{"UWP", "uwp-value"}}});
        EXPECT_EQ(std::any_cast<std::string>(uwp_only), "uwp-value");

        const std::any both = provide("OnPlatform", {.attributes = {{"UWP", "uwp-value"}, {"WinUI", "winui-value"}}});
        EXPECT_EQ(std::any_cast<std::string>(both), "winui-value");
    }

    TEST(on_platform_extension, macos_and_mac_catalyst_are_distinct)
    {
        const markup_extension_arguments args{.attributes = {{"macOS", "appkit"}, {"MacCatalyst", "catalyst"}}};
        {
            const environment_guard guard{device_platform::mac_os, device_idiom::desktop};
            EXPECT_EQ(std::any_cast<std::string>(provide("OnPlatform", args)), "appkit");
        }
        {
            const environment_guard guard{device_platform::mac_catalyst, device_idiom::desktop};
            EXPECT_EQ(std::any_cast<std::string>(provide("OnPlatform", args)), "catalyst");
        }
    }

    TEST(on_platform_extension, a_pre_resolved_value_passes_through_unconverted)
    {
        // "{OnPlatform iOS={StaticResource accent}}": the chosen value keeps its boxed form.
        const environment_guard guard{device_platform::ios, device_idiom::unknown};
        const markup_extension_arguments args{.values = {{"iOS", std::any{maui::graphics::colors::teal}}}};
        EXPECT_EQ(std::any_cast<color>(provide("OnPlatform", args)), maui::graphics::colors::teal);
    }

    TEST(on_platform_extension, the_positional_value_is_default)
    {
        // [ContentProperty(nameof(Default))].
        const environment_guard guard{device_platform::android, device_idiom::unknown};
        const std::any result = provide("OnPlatform", {.attributes = {{"", "everywhere"}}});
        EXPECT_EQ(std::any_cast<std::string>(result), "everywhere");
    }

    TEST(on_platform_extension, value_converters_are_rejected_loudly)
    {
        // C# supports Converter/ConverterParameter; the port defers IValueConverter (STATUS.md M7).
        EXPECT_EQ(parse_error_message(
                      [&] { return provide("OnPlatform", {.attributes = {{"iOS", "x"}, {"Converter", "c"}}}); }),
                  "Property 'Converter' of OnPlatform is not supported by the port yet (STATUS.md M7 deferrals)");
    }

    // ---- {OnIdiom} (OnIdiomExtension.cs + OnIdiomTests.cs) ----

    TEST(on_idiom_extension, selects_the_running_idiom)
    {
        // OnIdiomTests.StackLayoutOrientation: Phone -> Vertical, Tablet -> Horizontal.
        const markup_extension_arguments args{.attributes = {{"Phone", "Vertical"}, {"Tablet", "Horizontal"}}};
        {
            const environment_guard guard{device_platform::unknown, device_idiom::phone};
            EXPECT_EQ(std::any_cast<std::string>(provide("OnIdiom", args)), "Vertical");
        }
        {
            const environment_guard guard{device_platform::unknown, device_idiom::tablet};
            EXPECT_EQ(std::any_cast<std::string>(provide("OnIdiom", args)), "Horizontal");
        }
    }

    TEST(on_idiom_extension, a_matched_idiom_without_a_value_falls_to_default)
    {
        // C# GetValue: `Phone ?? Default`.
        const environment_guard guard{device_platform::unknown, device_idiom::phone};
        const std::any result =
            provide("OnIdiom", {.attributes = {{"Tablet", "tablet-value"}, {"Default", "default-value"}}});
        EXPECT_EQ(std::any_cast<std::string>(result), "default-value");
    }

    TEST(on_idiom_extension, an_unknown_idiom_uses_default_and_empty_means_skip)
    {
        const environment_guard guard{device_platform::unknown, device_idiom::unknown};
        EXPECT_EQ(std::any_cast<std::string>(
                      provide("OnIdiom", {.attributes = {{"Watch", "w"}, {"Default", "default-value"}}})),
                  "default-value");
        EXPECT_FALSE(provide("OnIdiom", {.attributes = {{"Watch", "w"}}}).has_value());
    }

    TEST(on_idiom_extension, all_unset_throws)
    {
        EXPECT_EQ(parse_error_message([&] { return provide("OnIdiom", {}); }),
                  "OnIdiomExtension requires a non-null value to be specified for at least one idiom or Default.");
    }

    // ---- {AppThemeBinding} (AppThemeBindingExtension.cs + AppThemeBinding.cs + OnAppThemeTests.cs) ----

    TEST(app_theme_binding_extension, picks_light_and_dark)
    {
        // OnAppThemeTests.OnAppThemeExtensionLightDarkColor — raw strings, converted late by the applier.
        const std::any result = provide("AppThemeBinding", {.attributes = {{"Light", "Green"}, {"Dark", "Red"}}});
        const auto* binding = std::any_cast<app_theme_binding>(&result);
        ASSERT_NE(binding, nullptr);
        EXPECT_EQ(std::any_cast<std::string>(binding->pick(app_theme::light)), "Green");
        EXPECT_EQ(std::any_cast<std::string>(binding->pick(app_theme::dark)), "Red");
    }

    TEST(app_theme_binding_extension, an_unspecified_theme_defaults_to_light)
    {
        // OnAppThemeTests.OnAppThemeUnspecifiedThemeDefaultsToLightColor.
        const std::any result = provide("AppThemeBinding", {.attributes = {{"Light", "Green"}, {"Dark", "Red"}}});
        EXPECT_EQ(std::any_cast<std::string>(std::any_cast<app_theme_binding>(&result)->pick(app_theme::unspecified)),
                  "Green");
    }

    TEST(app_theme_binding_extension, an_unset_theme_slot_falls_to_default)
    {
        // OnAppThemeTests.OnAppThemeUnspecifiedLightColorDefaultsToDefault (+ the dark mirror).
        const std::any result = provide("AppThemeBinding", {.attributes = {{"Default", "Green"}, {"Dark", "Red"}}});
        const auto* binding = std::any_cast<app_theme_binding>(&result);
        EXPECT_EQ(std::any_cast<std::string>(binding->pick(app_theme::light)), "Green");

        const std::any mirrored = provide("AppThemeBinding", {.attributes = {{"Default", "G"}, {"Light", "L"}}});
        EXPECT_EQ(std::any_cast<std::string>(std::any_cast<app_theme_binding>(&mirrored)->pick(app_theme::dark)), "G");
    }

    TEST(app_theme_binding_extension, the_positional_value_is_default)
    {
        const std::any result = provide("AppThemeBinding", {.attributes = {{"", "everywhere"}}});
        const auto* binding = std::any_cast<app_theme_binding>(&result);
        EXPECT_EQ(std::any_cast<std::string>(binding->pick(app_theme::light)), "everywhere");
        EXPECT_EQ(std::any_cast<std::string>(binding->pick(app_theme::dark)), "everywhere");
    }

    TEST(app_theme_binding_extension, a_pre_resolved_value_passes_through)
    {
        // "{AppThemeBinding Light={StaticResource c}, …}" — the slot keeps the boxed form, and a
        // dynamic_resource-holding slot is the applier's SetDynamicResource route (C# ApplyCore).
        const markup_extension_arguments args{.values = {{"Light", std::any{maui::graphics::colors::teal}}}};
        const std::any result = provide("AppThemeBinding", args);
        EXPECT_EQ(std::any_cast<color>(std::any_cast<app_theme_binding>(&result)->pick(app_theme::light)),
                  maui::graphics::colors::teal);
    }

    TEST(app_theme_binding_extension, all_unset_throws)
    {
        EXPECT_EQ(parse_error_message([&] { return provide("AppThemeBinding", {}); }),
                  "AppThemeBindingExtension requires a non-null value to be specified for at least one theme or "
                  "Default.");
    }

    // ---- {Binding} (BindingExtension.cs) ----

    // A test IValueConverter (the C# tests' ReverseConverter shape, identity here — only the
    // instance plumbing is under test at the extension level).
    class reversing_converter final : public maui::controls::i_value_converter
    {
    public:
        [[nodiscard]] std::any convert(const std::any& value, maui::core::type_tag /*target_type*/,
                                       const std::any& /*parameter*/) override
        {
            return value;
        }
        [[nodiscard]] std::any convert_back(const std::any& value, maui::core::type_tag /*target_type*/,
                                            const std::any& /*parameter*/) override
        {
            return value;
        }
    };

    // The built C# Binding object the request carries (the W1-02 engine).
    std::shared_ptr<maui::controls::binding> built_binding(const std::any& result)
    {
        const auto* request = std::any_cast<binding_request>(&result);
        if (request == nullptr)
        {
            return nullptr;
        }
        return std::dynamic_pointer_cast<maui::controls::binding>(request->instance);
    }

    TEST(binding_extension, defaults_to_self_path_and_default_mode)
    {
        // C# Path defaults to Binding.SelfPath ".", Mode to BindingMode.Default.
        const std::any result = provide("Binding", {});
        const auto* request = std::any_cast<binding_request>(&result);
        ASSERT_NE(request, nullptr);
        EXPECT_EQ(request->path, ".");
        EXPECT_EQ(request->mode, binding_mode::default_mode);
        const std::shared_ptr<maui::controls::binding> built = built_binding(result);
        ASSERT_NE(built, nullptr); // ProvideValue => new Binding(".", Default, …)
        EXPECT_EQ(built->path(), ".");
        EXPECT_EQ(built->mode(), binding_mode::default_mode);
        EXPECT_EQ(built->converter(), nullptr);
        EXPECT_TRUE(built->string_format().empty());
        EXPECT_FALSE(built->has_source());
    }

    TEST(binding_extension, parses_path_and_mode)
    {
        // [ContentProperty(nameof(Path))]: "{Binding Name, Mode=TwoWay}".
        const std::any result = provide("Binding", {.attributes = {{"", "Name"}, {"Mode", "TwoWay"}}});
        const auto* request = std::any_cast<binding_request>(&result);
        ASSERT_NE(request, nullptr);
        EXPECT_EQ(request->path, "Name");
        EXPECT_EQ(request->mode, binding_mode::two_way);
        const std::shared_ptr<maui::controls::binding> built = built_binding(result);
        ASSERT_NE(built, nullptr);
        EXPECT_EQ(built->path(), "Name");
        EXPECT_EQ(built->mode(), binding_mode::two_way);

        const std::any named = provide("Binding", {.attributes = {{"Path", "Title"}, {"Mode", "OneWayToSource"}}});
        EXPECT_EQ(std::any_cast<binding_request>(&named)->path, "Title");
        EXPECT_EQ(std::any_cast<binding_request>(&named)->mode, binding_mode::one_way_to_source);
    }

    TEST(binding_extension, a_malformed_mode_throws)
    {
        // Enum.Parse failure surfaces as a XamlParseException (TypeConversionExtensions' shape);
        // names are case-sensitive like C#'s ignoreCase: false.
        EXPECT_EQ(parse_error_message([&] { return provide("Binding", {.attributes = {{"Mode", "Sideways"}}}); }),
                  "Cannot convert \"Sideways\" into BindingMode");
        EXPECT_EQ(parse_error_message([&] { return provide("Binding", {.attributes = {{"Mode", "twoway"}}}); }),
                  "Cannot convert \"twoway\" into BindingMode");
    }

    TEST(binding_extension, string_format_is_set_on_the_binding)
    {
        // BindingExtension.StringFormat -> new Binding(…, stringFormat: StringFormat, …).
        const std::any result = provide("Binding", {.attributes = {{"", "Name"}, {"StringFormat", "{0:F2}"}}});
        const std::shared_ptr<maui::controls::binding> built = built_binding(result);
        ASSERT_NE(built, nullptr);
        EXPECT_EQ(built->string_format(), "{0:F2}");
    }

    TEST(binding_extension, a_pre_resolved_converter_is_set_on_the_binding)
    {
        // "{Binding Name, Converter={StaticResource …}}": the nested extension resolves to the boxed
        // resource value, here an i_value_converter instance.
        const std::shared_ptr<maui::controls::i_value_converter> converter = std::make_shared<reversing_converter>();
        markup_extension_arguments args{.attributes = {{"", "Name"}, {"ConverterParameter", "token"}}};
        args.values.insert_or_assign("Converter", std::any{converter});
        const std::any result = provide("Binding", args);
        const std::shared_ptr<maui::controls::binding> built = built_binding(result);
        ASSERT_NE(built, nullptr);
        EXPECT_EQ(built->converter(), converter);
        // ConverterParameter is a plain object slot: the raw attribute string arrives boxed.
        EXPECT_EQ(std::any_cast<std::string>(built->converter_parameter()), "token");
    }

    TEST(binding_extension, a_raw_string_converter_throws)
    {
        // No string -> IValueConverter instance lookup exists (C#'s reflective assignment of a
        // string to the IValueConverter CLR property is a XamlParseException too).
        EXPECT_EQ(parse_error_message([&] { return provide("Binding", {.attributes = {{"Converter", "reverse"}}}); }),
                  "Property 'Converter' of Binding requires a markup extension providing an "
                  "i_value_converter (e.g. {StaticResource …})");
    }

    TEST(binding_extension, a_pre_resolved_bindable_source_is_set_on_the_binding)
    {
        // "{Binding Name, Source={StaticResource vm}}": the create-pass / code-seeded resource box
        // (shared_ptr<bindable_object>) becomes the explicit walkable source.
        const auto source = std::make_shared<maui::controls::label>();
        markup_extension_arguments args{.attributes = {{"", "Name"}}};
        args.values.insert_or_assign("Source", std::any{std::static_pointer_cast<maui::core::bindable_object>(source)});
        const std::any result = provide("Binding", args);
        const std::shared_ptr<maui::controls::binding> built = built_binding(result);
        ASSERT_NE(built, nullptr);
        EXPECT_TRUE(built->has_source());
    }

    TEST(binding_extension, a_non_walkable_source_throws)
    {
        // A value-only Source (C# accepts any object) has no walkable form in the port — loud.
        EXPECT_EQ(parse_error_message([&] { return provide("Binding", {.attributes = {{"Source", "x"}}}); }),
                  "Property 'Source' of Binding requires a markup extension providing a "
                  "bindable_object (e.g. {StaticResource …})");
        markup_extension_arguments args;
        args.values.insert_or_assign("Source", std::any{std::string{"boxed-but-not-bindable"}});
        EXPECT_EQ(parse_error_message([&] { return provide("Binding", args); }),
                  "Property 'Source' of Binding requires a bindable_object provided by a markup "
                  "extension (STATUS.md M7 deferrals)");
    }

    TEST(binding_extension, target_null_and_fallback_values_pass_through)
    {
        // TargetNullValue / FallbackValue are object slots — the source form passes through.
        const std::any result = provide(
            "Binding", {.attributes = {{"", "Name"}, {"TargetNullValue", "none"}, {"FallbackValue", "missing"}}});
        const std::shared_ptr<maui::controls::binding> built = built_binding(result);
        ASSERT_NE(built, nullptr);
        EXPECT_EQ(std::any_cast<std::string>(built->target_null_value()), "none");
        EXPECT_EQ(std::any_cast<std::string>(built->fallback_value()), "missing");
    }

    TEST(binding_extension, deferred_properties_are_rejected_loudly)
    {
        // C# honors UpdateSourceEventName (reflective event lookup); the port rejects it instead of
        // dropping it silently.
        EXPECT_EQ(parse_error_message([&] {
                      return provide("Binding", {.attributes = {{"", "Name"}, {"UpdateSourceEventName", "Completed"}}});
                  }),
                  "Property 'UpdateSourceEventName' of Binding is not supported by the port yet (STATUS.md M7 "
                  "deferrals)");
    }

    TEST(binding_extension, an_unknown_attribute_throws)
    {
        EXPECT_EQ(parse_error_message([&] { return provide("Binding", {.attributes = {{"Frobnicate", "x"}}}); }),
                  "Markup extension Binding has no property 'Frobnicate'");
    }

    // ---- xaml_runtime_environment (the DeviceInfo.SetCurrent seam) ----

    TEST(runtime_environment, set_current_round_trips)
    {
        const xaml_runtime_environment saved = xaml_runtime_environment::current();
        xaml_runtime_environment::set_current({.platform = device_platform::tizen, .idiom = device_idiom::tv});
        EXPECT_EQ(xaml_runtime_environment::current().platform, device_platform::tizen);
        EXPECT_EQ(xaml_runtime_environment::current().idiom, device_idiom::tv);
        xaml_runtime_environment::set_current(saved);
    }
} // namespace
