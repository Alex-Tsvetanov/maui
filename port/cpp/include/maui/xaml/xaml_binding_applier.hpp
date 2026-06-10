#pragma once
// maui::xaml::xaml_binding_applier  <=  the ApplyPropertiesVisitor.TrySetBinding seam
//   (src/Controls/src/Xaml/ApplyPropertiesVisitor.cs — "If value is BindingBase, SetBinding")
//
// The hook the apply pass calls when a markup extension's provide_value result is a binding_request
// (markup_extensions.hpp — the port's stand-in for C#'s Binding object). C# routes the BindingBase
// into BindableObject.SetBinding right inside TrySetPropertyValue; the port's runtime string-path
// binding engine is a PARALLEL M7 unit, so the route is a replaceable seam instead:
//
//   - the DEFAULT applier REJECTS with a clear xaml_parse_exception ("no binding applier is
//     registered") — a {Binding} in markup is a loud load failure rather than silently dropped
//     markup, exactly like the other documented M7 deferrals;
//   - the runtime-binding unit registers the real applier via set_xaml_binding_applier (it
//     resolves `request.path` against the target's binding context and wires bind() / set_binding,
//     C#'s bindable.SetBinding(property, binding)); a non-bindable target property must fail with
//     TrySetPropertyValue's catch-all "Cannot assign property …" (LoaderTests
//     .TestSetBindingToNonBindablePropertyShouldThrow).
//
// The applier receives everything TrySetBinding consumed: the property registry the load resolves
// against (its property_entry carries the bindable descriptor name SetBinding needs), the target
// object + its concrete type_tag, the XAML attribute name, the parsed binding_request, and the
// node's line info for the error channel. Throws xaml_parse_exception on failure — the visitor's
// guarded() routes it through the hydration ExceptionHandler knob like every other apply error.
//
// Process-wide, like the markup-extension/static registries (the C# analog is a static code path,
// not per-load state). set_xaml_binding_applier(nullptr) restores the rejecting default (test seam).

#include <functional>
#include <string>

#include "maui/core/bindable_object.hpp"
#include "maui/core/type_tag.hpp"
#include "maui/xaml/markup_extensions.hpp"
#include "maui/xaml/xaml_property_registry.hpp"

namespace maui::xaml
{
    using xaml_binding_applier = std::function<void(
        const xaml_property_registry& properties, maui::core::bindable_object& target, maui::core::type_tag target_type,
        const std::string& xaml_name, const binding_request& request, int line_number, int line_position)>;

    // The applier the apply pass currently routes {Binding} results through (never null — the
    // rejecting default until one is registered).
    [[nodiscard]] const xaml_binding_applier& current_xaml_binding_applier();

    // Register the runtime-binding unit's applier; nullptr restores the rejecting default.
    void set_xaml_binding_applier(xaml_binding_applier applier);
} // namespace maui::xaml
