#pragma once
// maui::controls::element  <=  Microsoft.Maui.Controls.Element / VisualElement (lifecycle subset)
//
// The non-template base between bindable_object (the value / binding-context layer) and view<> (the
// handler / geometry layer). It carries the cross-cutting *tree lifecycle* every control shares:
//   - logical-children visitation (the port's stand-in for Element.LogicalChildren) — a container
//     overrides for_each_logical_child to expose its children; leaves keep the no-op default;
//   - the logical PARENT back-reference (Element.Parent) — set when a container attaches a child, so the
//     resource lookup + implicit-style match can walk UP the tree;
//   - BindingContext INHERITANCE — on_binding_context_changed propagates this element's context down to
//     each logical child (the role of Element.OnBindingContextChanged → SetChildInheritedBindingContext);
//   - the Window back-reference + Loaded/Unloaded events (VisualElement.Window + Loaded/Unloaded) — when a
//     root is hosted in a window, set_containing_window flows the window down the subtree;
//   - RESOURCES + DynamicResource + implicit styles (M5d): a lazily-created resource_dictionary (the
//     IResourcesProvider role), try_get_resource walking self→ancestors (ResourcesExtensions.TryGetResource),
//     set/remove_dynamic_resource binding a property to a resource key (re-applied when the resource changes
//     or the element reparents), and a merged_style resolving the implicit (TargetType-keyed) + class styles.
// attach_logical_child / detach_logical_child are the hooks a container calls when it gains / loses a child
// so the child inherits (or loses) the parent's context + window + resources (Element.OnChildAdded/Removed).
//
// Scope (M5d): the propagation machinery + resources/DynamicResource/implicit styles. Effects, the visual-
// vs-logical split, platform loaded-event wiring, modal stacks, system resources, and style sheets are out
// of scope (STATUS.md).

#include <any>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "maui/controls/merged_style.hpp"
#include "maui/controls/resource_dictionary.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/event.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::controls
{
    class window; // forward — the host; element only holds a non-owning back-ref

    class element : public maui::core::bindable_object
    {
    public:
        // ---- Window back-ref + Loaded/Unloaded (VisualElement.Window / Loaded / Unloaded) ----
        [[nodiscard]] window* containing_window() const
        {
            return window_;
        }
        maui::core::event<> loaded;
        maui::core::event<> unloaded;

        // Attach (non-null) or detach (null) this element + its logical subtree to a window. A non-null
        // transition fires loaded (top-down: this element first, then children); a null transition fires
        // unloaded (bottom-up: children first, then this element). Idempotent — same window is a no-op.
        // Mirrors VisualElement.Window propagation + SendLoaded/SendUnloaded.
        void set_containing_window(window* value);

        // ---- Resources (IResourcesProvider.Resources / VisualElement.Resources) ----
        // The element's resource dictionary, lazily created on first access (IsResourcesCreated flips true).
        // Changing a value re-applies any bound DynamicResource on this element + its subtree and re-resolves
        // the merged (implicit/class) style. NON-owning toward merged dictionaries (the caller owns those).
        [[nodiscard]] resource_dictionary& resources();
        [[nodiscard]] bool is_resources_created() const
        {
            return resources_ != nullptr;
        }

        // The logical parent (Element.Parent) — set when a container attaches this as a child. The resource
        // lookup and implicit-style match walk up this chain. NON-owning (a child never owns its parent).
        [[nodiscard]] element* logical_parent() const
        {
            return logical_parent_;
        }

        // Resolve `key` against this element's resources then each ancestor's (ResourcesExtensions
        // .TryGetResource). Returns the borrowed stored value or nullptr. The reference is valid until the
        // owning dictionary mutates.
        [[nodiscard]] const std::any* try_get_resource(std::string_view key) const;

        // Bind the property named `name` to the resource `key` (BindableObject.SetDynamicResource): resolve
        // it now (if found) at setter_specificity::dynamic_resource_setter, and re-apply whenever that
        // resource changes or this element reparents. Idempotent re-binding updates the key.
        void set_dynamic_resource(std::string name, std::string key);
        // Stop tracking the DynamicResource on `name` (BindableObject.RemoveDynamicResource). The already-set
        // value is kept (C# only stops future updates).
        void remove_dynamic_resource(std::string_view name);

    protected:
        element() = default;

        // Visit each direct logical child (default: none — a leaf). Containers override to expose theirs.
        // Every control is-a element, so propagation hands back element& and needs no cast at the call site.
        virtual void for_each_logical_child(const std::function<void(element&)>& visit) const
        {
            (void)visit;
        }

        // A container calls these when a child is added / removed so the child inherits (or loses) this
        // element's binding context + window + resources immediately (Element.OnChildAdded / OnChildRemoved).
        // detach is static — it only operates on `child` (clear its window + parent, then re-resolve its now
        // shorter resource chain); attach needs `this` (to flow this element's context/window/resources down).
        void attach_logical_child(element& child);
        static void detach_logical_child(element& child);

        // bindable_object::on_binding_context_changed override: raise the event (base) then propagate the
        // new context to every logical child as an inherited context.
        void on_binding_context_changed() override;

        // Declare this element's style target type (the implicit-style key + the type a style targets). A
        // control calls this in its constructor — `set_style_target_type<button>()` — so resources keyed by
        // type_tag::of<button>() match it. C# derives this from the runtime type; the reflection-free port
        // has the control declare it explicitly. Until set, the element has no implicit style.
        template <class TControl> void set_style_target_type()
        {
            style_target_type_ = maui::core::type_tag::of<TControl>();
            merged_style_.set_target_type(*style_target_type_);
        }

        // Apply the merged (implicit + class) style to this element (merged_style.apply). Called by the view
        // when an explicit style is set/cleared and by the resource/parent machinery on a relevant change.
        // The explicit (local) style is owned by view<>; merged_style only handles implicit + class styles.
        void refresh_merged_style();
        // Tell the merged_style which class names this control selects (VisualElement.StyleClass) and which
        // local style is set, so it can layer implicit < class < local correctly. Called by view<>.
        void set_merged_style_classes(std::vector<std::string> classes);

        // The element's resource chain changed (a resource value changed, or it reparented). view<> overrides
        // this to re-resolve a LOCAL style's base_resource_key against the new chain (the local style is owned
        // by view<>, so element can't touch it directly). Called AFTER the merged-style refresh. Default no-op.
        virtual void on_resource_chain_changed()
        {
        }

        merged_style merged_style_{*this}; // the implicit + class style resolver (MergedStyle)

    private:
        // Re-resolve every bound DynamicResource on THIS element against the current resource chain (used on
        // a reparent — the new ancestors may hold the key) and re-resolve the merged style, then recurse into
        // logical children (their chain changed too). Mirrors Element.OnParentResourcesChanged.
        void reapply_resources_from_chain();
        // THIS element's own resource dictionary changed: re-apply the affected DynamicResources + refresh
        // the merged style, then propagate the changed keys DOWN to logical children (Element
        // .OnResourcesChanged → each child). Down-propagation is why an element only subscribes to its OWN
        // dictionary — a change up the tree reaches a descendant by recursing through children.
        void on_resources_changed(const std::vector<resource_change>& values);
        // Re-apply the DynamicResources whose key is among `keys`, or ALL of them when `keys` is null,
        // resolving each against the current chain. Used by both paths above.
        void apply_dynamic_resources(const std::vector<resource_change>* keys);

        window* window_ = nullptr;          // non-owning back-ref to the hosting window (VisualElement.Window)
        element* logical_parent_ = nullptr; // non-owning back-ref to the logical parent (Element.Parent)
        std::unique_ptr<resource_dictionary> resources_; // lazily created (IsResourcesCreated)
        maui::core::scoped_connection resources_token_;  // own-dictionary values_changed subscription
        // DynamicResource bindings: property-name → resource-key (Element.DynamicResources). Both strings are
        // owned (a name or key may be a built string).
        std::unordered_map<std::string, std::string> dynamic_resources_;
        std::optional<maui::core::type_tag> style_target_type_; // this control's style TargetType (if any)
    };
} // namespace maui::controls
