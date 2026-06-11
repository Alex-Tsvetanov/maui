#pragma once
// maui::controls::behavior            <=  Microsoft.Maui.Controls.Behavior
// maui::controls::typed_behavior<T>   <=  Microsoft.Maui.Controls.Behavior<T>
// maui::controls::behavior_collection <=  Microsoft.Maui.Controls.AttachedCollection<Behavior> (internal)
//
// User-defined behaviors attach reusable logic to a bindable object: on_attached_to runs when the
// behavior is attached (subscribe, mutate, …) and on_detaching_from when it is removed. A view exposes
// a behavior_collection through behaviors() (VisualElement.Behaviors) — adding a behavior to the
// collection attaches it to the owning element at once, removing (or clearing) detaches it; the
// collection itself can also be attached to/detached from elements (the AttachedCollection face a Style
// uses in C#; styles carrying behaviors stay deferred — STATUS.md).
//
// The type guard: C# Behavior carries AssociatedType and AttachTo throws InvalidOperationException when
// the bindable is not an instance of it. The reflection-free port makes the guard a virtual predicate —
// base `behavior` accepts anything (AssociatedType = BindableObject); typed_behavior<T> checks via
// dynamic_cast — and attach_to REFUSES (returns false, nothing attached) instead of throwing, the port's
// no-exceptions seam convention.
//
// typed_behavior<T> keeps C#'s same-name overload pair (on_attached_to(bindable_object&) dispatching to
// on_attached_to(T&)); a subclass overriding only ONE of the pair should re-expose the other with a
// using-declaration (C++ name hiding — see the unit tests for the pattern).
//
// Statefulness note (behaviors.md): one behavior CAN be attached to several elements (C# allows it; a
// style shares its behaviors across every styled control), but a behavior holding per-element state
// should not be shared — the port preserves the C# semantics and adds no sharing guard.

#include <memory>
#include <vector>

#include "maui/core/bindable_object.hpp"

namespace maui::controls
{
    class behavior
    {
    public:
        behavior(const behavior&) = delete;
        behavior(behavior&&) = delete;
        behavior& operator=(const behavior&) = delete;
        behavior& operator=(behavior&&) = delete;
        virtual ~behavior() = default;

        // IAttachedObject.AttachTo: run the type guard, then on_attached_to. Returns false — attaching
        // NOTHING — when the bindable is not an instance of the behavior's associated type (C# throws
        // InvalidOperationException there).
        bool attach_to(maui::core::bindable_object& bindable)
        {
            if (!can_attach_to(bindable))
            {
                return false;
            }
            on_attached_to(bindable);
            return true;
        }
        // IAttachedObject.DetachFrom → OnDetachingFrom.
        void detach_from(maui::core::bindable_object& bindable)
        {
            on_detaching_from(bindable);
        }

    protected:
        behavior() = default;

        // The AssociatedType guard (Behavior.AssociatedType.IsInstanceOfType), as a reflection-free
        // predicate. The base associates with bindable_object — anything attaches.
        [[nodiscard]] virtual bool can_attach_to(const maui::core::bindable_object& bindable) const
        {
            (void)bindable;
            return true;
        }
        // Behavior.OnAttachedTo / OnDetachingFrom — the override points.
        virtual void on_attached_to(maui::core::bindable_object& bindable)
        {
            (void)bindable;
        }
        virtual void on_detaching_from(maui::core::bindable_object& bindable)
        {
            (void)bindable;
        }
    };

    // Behavior<T>: the typed specialization application developers derive from. The erased hooks
    // dispatch to the typed pair in the C# order (base first on attach, typed first on detach).
    template <class T> class typed_behavior : public behavior
    {
    protected:
        typed_behavior() = default;

        [[nodiscard]] bool can_attach_to(const maui::core::bindable_object& bindable) const final
        {
            return dynamic_cast<const T*>(&bindable) != nullptr;
        }
        void on_attached_to(maui::core::bindable_object& bindable) override
        {
            // Behavior<T>.OnAttachedTo(BindableObject) → OnAttachedTo((T)bindable). The guard already
            // proved the type, but a subclass may bypass attach_to — stay null-safe.
            if (auto* typed = dynamic_cast<T*>(&bindable))
            {
                on_attached_to(*typed);
            }
        }
        void on_detaching_from(maui::core::bindable_object& bindable) override
        {
            if (auto* typed = dynamic_cast<T*>(&bindable))
            {
                on_detaching_from(*typed);
            }
        }
        // The typed override points (Behavior<T>.OnAttachedTo(T) / OnDetachingFrom(T)).
        virtual void on_attached_to(T& bindable)
        {
            (void)bindable;
        }
        virtual void on_detaching_from(T& bindable)
        {
            (void)bindable;
        }
    };

    // AttachedCollection<Behavior>: the behavior list of one or more associated bindables. Owns the
    // behaviors (shared_ptr — the C# collection holds the only strong reference too); the associated
    // bindables are borrowed (the owning element outlives its own collection; a style's targets outlive
    // the application of the style).
    class behavior_collection
    {
    public:
        behavior_collection() = default;
        // The VisualElement.Behaviors shape: a collection created pre-attached to its owning element
        // (the BehaviorsPropertyKey defaultValueCreator runs collection.AttachTo(bindable)).
        explicit behavior_collection(maui::core::bindable_object& owner)
        {
            attach_to(owner);
        }
        // Non-copyable/non-movable: the associated-object back-references must not be duplicated.
        behavior_collection(const behavior_collection&) = delete;
        behavior_collection(behavior_collection&&) = delete;
        behavior_collection& operator=(const behavior_collection&) = delete;
        behavior_collection& operator=(behavior_collection&&) = delete;
        ~behavior_collection() = default;

        // IAttachedObject.AttachTo on the collection: remember the bindable and attach every behavior
        // already in the collection to it (AttachedCollection.OnAttachedTo).
        void attach_to(maui::core::bindable_object& bindable);
        // The reverse (OnDetachingFrom): detach every behavior from the bindable, then forget it.
        void detach_from(maui::core::bindable_object& bindable);

        // Add a behavior: it attaches to every associated bindable at once (InsertItem). A null pointer
        // is ignored.
        void add(std::shared_ptr<behavior> item);
        // Remove a behavior: it detaches from every associated bindable first (RemoveItem). Returns
        // true iff it was present.
        bool remove(const std::shared_ptr<behavior>& item);
        // Remove every behavior, detaching each from every associated bindable (ClearItems).
        void clear();

        [[nodiscard]] const std::vector<std::shared_ptr<behavior>>& items() const
        {
            return items_;
        }
        [[nodiscard]] std::size_t count() const
        {
            return items_.size();
        }

    private:
        std::vector<std::shared_ptr<behavior>> items_;
        std::vector<maui::core::bindable_object*> associated_; // the attached bindables (NON-owning)
    };
} // namespace maui::controls
