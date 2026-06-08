#pragma once
// maui::controls::element  <=  Microsoft.Maui.Controls.Element / VisualElement (lifecycle subset)
//
// The non-template base between bindable_object (the value / binding-context layer) and view<> (the
// handler / geometry layer). It carries the cross-cutting *tree lifecycle* every control shares:
//   - logical-children visitation (the port's stand-in for Element.LogicalChildren) — a container
//     overrides for_each_logical_child to expose its children; leaves keep the no-op default;
//   - BindingContext INHERITANCE — on_binding_context_changed propagates this element's context down to
//     each logical child (the role of Element.OnBindingContextChanged → SetChildInheritedBindingContext);
//   - the Window back-reference + Loaded/Unloaded events (VisualElement.Window + Loaded/Unloaded) — when a
//     root is hosted in a window, set_containing_window flows the window down the subtree, firing Loaded as
//     it attaches and Unloaded as it detaches.
// attach_logical_child / detach_logical_child are the hooks a container calls when it gains / loses a child
// so the child immediately inherits (or loses) the parent's context + window (Element.OnChildAdded/Removed).
//
// Scope (M5c): the propagation machinery itself. The fuller Element surface (Effects, resources, the visual-
// vs-logical split, platform loaded-event wiring, modal stacks) is out of scope (STATUS.md).

#include <functional>

#include "maui/core/bindable_object.hpp"
#include "maui/core/event.hpp"

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

    protected:
        element() = default;

        // Visit each direct logical child (default: none — a leaf). Containers override to expose theirs.
        // Every control is-a element, so propagation hands back element& and needs no cast at the call site.
        virtual void for_each_logical_child(const std::function<void(element&)>& visit) const
        {
            (void)visit;
        }

        // A container calls these when a child is added / removed so the child inherits (or loses) this
        // element's binding context + window immediately (the role of Element.OnChildAdded / OnChildRemoved).
        // detach is static — removing a child only clears ITS window; its last inherited BindingContext is
        // kept (C# leaves it until the child is reparented or the value is reset), so the parent is unused.
        void attach_logical_child(element& child);
        static void detach_logical_child(element& child);

        // bindable_object::on_binding_context_changed override: raise the event (base) then propagate the
        // new context to every logical child as an inherited context.
        void on_binding_context_changed() override;

    private:
        window* window_ = nullptr; // non-owning back-ref to the hosting window (VisualElement.Window)
    };
} // namespace maui::controls
