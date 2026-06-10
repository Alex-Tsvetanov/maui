#pragma once
// maui::xaml — the v1 markup extensions (M7 wave 2)  <=  src/Controls/src/Xaml/MarkupExtensions/*:
//
//   {StaticResource Key=…}   <=  Microsoft.Maui.Controls.Xaml.StaticResourceExtension
//   {DynamicResource Key=…}  <=  Microsoft.Maui.Controls.Xaml.DynamicResourceExtension
//   {x:Static Member=…}      <=  Microsoft.Maui.Controls.Xaml.StaticExtension
//   {x:Type TypeName=…}      <=  Microsoft.Maui.Controls.Xaml.TypeExtension
//   {x:Null}                 <=  Microsoft.Maui.Controls.Xaml.NullExtension
//   {OnPlatform …}           <=  Microsoft.Maui.Controls.Xaml.OnPlatformExtension
//   {OnIdiom …}              <=  Microsoft.Maui.Controls.Xaml.OnIdiomExtension
//   {AppThemeBinding …}      <=  Microsoft.Maui.Controls.Xaml.AppThemeBindingExtension
//   {Binding Path=… Mode=…}  <=  Microsoft.Maui.Controls.Xaml.BindingExtension
//
// The extension CLASSES are an implementation detail of markup_extensions.cpp (C# exposes them
// publicly only because the XAML parser instantiates them by reflection; the port's loader minting
// goes through markup_extension_registry factories, registered by register_standard_markup_extensions
// below). What IS public here are the MARKER VALUE TYPES some ProvideValue results carry — exactly
// the C# results the loader special-cases (ApplyPropertiesVisitor.TrySetPropertyValue /
// Setter.Apply): a DynamicResource result becomes SetDynamicResource, a BindingBase result becomes
// SetBinding. The U3 applier implements its half against the contracts documented per type.
//
// ATTRIBUTE CONVENTIONS (the factory side of the seam, i_markup_extension.hpp):
//   - the POSITIONAL value of "{Name value}" arrives under the EMPTY attribute name "" and maps to
//     the extension's [ContentProperty] (Key / Member / TypeName / Default / Path), mirroring
//     MarkupExtensionParser.SetPropertyValue's prop==null → GetContentPropertyName route. A named
//     spelling of the same property wins over the positional one.
//   - a name present in markup_extension_arguments::values (a nested extension the pipeline already
//     resolved) takes precedence over its raw string form; string-typed extension properties (Key,
//     Member, TypeName, Path, Mode, Converter) require the boxed value to BE a std::string.
//   - an attribute the extension has no property for throws xaml_parse_exception (C# fails the
//     reflective property lookup and the visitor surfaces a XamlParseException).
//
// VALUE-FORM CONTRACT ({OnPlatform}/{OnIdiom}/{AppThemeBinding} slots): the chosen value is handed on
// in its SOURCE form — the pre-resolved std::any, or the raw markup text boxed as std::string. C#
// converts eagerly inside ProvideValue via IValueConverterProvider against the target property's
// reflected type; the reflection-free port has no property-type metadata at ProvideValue time, so the
// U3 applier converts a std::string result against the target property's registered value type
// exactly like a literal attribute (xaml_property_registry::try_set_from_text) — the same net
// late-conversion result. An EMPTY std::any result means "no value for this platform/idiom": the
// applier SKIPS the assignment, leaving the property at its default. (Documented deviation: C#
// physically assigns BindableProperty.GetDefaultValue at manual specificity there, which would also
// shadow later style values; the port's skip leaves the property style-able.)

#include <any>
#include <string>

#include "maui/controls/dynamic_resource.hpp"
#include "maui/core/app_theme.hpp"
#include "maui/core/binding_mode.hpp"

namespace maui::xaml
{
    // {DynamicResource Key=…}'s ProvideValue result is maui::controls::dynamic_resource (the C#
    // Microsoft.Maui.Controls.Internals.DynamicResource lives in the Controls layer, so its port does
    // too — controls/dynamic_resource.hpp): a reference to a resource by key, NOT the resource value.
    // APPLIER CONTRACT: a result holding that marker is not assigned through the property registry —
    // the applier calls element::set_dynamic_resource(<target property name>, marker.key()), the port
    // of ApplyPropertiesVisitor/Setter.Apply routing a DynamicResource into SetDynamicResource.

    // The {x:Null} ProvideValue result (C# NullExtension returns null; std::any cannot hold "typed
    // null" without knowing the property type). APPLIER CONTRACT: assign the target property's null —
    // for a shared_ptr-typed bindable property that is a boxed null shared_ptr of the property's
    // exact value type. The v1 property registry has no per-type null factory yet, so until the
    // loader grows one, applying {x:Null} is a reported load failure rather than a silent skip
    // (value-typed properties reject null in C# too).
    struct xaml_null
    {
    };

    // The {Binding} ProvideValue result — the port's stand-in for the C# Binding OBJECT
    // (BindingExtension builds `new Binding(Path, Mode, …)`; the loader then SetBinding()s it).
    //
    // DEFERRAL (precise): the port's M5 bindings are TYPED-accessor (core/binding.hpp bind() over
    // property<T> pairs — no reflection, no string paths). A runtime string-path binding needs a
    // name → typed-accessor GETTER table that does not exist yet (the xaml_property_registry only
    // resolves name → SETTER). So {Binding} produces this marker and the APPLIER CONTRACT is:
    //   - path == "." (Binding.SelfPath, the default): bind the target property to the binding
    //     context itself — implementable today for a context that IS a bindable_object exposing the
    //     target property's type;
    //   - path naming a single REGISTERED property on the target's binding context: wire bind()
    //     through the typed accessors once the getter table lands (U3/U6);
    //   - nested paths ("Customer.Name"), Source/StringFormat/Converter instances/
    //     ConverterParameter/UpdateSourceEventName/TargetNullValue/FallbackValue: NOT representable
    //     yet — the factory rejects those attributes with xaml_parse_exception rather than dropping
    //     them silently (C# honors them; see STATUS.md M7 deferrals).
    struct binding_request
    {
        std::string path = ".";                                                 // Binding.SelfPath default
        maui::core::binding_mode mode = maui::core::binding_mode::default_mode; // BindingMode.Default
        std::string converter; // Converter attribute as a NAME (raw text); empty = none. Resolution
                               // against a value-converter table is deferred with the runtime binding.
    };

    // maui::xaml::app_theme_binding  <=  Microsoft.Maui.Controls.AppThemeBinding (the BindingBase
    // {AppThemeBinding} provides — C# returns the binding object; applying + re-applying on theme
    // change happens in AppThemeBinding.Apply/ApplyCore, which is the applier's half here).
    //
    // APPLIER CONTRACT: on apply, pick(application::requested_theme()) selects the slot value (still
    // in its source form — see the value-form contract above; a dynamic_resource-holding slot routes
    // to set_dynamic_resource exactly like C# ApplyCore's `value is DynamicResource` branch), then
    // re-apply on every application::requested_theme_changed (C# subscribes via its AppThemeProxy's
    // "__MAUI_ApplicationTheme__" dynamic resource). An empty picked slot follows the same skip rule
    // as OnPlatform's empty result.
    struct app_theme_binding
    {
        std::any light;         // AppThemeBinding.Light
        std::any dark;          // AppThemeBinding.Dark
        std::any default_value; // AppThemeBinding.Default (empty = unset)
        bool has_light = false; // _isLightSet
        bool has_dark = false;  // _isDarkSet

        // AppThemeBinding.GetValue: Dark → Dark-if-set else Default; Light AND Unspecified → Light-
        // if-set else Default (C# defaults an unspecified theme to the light branch).
        [[nodiscard]] const std::any& pick(maui::core::app_theme theme) const;
    };

    // Register the nine v1 extensions in markup_extension_registry::instance() (under their markup
    // names — "StaticResource", …, "x:Static" — plus the implicit "<name>Extension" aliases the
    // registry adds) and seed xaml_static_registry::instance() with the port's standard statics (the
    // named "Colors.*"). The explicit one-call setup, mirroring register_standard_xaml — called by
    // the U3 loader once, and by tests. Idempotent (re-registration replaces).
    void register_standard_markup_extensions();
} // namespace maui::xaml
