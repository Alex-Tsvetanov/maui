#pragma once
// maui::controls::template_binding  <=  Microsoft.Maui.Controls.TemplateBinding
//
// Binds a property on an element inside a control template to a property on the element's TEMPLATED
// PARENT (the nearest ancestor templated control). Attached via element::set_template_binding (the
// C# `SetBinding(prop, new TemplateBinding(path, mode))` / `new Binding(path, source:
// RelativeBindingSource.TemplatedParent)`) and (re-)applied by the element whenever its ancestor
// chain changes — the port's synchronous equivalent of C#'s FindTemplatedParentAsync await.
//
// The C# string path maps onto the M5 typed-accessor doctrine: the factories take the TARGET and
// SOURCE property descriptors (the names the change notifications key on) plus typed accessor
// functions over the concrete templated-parent / target-control types. Writes use the C# binding
// specificities exactly:
//   - one_way pushes into the target at from_binding (a binding value);
//   - two_way pushes into the target at from_handler — C# BindableObject.SetBinding stores a
//     realized-TwoWay binding at SetterSpecificity.FromHandler — and writes BACK into the templated
//     parent at from_handler too (C# BindingExpression's AllowChaining path goes through
//     IElementController.SetValueFromRenderer), so a later non-handler set on either end (a manual
//     set, another binding) takes over, exactly as in MAUI (the DoubleTwoWayBindingWorks ladder).
// An out-of-scope element (no templated parent / wrong parent type) clears the from-binding value,
// restoring the target's default — the unresolvable-path behavior.
//
// Lifetime: the binding (and its scoped_connections on the parent's and target's property_changed)
// is owned by the TARGET element. The templated parent is an ancestor that owns the template subtree,
// so it outlives the target in normal teardown (PROFILE §8 — same doctrine as trigger.hpp).
//
// content_presenter uses the generic ctor for its TemplatedParent.Content pull (the C# ContentPresenter
// constructor binding); the runtime-binding unit can layer string paths over this same rebind seam.

#include <any>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/setter_specificity.hpp"

namespace maui::controls
{
    class template_binding
    {
    public:
        // The rebind seam: wire `target` against the resolved `templated_parent` (which may be null —
        // out of scope), parking every subscription in `connections` (cleared before each rebind).
        using rebind_fn = maui::core::move_only_function<void(maui::core::bindable_object& target,
                                                              maui::core::bindable_object* templated_parent,
                                                              std::vector<maui::core::scoped_connection>& connections)>;

        // The generic form (the content_presenter pull): `target_property_name` keys replacement in
        // element::set_template_binding (one binding per target property, like C#).
        template_binding(std::string target_property_name, rebind_fn rebind)
            : target_property_name_(std::move(target_property_name)), rebind_(std::move(rebind))
        {
        }

        // TemplateBinding(path) / Binding(path, source: TemplatedParent) — one-way: target gets
        // select(templated parent) at from_binding, re-pushed on the source property's change.
        template <class T, class TParent>
        [[nodiscard]] static template_binding one_way(const maui::core::bindable_property<T>& target_property,
                                                      const maui::core::bindable_property<T>& source_property,
                                                      std::function<T(const TParent&)> select)
        {
            return template_binding{
                std::string{target_property.name()},
                [target_name = target_property.name(), source_name = source_property.name(),
                 select = std::move(select)](maui::core::bindable_object& target,
                                             maui::core::bindable_object* templated_parent,
                                             std::vector<maui::core::scoped_connection>& connections) {
                    auto* parent = dynamic_cast<TParent*>(templated_parent);
                    if (parent == nullptr)
                    {
                        target.clear_setter(target_name, maui::core::setter_specificity::from_binding);
                        return;
                    }
                    auto push = [&target, parent, target_name, select] {
                        target.apply_setter(target_name, std::any{select(*parent)},
                                            maui::core::setter_specificity::from_binding);
                    };
                    push();
                    auto* parent_object = static_cast<maui::core::bindable_object*>(parent);
                    connections.push_back(maui::core::connect_scoped(parent_object->property_changed,
                                                                     [source_name, push](std::string_view name) {
                                                                         if (name == source_name)
                                                                         {
                                                                             push();
                                                                         }
                                                                     }));
                }};
        }

        // TemplateBinding(path, BindingMode.TwoWay): additionally writes the target's value back into
        // the templated parent. `select` reads the source off the parent, `read` reads the target's
        // current value off the concrete target control (both the typed-accessor stand-ins for the
        // C# path); both directions write at from_handler (see header comment).
        template <class T, class TTarget, class TParent>
        [[nodiscard]] static template_binding two_way(const maui::core::bindable_property<T>& target_property,
                                                      const maui::core::bindable_property<T>& source_property,
                                                      std::function<T(const TParent&)> select,
                                                      std::function<T(const TTarget&)> read)
        {
            return template_binding{
                std::string{target_property.name()},
                [target_name = target_property.name(), source_name = source_property.name(), select = std::move(select),
                 read = std::move(read)](maui::core::bindable_object& target,
                                         maui::core::bindable_object* templated_parent,
                                         std::vector<maui::core::scoped_connection>& connections) {
                    auto* parent = dynamic_cast<TParent*>(templated_parent);
                    if (parent == nullptr)
                    {
                        target.clear_setter(target_name, maui::core::setter_specificity::from_handler);
                        return;
                    }
                    // Re-entrancy guard shared by both directions (the same shape as core::bind's):
                    // value-equality no-ops already break the loop; the guard makes it robust to
                    // asymmetric accessors.
                    auto guard = std::make_shared<bool>(false);
                    auto push = [&target, parent, target_name, select, guard] {
                        if (*guard)
                        {
                            return;
                        }
                        *guard = true;
                        target.apply_setter(target_name, std::any{select(*parent)},
                                            maui::core::setter_specificity::from_handler);
                        *guard = false;
                    };
                    push();
                    auto* parent_object = static_cast<maui::core::bindable_object*>(parent);
                    connections.push_back(maui::core::connect_scoped(parent_object->property_changed,
                                                                     [source_name, push](std::string_view name) {
                                                                         if (name == source_name)
                                                                         {
                                                                             push();
                                                                         }
                                                                     }));
                    connections.push_back(maui::core::connect_scoped(
                        target.property_changed,
                        [&target, parent_object, target_name, source_name, read, guard](std::string_view name) {
                            if (name != target_name || *guard)
                            {
                                return;
                            }
                            auto* typed_target = dynamic_cast<TTarget*>(&target);
                            if (typed_target == nullptr)
                            {
                                return;
                            }
                            *guard = true;
                            parent_object->apply_setter(source_name, std::any{read(*typed_target)},
                                                        maui::core::setter_specificity::from_handler);
                            *guard = false;
                        }));
                }};
        }

        // The target property this binding writes (element::set_template_binding replaces by it).
        [[nodiscard]] std::string_view target_property_name() const
        {
            return target_property_name_;
        }

        // (Re-)wire against the resolved templated parent; null = out of scope (subscriptions dropped,
        // bound value cleared by the rebind body). Called by element::reapply_template_bindings.
        void apply(maui::core::bindable_object& target, maui::core::bindable_object* templated_parent)
        {
            connections_.clear();
            rebind_(target, templated_parent, connections_);
        }

    private:
        std::string target_property_name_;
        rebind_fn rebind_;
        std::vector<maui::core::scoped_connection> connections_;
    };
} // namespace maui::controls
