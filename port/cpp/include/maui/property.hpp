#pragma once
// maui::property<T> / maui::ro_property<T, Getter> — the view-model-facing bindable members
// (PUBLIC_API_DESIGN.md §6, the LoginViewModel shape).
//
// These are deliberately LIGHTER than maui::core::property<T>: a core::property carries the full MAUI
// value-precedence machinery (handler vs binding vs manual specificity) because it backs a *control*
// whose setters have native side effects. A view-model member needs none of that — only "hold a value,
// notify on change" — so maui::property<T> is a plain notifying cell with operator() get/set ergonomics:
//
//     class login_view_model {
//     public:
//         maui::property<std::string> Username;                       // vm.Username()      -> get
//         maui::property<bool>        IsBusy{false};                  // vm.IsBusy(true)    -> set
//         maui::ro_property<bool>     IsNotBusy{[this]{                // computed, read-only
//             return !IsBusy(); }};
//     };
//
// ro_property holds a CAPTURING getter, written as a default member initializer. That placement matters:
// a default member initializer is a "complete-class context" ([class.mem]), so the lambda body may name
// any member of the enclosing view-model (even ones declared later) and capture `this`. The originally
// proposed NTTP form `ro_property<bool, +[](VM* self){ self->X() }>` does NOT compile — a lambda in a
// non-type template argument is parsed where the class is still incomplete, so `self->X()` is a
// "member access into incomplete type" error; the capturing-getter form is the working idiom and reads
// more directly (vm.IsNotBusy() with no owner argument). If the getter names a member that does not
// exist, that is a COMPILE ERROR here — exactly the guarantee the design asked for. (Resolving the markup
// PATH "IsNotBusy" to this member BY NAME is the separate reflection-gated step — see maui/xaml/feature.hpp.)

#include <functional>
#include <type_traits>
#include <utility>

#include "maui/core/event.hpp"

namespace maui
{
    // A notifying value cell. Non-copyable/non-movable: it owns a change event whose subscribers hold a
    // back-reference, so (like maui::core::observable) it must live at a stable address as a VM member.
    template <class T> class property
    {
    public:
        using value_type = T;

        property() = default;
        explicit property(T initial) : value_(std::move(initial))
        {
        }
        property(const property&) = delete;
        property(property&&) = delete;
        property& operator=(const property&) = delete;
        property& operator=(property&&) = delete;
        ~property() = default;

        // Get: `vm.Username()`. Set: `vm.Username("alex")`. Also explicit get()/set() for clarity.
        [[nodiscard]] const T& operator()() const
        {
            return value_;
        }
        void operator()(T value)
        {
            set(std::move(value));
        }
        [[nodiscard]] const T& get() const
        {
            return value_;
        }
        void set(T value)
        {
            if (value_ == value)
            {
                return; // no-change: do not fire (mirrors bindable_property change semantics)
            }
            T old = std::move(value_);
            value_ = std::move(value);
            changed.raise(old, value_);
        }

        // Fired (old, new) on a real change — the one-way data-flow seam for the binding engine and for
        // ad-hoc subscribers (connect returns a maui::core RAII token; hold it to stay subscribed).
        maui::core::event<T, T> changed;

    private:
        T value_{};
    };

    // A read-only COMPUTED property: no stored value, just a getter (typically `[this]{ ... }`) evaluated
    // on demand. Non-copyable/non-movable — the getter captures the owning view-model's `this`, so the
    // property must live at a stable address (the view-model is non-movable regardless).
    template <class T> class ro_property
    {
    public:
        using value_type = T;

        // Implicit so the default-member-initializer reads naturally: `ro_property<bool> HasError{[this]{...}}`.
        template <class Getter>
            requires std::is_invocable_r_v<T, const Getter&>
        ro_property(Getter getter) // NOLINT(google-explicit-constructor) — VM ergonomics
            : getter_(std::move(getter))
        {
        }
        ro_property(const ro_property&) = delete;
        ro_property(ro_property&&) = delete;
        ro_property& operator=(const ro_property&) = delete;
        ro_property& operator=(ro_property&&) = delete;
        ~ro_property() = default;

        // Evaluate: `vm.HasError()`.
        [[nodiscard]] T operator()() const
        {
            return getter_();
        }
        [[nodiscard]] T get() const
        {
            return getter_();
        }

        // A computed value has no intrinsic change signal (its inputs are other properties). The
        // view-model raises this when a dependency changes — e.g. in its ErrorMessage.changed handler —
        // so a one-way binding to this property can re-pull. (Wiring those dependencies automatically is
        // the reflection-gated step.)
        maui::core::event<> changed;

    private:
        std::function<T()> getter_;
    };
} // namespace maui
