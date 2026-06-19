#pragma once
// maui::controls::data_template  <=  Microsoft.Maui.Controls.DataTemplate (+ IDataTemplateController)
//
// The visual structure for templated items: an element_template plus the Values/Bindings staging the
// C# DataTemplate carries — property assignments and bindings applied to every created content
// (SetupContent = ApplyBindings then ApplyValues).
//
//   - Values  (IDictionary<BindableProperty, object>)      → name → boxed value, applied through
//     bindable_object::apply_setter at manual specificity (C# bindable.SetValue).
//   - Bindings (IDictionary<BindableProperty, BindingBase>) → name → a CONTEXT-binding applier. The
//     port's stand-in for `new Binding(path)` is the M5 typed-accessor doctrine: a typed selector
//     over the content's BindingContext (`set_binding(prop, [](const vm& v){ return v.name; })`);
//     the no-selector overload is `new Binding(".")` (the context itself is the value). Each applier
//     pushes at from_binding specificity now and on every BindingContextChanged of the created
//     content — the subscription lives exactly as long as the content (the C# binding is stored in
//     the object's own property store). The general string-path BindingBase slots into this same
//     name→applier seam when the runtime-binding unit lands (see xaml/markup_extensions.hpp).
//
// set_binding/set_value mirror C#'s mutual removal (SetBinding drops a staged value and vice versa);
// add_binding/add_value are the raw dictionary Adds (the C# collection initializers), which is what
// makes the "Binding and Value found for X" InvalidOperationException in create_content reachable.
//
// IDataTemplateController surface: id() (a process-wide counter starting above 100, like the C#
// idCounter) and id_string() — the recycling reuse-identifier. C# uses Type.FullName for the Type
// ctor so every DataTemplate(typeof(Foo)) shares one IdString; the reflection-free port mints one
// stable per-TControl string inside of<TControl>() (same type ⇒ same id_string), and a per-instance
// string otherwise — the semantics the collections layer keys on, without reflection names.

#include <any>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/templates/element_template.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::controls
{
    class data_template : public element_template
    {
    public:
        // DataTemplate() — only meaningful for XAML/HotReload staging in C#; create_content on a
        // loader-less template returns the label fallback (element_template).
        data_template();
        // DataTemplate(Func<object>) — throws std::invalid_argument on a null loader (C#
        // ArgumentNullException).
        explicit data_template(loader load_template);

        // DataTemplate(Type) — the reflection-free stand-in: load via make_shared<TControl>, mark
        // recyclable, and share one id_string per TControl (C# type.FullName). Also captures the
        // TControl type_tag (content_type()) so a native cell can create_handler<TControl>() and host
        // the realized content's native view — the reflection-free analog of the C# Type ctor letting
        // TemplatedCell2 do `CreateContent(...) as View` then `view.ToPlatform(mauiContext)`.
        template <class TControl> [[nodiscard]] static std::shared_ptr<data_template> of()
        {
            static const std::string type_id_string = make_type_id_string();
            return std::shared_ptr<data_template>(new data_template(
                [] { return std::static_pointer_cast<maui::core::bindable_object>(std::make_shared<TControl>()); },
                type_id_string, maui::core::type_tag::of<TControl>()));
        }

        // ---- IDataTemplateController ----
        [[nodiscard]] int id() const
        {
            return id_;
        }
        [[nodiscard]] const std::string& id_string() const
        {
            return id_string_;
        }

        // The TControl type_tag of a type-activated template (of<TControl>()), or nullopt for a
        // loader-only / selector template. A native backend uses it to create_handler<TControl>() and
        // realize the created content's native view inside a cell (the C# `view.ToPlatform(mauiContext)`
        // step). Loader-only templates carry no static control type, so the native cell falls back to
        // the item text mirror (as before) — documented, and exercised only by type-activated templates.
        [[nodiscard]] const std::optional<maui::core::type_tag>& content_type() const
        {
            return content_type_;
        }

        // ---- Values (DataTemplate.SetValue / the Values dictionary) ----
        // SetValue: stage `descriptor` = `value` on every created content (manual specificity) and
        // drop a staged binding for the same property (C# Bindings.Remove). The descriptor reference
        // API cannot express C#'s null-property ArgumentNullException — that guard is structural here.
        template <class T> void set_value(const maui::core::bindable_property<T>& descriptor, T value)
        {
            bindings_.erase(std::string{descriptor.name()});
            add_value(descriptor, std::move(value));
        }
        // The raw Values.Add (the C# collection initializer) — no counterpart removal, so a property
        // staged in BOTH dictionaries makes create_content throw (SetValueAndBinding).
        template <class T> void add_value(const maui::core::bindable_property<T>& descriptor, T value)
        {
            values_.insert_or_assign(std::string{descriptor.name()}, std::any{std::move(value)});
        }

        // ---- Bindings (DataTemplate.SetBinding / the Bindings dictionary) ----
        // SetBinding with a typed context selector (`new Binding("Path")` as a typed accessor): on
        // every created content, descriptor = select(*binding_context<TContext>()) at from_binding,
        // re-applied on each context change; an absent/mismatched context clears the bound value
        // (the binding resolves to the property default, like an unresolvable C# path).
        template <class T, class TContext>
        void set_binding(const maui::core::bindable_property<T>& descriptor, std::function<T(const TContext&)> select)
        {
            values_.erase(std::string{descriptor.name()});
            add_binding(descriptor, std::move(select));
        }
        // SetBinding for the self path (`new Binding(".")`): the context itself is the value.
        template <class T> void set_binding(const maui::core::bindable_property<T>& descriptor)
        {
            set_binding<T, T>(descriptor, [](const T& context) { return context; });
        }
        // The raw Bindings.Add (the C# collection initializer) — no counterpart removal.
        template <class T, class TContext>
        void add_binding(const maui::core::bindable_property<T>& descriptor, std::function<T(const TContext&)> select)
        {
            bindings_.insert_or_assign(std::string{descriptor.name()},
                                       make_context_applier<T, TContext>(descriptor.name(), std::move(select)));
        }
        template <class T> void add_binding(const maui::core::bindable_property<T>& descriptor)
        {
            add_binding<T, T>(descriptor, [](const T& context) { return context; });
        }

    protected:
        // DataTemplate.SetupContent: ApplyBindings (throwing std::runtime_error — C#
        // InvalidOperationException — when the property is also staged in Values), then ApplyValues.
        void setup_content(maui::core::bindable_object& item) const override;

    private:
        // A staged binding: applied once to each created content.
        using binding_applier = std::function<void(maui::core::bindable_object&)>;

        // The type-activated ctor used by of<TControl>() — also records the TControl type_tag so a
        // native cell can resolve the matching handler for the realized content.
        data_template(loader load_template, std::string id_string, maui::core::type_tag content_type);
        [[nodiscard]] static int next_id();
        [[nodiscard]] static std::string make_type_id_string();

        // Build the context-observing applier for one staged binding: push now, then on every
        // BindingContextChanged of the content. The subscription is deliberately untracked — it lives
        // in the content's own event, i.e. exactly as long as the content (the C# binding lifetime).
        template <class T, class TContext>
        [[nodiscard]] static binding_applier make_context_applier(std::string_view name,
                                                                  std::function<T(const TContext&)> select)
        {
            return [name, select = std::move(select)](maui::core::bindable_object& content) {
                auto push = [name, select](maui::core::bindable_object& target) {
                    if (const auto context = target.binding_context<TContext>())
                    {
                        target.apply_setter(name, std::any{select(*context)},
                                            maui::core::setter_specificity::from_binding);
                    }
                    else
                    {
                        target.clear_setter(name, maui::core::setter_specificity::from_binding);
                    }
                };
                push(content);
                content.binding_context_changed.connect([&content, push] { push(content); });
            };
        }

        int id_;                // DataTemplate._id (idCounter starts at 100; first id is 101)
        std::string id_string_; // DataTemplate._idString (the recycling reuse-identifier)
        // The static control type of a type-activated template (of<TControl>()), used by a native cell
        // to create the matching handler for the realized content (nullopt for loader-only templates).
        std::optional<maui::core::type_tag> content_type_;
        // Ordered (std::map) so create_content applies deterministically; keys are descriptor names.
        std::map<std::string, std::any, std::less<>> values_;
        std::map<std::string, binding_applier, std::less<>> bindings_;
    };
} // namespace maui::controls
