#pragma once
// maui::controls::property_trigger<T>  <=  Microsoft.Maui.Controls.Trigger (+ PropertyCondition)
//
// A trigger watches one typed property and applies a bundle of setters while that property equals a
// target value, un-applying them when it stops matching — the code-first, reflection-free analog of a
// XAML <Trigger Property="…" Value="…">. C#'s Trigger reads bindable.GetValue(Property) and observes
// PropertyChanged by name; the typed port instead watches a concrete property<T> through its `.changed`
// event and compares with operator==, pushing the setters at setter_specificity::trigger. That sits one
// rung ABOVE a manual set (ManualTriggerBaseline), so an active trigger overrides a manually-set value
// and cleanly restores it when the trigger deactivates — exactly the C# precedence.
//
// Setters apply to the `target` passed to attach(); in the common case the watched property is one of
// the target's own properties. Setter.TargetName (re-targeting a different element), EnterActions/
// ExitActions, MultiTrigger, per-trigger index ordering, and a BindingContext-sourced DataTrigger are
// deferred (STATUS.md) — DataTrigger in particular lands with BindingContext in M5c.

#include <utility>
#include <vector>

#include "maui/controls/setter.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/event.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property.hpp"
#include "maui/core/setter_specificity.hpp"

namespace maui::controls
{
    // RAII teardown for an attached trigger: dropping it unsubscribes from the watched property and
    // un-applies the trigger's setters (a safe no-op if the trigger is not currently active, since
    // clearing an unset specificity does nothing). Move-only — mirrors binding_handle.
    class trigger_handle
    {
    public:
        trigger_handle() = default;
        trigger_handle(maui::core::scoped_connection connection, maui::core::move_only_function<void()> unapply)
            : connection_(std::move(connection)), unapply_(std::move(unapply))
        {
        }
        trigger_handle(const trigger_handle&) = delete;
        trigger_handle& operator=(const trigger_handle&) = delete;
        trigger_handle(trigger_handle&&) noexcept = default;
        trigger_handle& operator=(trigger_handle&&) noexcept = default;
        ~trigger_handle()
        {
            reset();
        }

        // Tear the trigger down now (idempotent): unsubscribe + un-apply the setters.
        void reset()
        {
            connection_.reset();
            if (unapply_)
            {
                auto unapply = std::move(unapply_);
                unapply_ = nullptr;
                unapply();
            }
        }
        [[nodiscard]] bool active() const
        {
            return static_cast<bool>(unapply_);
        }

    private:
        maui::core::scoped_connection connection_;
        maui::core::move_only_function<void()> unapply_;
    };

    template <class T> class property_trigger
    {
    public:
        property_trigger(maui::core::property<T>& watched, T value) : watched_(&watched), value_(std::move(value))
        {
        }

        // Add a setter applied while the trigger is active (fluent).
        property_trigger& add(setter value)
        {
            setters_.push_back(std::move(value));
            return *this;
        }

        // Attach to `target` (which must own the watched property): apply the setters now if the watched
        // value already matches, then keep them in sync as the watched property changes. The returned
        // handle tears the trigger down (unsubscribe + un-apply) when dropped. The trigger and target must
        // outlive the handle.
        [[nodiscard]] trigger_handle attach(maui::core::bindable_object& target)
        {
            set_active(target, watched_->get() == value_);
            auto connection =
                maui::core::connect_scoped(watched_->changed, [this, &target](const T& /*old*/, const T& new_value) {
                    set_active(target, new_value == value_);
                });
            return trigger_handle{std::move(connection), [this, &target] { set_active(target, false); }};
        }

    private:
        void set_active(maui::core::bindable_object& target, bool active)
        {
            if (active == active_)
            {
                return;
            }
            active_ = active;
            for (const auto& value : setters_)
            {
                if (active)
                {
                    value.apply(target, maui::core::setter_specificity::trigger);
                }
                else
                {
                    value.unapply(target, maui::core::setter_specificity::trigger);
                }
            }
        }

        maui::core::property<T>* watched_;
        T value_;
        std::vector<setter> setters_;
        bool active_ = false;
    };
} // namespace maui::controls
