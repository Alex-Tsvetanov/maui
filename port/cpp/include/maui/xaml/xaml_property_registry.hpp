#pragma once
// maui::xaml::xaml_property_registry — per-type XAML property + content metadata (M7 wave 1).
//
// C# counterpart: ApplyPropertiesVisitor (src/Controls/src/Xaml/ApplyPropertiesVisitor.cs) — its
// GetBindableProperty finds the static `{Name}Property` field by reflection and TrySetValue routes
// through BindableObject.SetValue; GetContentPropertyName reads the [ContentProperty] attribute
// (walking base types) so element content lands on the right member. No reflection here (PROFILE §6),
// so each control's XAML surface is REGISTERED explicitly, keyed by the concrete control's type_tag
// (from the xaml_type_registry registration) + the XAML attribute name (PascalCase, exactly as it
// appears in markup). C#'s base-type walks (inherited properties, inherited [ContentProperty]) are
// FLATTENED into each concrete type's registration instead — the standard registrations re-register
// the shared view<> surface per control via the same shared descriptors, so no inheritance walk is
// needed at lookup time.
//
// Two registration routes, matching how the controls expose their members:
//   - register_bindable_property: a BINDABLE property routes through the existing
//     bindable_object::apply_setter seam (the typed-property channel every style/trigger/VSM setter
//     uses) at setter_specificity::manual_value_setter — C#'s TrySetValue calls the plain
//     BindableObject.SetValue, i.e. SetterSpecificity.ManualValueSetter, so a XAML-set value outranks
//     styles and is itself overridable by a later manual set. The boxed std::any crosses the boundary
//     once; storage stays typed in property<T>.
//   - register_property: a NON-bindable member (window title, …) is an explicit typed lambda calling
//     the control's typed API; the wrapper downcasts the target and unboxes the value.
//
// Error strategy (xaml_parse_exception.hpp): lookups are throw-free — find()/content_property()
// return nullptr and try_set()/try_add_child() return false on a miss, mirroring C#'s
// TrySetPropertyValue bool + out-exception convention; the M7 loader turns a false into a thrown
// xaml_parse_exception. A boxed value whose type does not match the property's value type also
// reports false rather than throwing — C# TrySetValue pre-checks ReturnType.IsInstanceOfType before
// SetValue for exactly this reason. (On the bindable route, a name-routed apply_setter against an
// object that lacks the property is a silent no-op — like C# SetValue accepting any BindableObject —
// so pairing the tag with the right instance is the loader's contract.)
//
// ContentProperty metadata ([ContentProperty]) is ported as explicit per-type registrations in the
// two shapes the attribute takes in practice:
//   - a VALUE content property (Label: [ContentProperty(nameof(Text))]) — the attribute name element
//     text is routed to, via register_content_property / content_property();
//   - a CHILD sink (ContentPage→set_content, Layout→add, Window→set_content, NavigationPage→push) —
//     an add_child function registered per container type, via register_add_child / try_add_child.

#include <any>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/type_tag.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"

namespace maui::xaml
{
    class xaml_property_registry
    {
    public:
        // The type-erased application of one XAML attribute value to a target object. Returns false
        // when the value (or the target, on the typed route) is not what the registration expects —
        // the port of C# TrySetValue's bool result.
        using setter = std::function<bool(maui::core::bindable_object& target, const std::any& value)>;

        // How a parsed child element is attached to a container ([ContentProperty] child sink).
        // Returns false when the child is not of the type the container accepts.
        using add_child_fn =
            std::function<bool(maui::core::bindable_object& parent, maui::core::bindable_object& child)>;

        struct property_entry
        {
            setter set;                      // apply the boxed value to the target
            maui::core::type_tag value_type; // the T the std::any must hold; names the converter implicitly
            // The backing bindable_property descriptor's name (the apply_setter / set_dynamic_resource
            // routing key); empty for a non-bindable registration. The M7 loader's DynamicResource
            // path needs it — C# TrySetDynamicResource requires the BindableProperty the reflection
            // lookup produced, and element::set_dynamic_resource keys on the descriptor name.
            std::string_view bindable_name;
        };

        // ---- registration ------------------------------------------------------------------------

        // Route a BINDABLE property through bindable_object::apply_setter (see the header comment).
        // `xaml_name` is the markup attribute ("Text"); the descriptor supplies the snake_case
        // property name + value type. The descriptor must be one of the shared static descriptors
        // (its name string and itself outlive the registry).
        template <class TControl, class T>
        void register_bindable_property(std::string xaml_name, const maui::core::bindable_property<T>& descriptor)
        {
            const std::string_view property_name = descriptor.name();
            add_property(
                maui::core::type_tag::of<TControl>(), std::move(xaml_name),
                property_entry{.set =
                                   [property_name](maui::core::bindable_object& target, const std::any& value) {
                                       if (std::any_cast<T>(&value) == nullptr)
                                       {
                                           return false; // C# ReturnType.IsInstanceOfType pre-check
                                       }
                                       target.apply_setter(property_name, value,
                                                           maui::core::setter_specificity::manual_value_setter);
                                       return true;
                                   },
                               .value_type = maui::core::type_tag::of<T>(),
                               .bindable_name = property_name});
        }

        // Register a NON-bindable member as an explicit typed lambda calling the control's typed API.
        // `set` is any callable `void(TControl&, const T&)`; the wrapper downcasts/unboxes and
        // reports false on a mismatch.
        template <class TControl, class T, class F> void register_property(std::string xaml_name, F set)
        {
            add_property(
                maui::core::type_tag::of<TControl>(), std::move(xaml_name),
                property_entry{.set =
                                   [set = std::move(set)](maui::core::bindable_object& target, const std::any& value) {
                                       auto* control = dynamic_cast<TControl*>(&target);
                                       const T* typed = std::any_cast<T>(&value);
                                       if (control == nullptr || typed == nullptr)
                                       {
                                           return false;
                                       }
                                       set(*control, *typed);
                                       return true;
                                   },
                               .value_type = maui::core::type_tag::of<T>()});
        }

        // The fully-raw primitive (used by the typed forms above; handy for test fakes): register an
        // already type-erased setter plus the value type its std::any must hold.
        template <class TControl>
        void register_property(std::string xaml_name, setter set, maui::core::type_tag value_type)
        {
            add_property(maui::core::type_tag::of<TControl>(), std::move(xaml_name),
                         property_entry{.set = std::move(set), .value_type = value_type});
        }

        // A property whose XAML LITERAL is a plain string (boxed by `box_set` into the control's
        // object-ish target — e.g. CollectionView.Header, a boxed_item) but which ALSO supports a
        // {Binding}: `bindable_name` is the backing bindable_property's name, so the M7 binding applier
        // routes Header="{Binding …}" through element::set_binding (the bound value must already match
        // the property's type at runtime). Literals behave EXACTLY as the plain register_property<…,string>
        // form (value_type stays std::string, boxed on set), so pages using a literal Header/Footer are
        // unaffected; only the {Binding} path is added.
        template <class TControl>
        void register_string_literal_bindable(std::string xaml_name, std::string_view bindable_name,
                                              std::function<void(TControl&, const std::string&)> box_set)
        {
            add_property(maui::core::type_tag::of<TControl>(), std::move(xaml_name),
                         property_entry{.set =
                                            [box_set = std::move(box_set)](maui::core::bindable_object& target,
                                                                           const std::any& value) {
                                                auto* control = dynamic_cast<TControl*>(&target);
                                                const std::string* text = std::any_cast<std::string>(&value);
                                                if (control == nullptr || text == nullptr)
                                                {
                                                    return false;
                                                }
                                                box_set(*control, *text);
                                                return true;
                                            },
                                        .value_type = maui::core::type_tag::of<std::string>(),
                                        .bindable_name = bindable_name});
        }

        // [ContentProperty("Text")]-style VALUE content: name the XAML attribute element text routes to.
        template <class TControl> void register_content_property(std::string xaml_name)
        {
            set_content_property(maui::core::type_tag::of<TControl>(), std::move(xaml_name));
        }

        // [ContentProperty] CHILD sink: how a parsed child element is attached to TParent. `add` is
        // any callable `bool(TParent&, maui::core::bindable_object& child)` — it downcasts the child
        // to what the container accepts and returns false otherwise.
        template <class TParent, class F> void register_add_child(F add)
        {
            set_add_child(
                maui::core::type_tag::of<TParent>(),
                [add = std::move(add)](maui::core::bindable_object& parent, maui::core::bindable_object& child) {
                    auto* typed_parent = dynamic_cast<TParent*>(&parent);
                    return typed_parent != nullptr && add(*typed_parent, child);
                });
        }

        // The named variant: also record the CLR collection-property name the sink stands in for
        // (Layout "Children", ContentPage "Content", Window "Page"), so the loader can route the
        // property-element spelling — <StackLayout.Children><Label/></…> — through the same sink.
        // C# reaches those through GetRuntimeProperties + the IEnumerable Add() walk
        // (ApplyPropertiesVisitor.TryAddToProperty); the reflection-free port names them explicitly.
        template <class TParent, class F> void register_add_child(std::string property_name, F add)
        {
            register_add_child<TParent>(std::move(add));
            set_child_property_name(maui::core::type_tag::of<TParent>(), std::move(property_name));
        }

        // ---- lookup / application (all throw-free; see the error strategy above) -------------------

        // The property registered for (type, xaml_name), or nullptr. The pointer stays valid across
        // later registrations (node-based maps), until that name is replaced.
        [[nodiscard]] const property_entry* find(maui::core::type_tag type, std::string_view xaml_name) const;

        // Apply an already-typed boxed value: false on unknown type/property or a value/target mismatch.
        [[nodiscard]] bool try_set(maui::core::type_tag type, maui::core::bindable_object& target,
                                   std::string_view xaml_name, const std::any& value) const;

        // Convert-then-apply: resolve the property, convert `text` with the converter its value type
        // names implicitly (see xaml_converter_registry), and apply. False on unknown property, no
        // converter for the value type, or a mismatch; propagates the converter's
        // xaml_parse_exception on a malformed literal.
        [[nodiscard]] bool try_set_from_text(maui::core::type_tag type, maui::core::bindable_object& target,
                                             std::string_view xaml_name, const std::string& text,
                                             const xaml_converter_registry& converters) const;

        // The VALUE content-property name for `type`, or nullptr when it has none (C#
        // GetContentPropertyName returns null — the loader's "cannot set content" error).
        [[nodiscard]] const std::string* content_property(maui::core::type_tag type) const;

        // Attach `child` to `parent` through the container's registered child sink: false when the
        // parent type has none, the parent instance is not that type, or the child type is rejected.
        [[nodiscard]] bool try_add_child(maui::core::type_tag parent_type, maui::core::bindable_object& parent,
                                         maui::core::bindable_object& child) const;

        // Whether `xaml_name` is the registered child-sink property name of `type` (the
        // <Type.Children> / <Type.Content> property-element spelling — see the named
        // register_add_child overload).
        [[nodiscard]] bool is_child_property(maui::core::type_tag type, std::string_view xaml_name) const;

    private:
        struct per_type
        {
            std::unordered_map<std::string, property_entry> properties;
            std::string content_property; // empty = none ([ContentProperty] absent)
            add_child_fn add_child;       // null = not a child container
            std::string child_property;   // the sink's CLR property name; empty = collection-items only
        };

        // The non-template insertion paths the registration templates lower onto (defined in the .cpp).
        void add_property(maui::core::type_tag type, std::string xaml_name, property_entry entry);
        void set_content_property(maui::core::type_tag type, std::string xaml_name);
        void set_add_child(maui::core::type_tag type, add_child_fn add);
        void set_child_property_name(maui::core::type_tag type, std::string xaml_name);

        std::unordered_map<maui::core::type_tag, per_type> types_;
    };

    // The process-wide default registry (Meyers singleton), the property sibling of
    // default_xaml_type_registry().
    [[nodiscard]] xaml_property_registry& default_xaml_property_registry();
} // namespace maui::xaml
