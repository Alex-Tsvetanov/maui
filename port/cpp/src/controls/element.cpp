// maui::controls::element — tree-lifecycle propagation + resources / DynamicResource / implicit styles
// (element.hpp). Ported from Element.cs (Parent / OnChildAdded / OnResourcesChanged / OnSetDynamicResource),
// ResourcesExtensions.TryGetResource, and MergedStyle (via merged_style_).
#include "maui/controls/element.hpp"

#include <any>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/bindings/binding.hpp" // runtime bindings (W1-02): the string-path overload
#include "maui/controls/bindings/binding_base.hpp"
#include "maui/controls/bindings/binding_diagnostics.hpp"
#include "maui/controls/resource_dictionary.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/event.hpp"
#include "maui/core/setter_specificity.hpp"

// --- animations (W1-14) ---
#include <algorithm>

#include "detail/element_animations.hpp"
// --- end animations (W1-14) ---

namespace maui::controls
{
    void element::on_binding_context_changed()
    {
        // runtime bindings (W1-02): re-apply this element's own bindings against the new context
        // FIRST (C# BindingContextPropertyChanged → ApplyBindings before OnBindingContextChanged).
        reapply_bindings(/*from_binding_context_changed=*/true);
        maui::core::bindable_object::on_binding_context_changed(); // raise binding_context_changed
        const auto& context = raw_binding_context();
        // --- templates (W1-09): route through the overridable SetChildInheritedBindingContext seam ---
        for_each_logical_child(
            [this, &context](element& child) { set_child_inherited_binding_context(child, context); });
    }

    // --- runtime bindings (W1-02) -------------------------------------------------------------
    // (the destructor — binding unapply + template_bindings_ teardown — lives in
    // element_templates.cpp, where template_binding is a complete type)

    void element::set_binding(const std::string& property_name, std::shared_ptr<binding_base> binding)
    {
        if (!binding)
        {
            throw std::invalid_argument("element::set_binding: binding is null");
        }
        // BindableObject.SetBinding: a OneWay binding on a read-only property is refused (logged).
        if (property_is_read_only(property_name).value_or(false) &&
            binding->mode() == maui::core::binding_mode::one_way)
        {
            send_binding_failure("element::set_binding: cannot set a OneWay binding on read-only property '" +
                                 property_name + "'");
            return;
        }
        // The realized mode picks the apply specificity: TwoWay applies at from_handler (the
        // dotnet/maui#16849 rule — a later manual set removes it, a binding update reinstates it),
        // everything else at from_binding.
        maui::core::binding_mode realized = binding->mode();
        if (realized == maui::core::binding_mode::default_mode)
        {
            realized = property_default_binding_mode(property_name).value_or(maui::core::binding_mode::one_way);
        }
        if (realized == maui::core::binding_mode::two_way && property_is_read_only(property_name).value_or(false))
        {
            realized = maui::core::binding_mode::one_way_to_source;
        }
        const maui::core::setter_specificity specificity = realized == maui::core::binding_mode::two_way
                                                               ? maui::core::setter_specificity::from_handler
                                                               : maui::core::setter_specificity::from_binding;

        // A value above from_binding is silently demoted so the binding's first apply replaces it.
        demote_value_to_binding(property_name);

        if (auto it = bindings_.find(property_name); it != bindings_.end())
        {
            it->second.binding->unapply();
        }
        bound_property& bound = bindings_[property_name];
        bound.binding = std::move(binding);
        bound.specificity = specificity;
        bound.binding->apply(raw_binding_context(), *this, property_name, /*from_binding_context_changed=*/false,
                             specificity);
    }

    void element::set_binding(const std::string& property_name, std::string path, maui::core::binding_mode mode)
    {
        set_binding(property_name, std::make_shared<class binding>(std::move(path), mode));
    }

    void element::remove_binding(std::string_view property_name)
    {
        if (auto it = bindings_.find(std::string{property_name}); it != bindings_.end())
        {
            it->second.binding->unapply();
            bindings_.erase(it);
        }
    }

    std::shared_ptr<binding_base> element::binding_for(std::string_view property_name) const
    {
        if (auto it = bindings_.find(std::string{property_name}); it != bindings_.end())
        {
            return it->second.binding;
        }
        return nullptr;
    }

    void element::reapply_bindings(bool from_binding_context_changed)
    {
        // Snapshot (C# copies the property contexts): an apply may mutate the binding map.
        std::vector<std::pair<std::string, bound_property>> entries;
        entries.reserve(bindings_.size());
        for (const auto& [name, bound] : bindings_)
        {
            entries.emplace_back(name, bound);
        }
        for (const auto& [name, bound] : entries)
        {
            bound.binding->unapply(from_binding_context_changed);
            bound.binding->apply(raw_binding_context(), *this, name, from_binding_context_changed, bound.specificity);
        }
    }

    // --- end runtime bindings (W1-02) -----------------------------------------------------------

    void element::set_containing_window(window* value)
    {
        if (window_ == value)
        {
            return;
        }
        const bool was_attached = window_ != nullptr;
        window_ = value;
        if (value != nullptr && !was_attached)
        {
            loaded.raise(); // attach: this element is now in a window — fire before children attach
        }
        for_each_logical_child([value](element& child) { child.set_containing_window(value); });
        if (value == nullptr && was_attached)
        {
            unloaded.raise(); // detach: children have detached first; this element leaves the window last
        }
    }

    resource_dictionary& element::resources()
    {
        if (!resources_)
        {
            resources_ = std::make_unique<resource_dictionary>();
            // A change to our own dictionary re-applies bound DynamicResources + the merged style here, then
            // propagates the changed keys down to children (Element.OnResourcesChanged).
            resources_token_ = maui::core::connect_scoped(
                resources_->values_changed,
                [this](const std::vector<resource_change>& values) { on_resources_changed(values); });
        }
        return *resources_;
    }

    const std::any* element::try_get_resource(std::string_view key) const
    {
        // ResourcesExtensions.TryGetResource: walk self → ancestors; the first dictionary that has the key
        // wins (so a closer scope overrides a farther one).
        for (const element* current = this; current != nullptr; current = current->logical_parent_)
        {
            if (current->resources_)
            {
                if (const std::any* value = current->resources_->try_get(key))
                {
                    return value;
                }
            }
        }
        return nullptr;
    }

    void element::set_dynamic_resource(std::string name, std::string key)
    {
        // Record the binding (Element.DynamicResources) then resolve it now (OnSetDynamicResource →
        // OnResourceChanged if the key is currently resolvable; an absent key resolves later via the change
        // notification). Re-binding the same property updates the key.
        const std::string property_name = name;
        dynamic_resources_.insert_or_assign(std::move(name), key);
        if (const std::any* value = try_get_resource(key))
        {
            apply_setter(property_name, *value, maui::core::setter_specificity::dynamic_resource_setter);
        }
    }

    void element::remove_dynamic_resource(std::string_view name)
    {
        // C# only stops future updates (the already-applied value is kept), so just drop the binding.
        dynamic_resources_.erase(std::string{name});
    }

    void element::apply_dynamic_resources(const std::vector<resource_change>* keys)
    {
        if (dynamic_resources_.empty())
        {
            return;
        }
        for (const auto& [property_name, resource_key] : dynamic_resources_)
        {
            // When `keys` is supplied, only re-apply bindings whose resource key is among the changed keys
            // (Element.OnResourcesChanged matches dynR.Value.Item1 == value.Key); otherwise re-apply all.
            if (keys != nullptr)
            {
                bool affected = false;
                for (const resource_change& change : *keys)
                {
                    if (change.key == resource_key)
                    {
                        affected = true;
                        break;
                    }
                }
                if (!affected)
                {
                    continue;
                }
            }
            if (const std::any* value = try_get_resource(resource_key))
            {
                apply_setter(property_name, *value, maui::core::setter_specificity::dynamic_resource_setter);
            }
        }
    }

    void element::on_resources_changed(const std::vector<resource_change>& values)
    {
        // Re-apply our own affected DynamicResources + re-resolve the merged style (an implicit/class/base
        // style we point at may have changed) + the local style's base-by-key, then propagate DOWN to
        // logical children.
        apply_dynamic_resources(&values);
        merged_style_.refresh();
        on_resource_chain_changed();
        for_each_logical_child([&values](element& child) {
            // A key the CHILD defines in its OWN dictionary shadows the parent's, so it is not forwarded
            // (ResourcesChangedNotRaisedIfKeyExistsInCurrent). Filter those out before recursing.
            std::vector<resource_change> forwarded;
            forwarded.reserve(values.size());
            for (const resource_change& change : values)
            {
                if (!child.resources_ || !child.resources_->contains_key(change.key))
                {
                    forwarded.push_back(change);
                }
            }
            if (!forwarded.empty())
            {
                child.on_resources_changed(forwarded);
            }
        });
    }

    void element::reapply_resources_from_chain()
    {
        // The element's ancestor chain changed (a reparent): every bound DynamicResource may now resolve to a
        // different (or newly-available) value, the merged style's implicit/class lookup may have moved, and a
        // local style's base_resource_key may now resolve.
        apply_dynamic_resources(nullptr);
        merged_style_.refresh();
        on_resource_chain_changed();
        // --- templates (W1-09): the ancestor chain also defines the TEMPLATE scope — re-resolve the
        // templated parent and re-apply any template bindings before recursing ---
        reapply_template_bindings();
        for_each_logical_child([](element& child) { child.reapply_resources_from_chain(); });
    }

    void element::refresh_merged_style()
    {
        merged_style_.refresh();
    }

    void element::set_merged_style_classes(std::vector<std::string> classes)
    {
        merged_style_.set_style_classes(std::move(classes));
    }

    void element::attach_logical_child(element& child)
    {
        child.on_logical_parent_changing(this); // Element.OnParentChangingCore (ImmutableBrush throws here)
        child.logical_parent_ = this;
        // --- templates (W1-09): route through the overridable SetChildInheritedBindingContext seam ---
        set_child_inherited_binding_context(child, raw_binding_context());
        child.set_containing_window(window_);
        // Now that the child is in our subtree, its resource chain includes ours: re-resolve its (and its
        // descendants') DynamicResources + implicit styles against the extended chain.
        child.reapply_resources_from_chain();
        child.parent_set.raise(); // runtime bindings (W1-02): Element.ParentSet — ancestry re-resolution
    }

    void element::detach_logical_child(element& child)
    {
        child.on_logical_parent_changing(nullptr); // Element.OnParentChangingCore (ImmutableBrush throws here)
        child.set_containing_window(nullptr);
        child.logical_parent_ = nullptr;
        // The child's resource chain shrank to itself — re-resolve so an implicit style from the old
        // parent un-applies.
        child.reapply_resources_from_chain();
        // runtime bindings (W1-02), corrected against Element.cs SetParent(null): detaching CLEARS the
        // child's INHERITED binding context (`SetInheritedBindingContext(this, null)`) — an explicitly
        // set context survives via the set_inherited_binding_context guard. The relative-source
        // ancestor tests (RelativeSourceBindingTests.cs) pin this: a detached subtree loses the
        // contexts it inherited from the old ancestors.
        child.set_inherited_binding_context(maui::core::bindable_object::binding_context_box{});
        child.parent_set.raise(); // runtime bindings (W1-02): Element.ParentSet — ancestry re-resolution
    }

    // --- animations (W1-14) ---
    // VisualElement.BatchBegin / BatchCommit: nest a counter; the last commit raises batch_committed.
    void element::batch_begin()
    {
        ++batched_;
    }

    void element::batch_commit()
    {
        batched_ = std::max(0, batched_ - 1);
        if (!batched())
        {
            batch_committed.raise();
        }
    }

    detail::element_animations& element::animation_state()
    {
        if (!animation_state_)
        {
            animation_state_ = std::make_shared<detail::element_animations>();
        }
        return *animation_state_;
    }
    // --- end animations (W1-14) ---

    // --- styles tail (W1-15) ---
    // The named-event registrar (EventTrigger's reflection-free seam — see element.hpp).
    void element::register_named_event(std::string name,
                                       std::function<maui::core::scoped_connection(std::function<void()>)> subscribe)
    {
        named_events_.insert_or_assign(std::move(name), std::move(subscribe));
    }

    maui::core::scoped_connection element::connect_named_event(std::string_view name, std::function<void()> handler)
    {
        if (const auto it = named_events_.find(std::string{name}); it != named_events_.end())
        {
            return it->second(std::move(handler));
        }
        return {}; // unknown event name — attach nothing (C# logs a warning)
    }
    // --- end styles tail (W1-15) ---
} // namespace maui::controls
