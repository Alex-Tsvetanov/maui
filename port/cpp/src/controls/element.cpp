// maui::controls::element — tree-lifecycle propagation + resources / DynamicResource / implicit styles
// (element.hpp). Ported from Element.cs (Parent / OnChildAdded / OnResourcesChanged / OnSetDynamicResource),
// ResourcesExtensions.TryGetResource, and MergedStyle (via merged_style_).
#include "maui/controls/element.hpp"

#include <any>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/resource_dictionary.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/event.hpp"
#include "maui/core/setter_specificity.hpp"

namespace maui::controls
{
    void element::on_binding_context_changed()
    {
        maui::core::bindable_object::on_binding_context_changed(); // raise binding_context_changed
        const auto& context = raw_binding_context();
        for_each_logical_child([&context](element& child) { child.set_inherited_binding_context(context); });
    }

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
        child.logical_parent_ = this;
        child.set_inherited_binding_context(raw_binding_context());
        child.set_containing_window(window_);
        // Now that the child is in our subtree, its resource chain includes ours: re-resolve its (and its
        // descendants') DynamicResources + implicit styles against the extended chain.
        child.reapply_resources_from_chain();
    }

    void element::detach_logical_child(element& child)
    {
        child.set_containing_window(nullptr);
        child.logical_parent_ = nullptr;
        // The child's last inherited BindingContext is kept (C# leaves it until reparented/reset), but its
        // resource chain shrank to itself — re-resolve so an implicit style from the old parent un-applies.
        child.reapply_resources_from_chain();
    }
} // namespace maui::controls
