#pragma once
// maui::xaml::register_runtime_bindings  <=  ApplyPropertiesVisitor.TrySetBinding
//   (src/Controls/src/Xaml/ApplyPropertiesVisitor.cs — "if (value is BindingBase binding) …
//    bindable.SetBinding(property, binding)")
//
// The runtime-binding half of the xaml_binding_applier seam (xaml_binding_applier.hpp): installs
// the REAL applier that routes a {Binding} result — the maui::controls::binding the BindingExtension
// factory built (markup_extensions.hpp) — into element::set_binding under the property registry's
// bindable descriptor name. C# reaches SetBinding through reflection-found BindableProperty fields;
// the reflection-free port resolves the XAML attribute through xaml_property_registry and fails with
// TrySetPropertyValue's catch-all "Cannot assign property …" when the attribute names no bindable
// property or the target is not a controls::element (LoaderTests
// .TestSetBindingToNonBindablePropertyShouldThrow).
//
// Explicit registration per PROFILE §6 (no static initializer magic): call once before loading XAML
// that uses {Binding} — alongside register_standard_markup_extensions. Idempotent; process-wide like
// the seam it fills. set_xaml_binding_applier(nullptr) restores the rejecting default (test seam).

namespace maui::xaml
{
    void register_runtime_bindings();
} // namespace maui::xaml
