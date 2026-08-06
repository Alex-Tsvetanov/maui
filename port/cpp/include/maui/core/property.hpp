#pragma once
// maui::core::property<T> — the member-object form of a bindable property (PROFILE.md §11).
//
// Each property<T> is a strongly-typed value slot living inside its owning bindable_object (e.g.
// `property<std::string> text;` on a button), referencing the shared bindable_property<T> descriptor
// for its name/default/callbacks. It owns the per-instance value store (setter_specificity_list<T>)
// and implements value precedence + change notification — the role C#'s BindableObject.SetValueActual
// played, but fully typed: no std::any, no boxing, no RTTI; get() returns const T& with no copy.
//
// Storage is lazy: an unset property holds the shared descriptor default (no per-instance allocation)
// until a value is set (or a default-value creator materializes one on first read). Ordering matches
// MAUI: handler values are overridden by any other set, a value below the current specificity is kept
// silently for later un-apply, and notification fires only on a real change as
// changing-callback -> changing event -> store -> changed event -> changed-callback -> .changed.
//
// Usage: declare a `static const bindable_property<T> xxx_property{...}`, then a member
// `property<T> xxx{*this, xxx_property};`.

#include <any>
#include <memory>
#include <type_traits>
#include <utility>

#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/boxed_value.hpp"
#include "maui/core/event.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/setter_specificity_list.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::core
{
    template <class T> class property
    {
    public:
        property(bindable_object& owner, const bindable_property<T>& descriptor)
            : owner_(&owner), descriptor_(&descriptor)
        {
            // Self-register a typed-erased handle so a style/trigger/VSM setter can reach this typed slot by
            // the descriptor name (bindable_object::apply_setter / clear_setter). `this` is stable (property
            // is non-copyable/non-movable) and is unregistered in the destructor.
            owner_->register_property(
                descriptor_->name(),
                bindable_object::property_handle{
                    .apply =
                        [this](const std::any& value, setter_specificity specificity) {
                            set(std::any_cast<const T&>(value), specificity);
                        },
                    .clear = [this](setter_specificity specificity) { clear(specificity); },
                    // --- runtime bindings (W1-02): the name->getter / converting-setter channel ---
                    .get = [this]() -> std::any { return box_value<T>(get()); },
                    .get_object = [this]() -> std::shared_ptr<bindable_object> { return object_form(get()); },
                    .try_apply =
                        [this](const std::any& value, setter_specificity specificity) {
                            if (auto unboxed = try_unbox<T>(value))
                            {
                                set(std::move(*unboxed), specificity);
                                return true;
                            }
                            return false;
                        },
                    .get_default = [this]() -> std::any { return box_value<T>(default_for_owner()); },
                    .is_set = [this]() -> bool { return is_set(); },
                    .demote_to_binding = [this] { demote_to_binding_specificity(); },
                    .type = type_tag::of<T>(),
                    .default_binding_mode = descriptor.default_binding_mode(),
                    .is_read_only = descriptor.is_read_only()});
        }
        property(const property&) = delete;
        property(property&&) = delete;
        property& operator=(const property&) = delete;
        property& operator=(property&&) = delete;
        ~property()
        {
            owner_->unregister_property(descriptor_->name());
        }

        // Effective (highest-specificity) value; falls back to the shared descriptor default. The
        // reference is valid until the next set()/clear().
        [[nodiscard]] const T& get() const
        {
            ensure_default_materialized();
            return values_.empty() ? descriptor_->default_value() : values_.value_ref();
        }

        // Whether a value has been explicitly set at any specificity (the analog of C#'s
        // BindableObject.IsSet(BindableProperty)): true once set() has stored a value, false while the
        // property still reads the shared descriptor default. A lazily-materialized default-value-creator
        // value is NOT treated as "set" (mirroring C#), so this does not materialize the default.
        [[nodiscard]] bool is_set() const
        {
            return !values_.empty();
        }

        // The descriptor's default binding mode + read-only flag — read by bind() to resolve
        // binding_mode::default_mode and to downgrade two_way on a read-only target (mirrors C#).
        [[nodiscard]] binding_mode default_binding_mode() const
        {
            return descriptor_->default_binding_mode();
        }
        [[nodiscard]] bool is_read_only() const
        {
            return descriptor_->is_read_only();
        }

        void set(T value)
        {
            set(std::move(value), setter_specificity::manual_value_setter);
        }
        void set(T value, setter_specificity specificity)
        {
            auto const pin = pin_owner();
            const auto& callbacks = descriptor_->callbacks();
            if (callbacks.validate_value && !callbacks.validate_value(*owner_, value))
            {
                return; // invalid value, ignored (C# logs a warning)
            }
            if (callbacks.coerce_value)
            {
                value = callbacks.coerce_value(*owner_, value);
            }
            ensure_default_materialized();

            T const original = values_.empty() ? descriptor_->default_value() : values_.value_ref();
            setter_specificity original_specificity =
                values_.empty() ? setter_specificity::default_value : values_.specificity();

            // A non-handler set overrides a value that came from the handler.
            if (specificity != setter_specificity::from_handler &&
                original_specificity == setter_specificity::from_handler)
            {
                values_.remove(setter_specificity::from_handler);
                original_specificity = values_.empty() ? setter_specificity::default_value : values_.specificity();
            }

            // A value below the current specificity is kept silently so it can be restored later.
            if (specificity < original_specificity)
            {
                values_.set(specificity, std::move(value));
                return;
            }

            bool const same_value = value == original;
            if (!same_value)
            {
                if (callbacks.property_changing)
                {
                    callbacks.property_changing(*owner_, original, value);
                }
                owner_->on_property_changing(descriptor_->name());
            }

            values_.set(specificity, value);

            if (!same_value)
            {
                owner_->on_property_changed(descriptor_->name());
                if (callbacks.property_changed)
                {
                    callbacks.property_changed(*owner_, original, value);
                }
                changed.raise(original, value);
            }
        }

        void clear()
        {
            clear(setter_specificity::manual_value_setter);
        }
        void clear(setter_specificity specificity)
        {
            auto const pin = pin_owner();
            const auto& callbacks = descriptor_->callbacks();
            T const original = values_.empty() ? descriptor_->default_value() : values_.value_ref();
            if (!values_.empty() && values_.specificity() == setter_specificity::from_handler)
            {
                values_.remove(setter_specificity::from_handler);
            }
            T const new_value = cleared_effective(specificity);
            bool const did_change = !(original == new_value);
            if (did_change)
            {
                if (callbacks.property_changing)
                {
                    callbacks.property_changing(*owner_, original, new_value);
                }
                owner_->on_property_changing(descriptor_->name());
            }
            values_.remove(specificity);
            if (callbacks.coerce_value)
            {
                callbacks.coerce_value(*owner_, new_value); // for side effects (C# discards the result)
            }
            if (did_change)
            {
                owner_->on_property_changed(descriptor_->name());
                if (callbacks.property_changed)
                {
                    callbacks.property_changed(*owner_, original, new_value);
                }
                changed.raise(original, new_value);
            }
        }

        // Fired (old, new) when the effective value changes (delivered by const reference).
        event<T, T> changed;

    private:
        // Keep the owner alive for the whole of set()/clear(). Both run USER CODE mid-body — the
        // coerce/validate/changing/changed callbacks, on_property_changing/on_property_changed (which
        // fan out through every override down to bindable_object's raise), and `changed` — and every
        // frame on the way back out still touches the object: property.hpp's own tail, but also
        // element's effects fan-out (element_effects.cpp:117-118) and view's handler/z-order push. A
        // handler that drops the last shared_ptr to the owner would otherwise run operator delete
        // underneath all of those frames.
        //
        // This is C#'s guarantee restated, not a new one: BindableObject.cs:637-644 raises and THEN
        // touches `this`, and Element.cs:696-706 documents that the trailing work is deliberately
        // sequenced after both raises ("It can cause somewhat confusing behavior if the handler update
        // happens between these two calls"). So the order is load-bearing and must not be flipped; the
        // CLR simply roots `this` for the method body and the port has to say so out loud. Nothing here
        // asks "am I still alive" — the object cannot die in the first place, so no frame needs a check.
        //
        // Empty (a harmless no-op) when the owner is not itself shared_ptr-owned. A pure stack local is
        // then safe anyway — a handler has no way to end its scope. A SUBOBJECT of a shared_ptr-owned
        // parent is the one residual gap: releasing the parent frees the member too, and there is no
        // refcount on the member to pin. Closing that needs the pin to name the OWNING object rather
        // than `this`, which property<T> cannot see; it is a separate change, not something a wider
        // pin here would catch.
        [[nodiscard]] std::shared_ptr<bindable_object> pin_owner() const
        {
            return owner_->weak_from_this().lock();
        }

        // --- runtime bindings (W1-02) helpers ---
        // The value as a walkable bindable_object node: only a shared_ptr<U> with U deriving
        // bindable_object has one (the engine's chain hop); every other T answers null.
        [[nodiscard]] static std::shared_ptr<bindable_object> object_form(const T& value)
        {
            if constexpr (detail::is_shared_ptr<T>::value)
            {
                using element_t = typename T::element_type;
                if constexpr (std::is_base_of_v<bindable_object, element_t>)
                {
                    return value;
                }
            }
            (void)value;
            return nullptr;
        }
        // The default a failed binding resolution falls back to (C# BindableProperty.GetDefaultValue:
        // the default-value creator wins over the static default when present).
        [[nodiscard]] T default_for_owner() const
        {
            if (descriptor_->has_default_value_creator())
            {
                return descriptor_->create_default(*owner_);
            }
            return descriptor_->default_value();
        }
        // BindableObject.SetBinding's silent demote: move the top value down to from_binding (no
        // change events fire — the effective value is unchanged) so the binding's first apply at
        // from_binding (or above) replaces it instead of being outranked by an older manual set.
        void demote_to_binding_specificity()
        {
            if (values_.empty())
            {
                return;
            }
            const setter_specificity top = values_.specificity();
            if (!(setter_specificity::from_binding < top))
            {
                return;
            }
            T value = values_.value_ref();
            values_.remove(top);
            values_.set(setter_specificity::from_binding, std::move(value));
        }
        // --- end runtime bindings (W1-02) helpers ---

        void ensure_default_materialized() const
        {
            if (values_.empty() && descriptor_->has_default_value_creator())
            {
                values_.set(setter_specificity::default_value, descriptor_->create_default(*owner_));
            }
        }
        // The value that would be effective if `specificity` were removed.
        [[nodiscard]] T cleared_effective(setter_specificity specificity) const
        {
            if (values_.empty())
            {
                return descriptor_->default_value();
            }
            if (values_.specificity() == specificity)
            {
                return values_.count() >= 2 ? values_.cleared_value() : descriptor_->default_value();
            }
            return values_.value_ref();
        }

        bindable_object* owner_;
        const bindable_property<T>* descriptor_;
        mutable setter_specificity_list<T> values_; // mutable: a creator materializes lazily from get()
    };
} // namespace maui::core
