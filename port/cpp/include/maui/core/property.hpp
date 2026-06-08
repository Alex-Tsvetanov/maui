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
#include <utility>

#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/setter_specificity_list.hpp"

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
            owner_->register_property(descriptor_->name(),
                                      bindable_object::property_handle{
                                          .apply =
                                              [this](const std::any& value, setter_specificity specificity) {
                                                  set(std::any_cast<const T&>(value), specificity);
                                              },
                                          .clear = [this](setter_specificity specificity) { clear(specificity); }});
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
