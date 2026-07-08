#pragma once
// maui::controls::property_trigger<T> / multi_trigger / data_trigger<…>
//   <=  Microsoft.Maui.Controls.Trigger / MultiTrigger / DataTrigger (+ PropertyCondition / BindingCondition)
//
// A trigger watches a condition and applies a bundle of setters while it holds, un-applying them when it
// stops — the code-first, reflection-free analog of XAML triggers. C#'s triggers read GetValue(Property)
// and observe PropertyChanged by name and reflect over a Binding path; the typed port watches a concrete
// property<T> (or the typed BindingContext) through `.changed` / binding_context_changed and compares with
// operator==, pushing setters at setter_specificity::trigger — one rung ABOVE a manual set, so an active
// trigger overrides a manually-set value and cleanly restores it on deactivation (the C# precedence).
//
//   - property_trigger<T> (Trigger): one property == value condition, with EnterActions / ExitActions and
//     Setter.TargetName support (via the setter's retarget).
//   - multi_trigger (MultiTrigger): several conditions (each a typed property == value); setters apply only
//     while ALL hold (the MultiCondition aggregate).
//   - data_trigger<Context, Value> (DataTrigger): the condition reads the target's typed BindingContext via
//     an accessor and compares to a value; re-evaluated when BindingContext changes.
//
// Each trigger is attached to a target (which owns the watched property / the binding context) and returns
// a trigger_handle that tears it down (unsubscribe + un-apply) when dropped — the M5b RAII contract.

#include <functional>
#include <memory>
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
    // A TriggerAction (TriggerAction<T>): a callback invoked with the target when the condition enters /
    // exits. The reflection-free analog of the abstract C# TriggerAction.Invoke(sender).
    using trigger_action = std::function<void(maui::core::bindable_object&)>;

    // RAII teardown for an attached trigger: dropping it unsubscribes from the watched property and
    // un-applies the trigger's setters (a safe no-op if the trigger is not currently active, since
    // clearing an unset specificity does nothing). Move-only — mirrors binding_handle.
    class trigger_handle
    {
    public:
        trigger_handle() = default;
        trigger_handle(std::vector<maui::core::scoped_connection> connections,
                       maui::core::move_only_function<void()> unapply)
            : connections_(std::move(connections)), unapply_(std::move(unapply))
        {
        }
        trigger_handle(maui::core::scoped_connection connection, maui::core::move_only_function<void()> unapply)
            : unapply_(std::move(unapply))
        {
            connections_.push_back(std::move(connection));
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
            connections_.clear();
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
        std::vector<maui::core::scoped_connection> connections_;
        maui::core::move_only_function<void()> unapply_;
    };

    // The shared apply/unapply core of every trigger (TriggerBase.OnConditionChanged): fire EnterActions +
    // apply the setters when the (aggregate) condition becomes true; un-apply + fire ExitActions when false.
    // Holds the setters + actions; subclasses/holders supply the condition wiring in attach().
    class trigger_body
    {
    public:
        void add_setter(setter value)
        {
            setters_.push_back(std::move(value));
        }
        void add_enter_action(trigger_action action)
        {
            enter_actions_.push_back(std::move(action));
        }
        void add_exit_action(trigger_action action)
        {
            exit_actions_.push_back(std::move(action));
        }

        // Drive the body to `active`: a no-op if unchanged. On true: EnterActions then setters (the C#
        // order); on false: setters un-applied then ExitActions.
        void set_active(maui::core::bindable_object& target, bool active)
        {
            if (active == active_)
            {
                return;
            }
            active_ = active;
            if (active)
            {
                for (const trigger_action& action : enter_actions_)
                {
                    action(target);
                }
                for (const setter& value : setters_)
                {
                    value.apply(target, maui::core::setter_specificity::trigger);
                }
            }
            else
            {
                for (const setter& value : setters_)
                {
                    value.unapply(target, maui::core::setter_specificity::trigger);
                }
                for (const trigger_action& action : exit_actions_)
                {
                    action(target);
                }
            }
        }

    private:
        std::vector<setter> setters_;
        std::vector<trigger_action> enter_actions_;
        std::vector<trigger_action> exit_actions_;
        bool active_ = false;
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
            body_.add_setter(std::move(value));
            return *this;
        }
        // Add an EnterAction (fired when the condition becomes true) / ExitAction (when it becomes false).
        property_trigger& add_enter_action(trigger_action action)
        {
            body_.add_enter_action(std::move(action));
            return *this;
        }
        property_trigger& add_exit_action(trigger_action action)
        {
            body_.add_exit_action(std::move(action));
            return *this;
        }

        // Attach to `target` (which must own the watched property): apply the setters now if the watched
        // value already matches, then keep them in sync as the watched property changes. The returned
        // handle tears the trigger down (unsubscribe + un-apply) when dropped. The trigger and target must
        // outlive the handle.
        [[nodiscard]] trigger_handle attach(maui::core::bindable_object& target)
        {
            body_.set_active(target, watched_->get() == value_);
            auto connection =
                maui::core::connect_scoped(watched_->changed, [this, &target](const T& /*old*/, const T& new_value) {
                    body_.set_active(target, new_value == value_);
                });
            return trigger_handle{std::move(connection), [this, &target] { body_.set_active(target, false); }};
        }

    private:
        maui::core::property<T>* watched_;
        T value_;
        trigger_body body_;
    };

    // A type-erased condition for multi_trigger: knows how to read its current truth on a target and to
    // subscribe to its source so a change re-evaluates the aggregate. Each concrete condition is a typed
    // property == value test (PropertyCondition); BindingCondition (a binding-context test) could be added
    // the same way if a multi-trigger needs it.
    class i_trigger_condition
    {
    public:
        virtual ~i_trigger_condition() = default;
        [[nodiscard]] virtual bool is_satisfied() const = 0;
        // Subscribe so `on_changed` runs whenever this condition's truth might change.
        [[nodiscard]] virtual maui::core::scoped_connection observe(std::function<void()> on_changed) = 0;

    protected:
        i_trigger_condition() = default;
        i_trigger_condition(const i_trigger_condition&) = default;
        i_trigger_condition(i_trigger_condition&&) = default;
        i_trigger_condition& operator=(const i_trigger_condition&) = default;
        i_trigger_condition& operator=(i_trigger_condition&&) = default;
    };

    // A property == value condition over a typed property<T> (PropertyCondition).
    template <class T> class property_condition : public i_trigger_condition
    {
    public:
        property_condition(maui::core::property<T>& watched, T value) : watched_(&watched), value_(std::move(value))
        {
        }
        [[nodiscard]] bool is_satisfied() const override
        {
            return watched_->get() == value_;
        }
        [[nodiscard]] maui::core::scoped_connection observe(std::function<void()> on_changed) override
        {
            return maui::core::connect_scoped(
                watched_->changed, [on_changed = std::move(on_changed)](const T&, const T&) { on_changed(); });
        }

    private:
        maui::core::property<T>* watched_;
        T value_;
    };

    // MultiTrigger: applies its setters only while EVERY condition is satisfied (the MultiCondition
    // aggregate). Conditions are added typed; the trigger erases them behind i_trigger_condition.
    class multi_trigger
    {
    public:
        // Add a (typed) property == value condition (fluent).
        template <class T> multi_trigger& add_condition(maui::core::property<T>& watched, T value)
        {
            conditions_.push_back(std::make_unique<property_condition<T>>(watched, std::move(value)));
            return *this;
        }
        multi_trigger& add(setter value)
        {
            body_.add_setter(std::move(value));
            return *this;
        }
        multi_trigger& add_enter_action(trigger_action action)
        {
            body_.add_enter_action(std::move(action));
            return *this;
        }
        multi_trigger& add_exit_action(trigger_action action)
        {
            body_.add_exit_action(std::move(action));
            return *this;
        }

        // Attach to `target`: apply the setters now if all conditions hold, then re-evaluate the aggregate on
        // every condition change. The handle tears it down when dropped.
        [[nodiscard]] trigger_handle attach(maui::core::bindable_object& target)
        {
            body_.set_active(target, all_satisfied());
            std::vector<maui::core::scoped_connection> connections;
            connections.reserve(conditions_.size());
            for (const std::unique_ptr<i_trigger_condition>& condition : conditions_)
            {
                connections.push_back(
                    condition->observe([this, &target] { body_.set_active(target, all_satisfied()); }));
            }
            return trigger_handle{std::move(connections), [this, &target] { body_.set_active(target, false); }};
        }

    private:
        [[nodiscard]] bool all_satisfied() const
        {
            for (const std::unique_ptr<i_trigger_condition>& condition : conditions_)
            {
                if (!condition->is_satisfied())
                {
                    return false;
                }
            }
            return true; // vacuously true with no conditions (matches MultiCondition's newState = true)
        }

        std::vector<std::unique_ptr<i_trigger_condition>> conditions_;
        trigger_body body_;
    };

    // DataTrigger: the condition reads the target's typed BindingContext (as Context) via `accessor` and
    // compares the result to `value`. Re-evaluated when the target's binding context changes (the typed
    // analog of C#'s BindingCondition over a Binding path). A null/mismatched-type context never matches.
    template <class Context, class Value> class data_trigger
    {
    public:
        data_trigger(std::function<Value(const Context&)> accessor, Value value)
            : accessor_(std::move(accessor)), value_(std::move(value))
        {
        }

        data_trigger& add(setter value)
        {
            body_.add_setter(std::move(value));
            return *this;
        }
        data_trigger& add_enter_action(trigger_action action)
        {
            body_.add_enter_action(std::move(action));
            return *this;
        }
        data_trigger& add_exit_action(trigger_action action)
        {
            body_.add_exit_action(std::move(action));
            return *this;
        }

        // Attach to `target`: evaluate against the current BindingContext now, then re-evaluate whenever the
        // context changes (binding_context_changed). The handle tears it down when dropped.
        [[nodiscard]] trigger_handle attach(maui::core::bindable_object& target)
        {
            body_.set_active(target, matches(target));
            auto connection = maui::core::connect_scoped(
                target.binding_context_changed, [this, &target] { body_.set_active(target, matches(target)); });
            return trigger_handle{std::move(connection), [this, &target] { body_.set_active(target, false); }};
        }

    private:
        [[nodiscard]] bool matches(maui::core::bindable_object& target) const
        {
            const std::shared_ptr<Context> context = target.binding_context<Context>();
            return context != nullptr && accessor_(*context) == value_;
        }

        std::function<Value(const Context&)> accessor_;
        Value value_;
        trigger_body body_;
    };

    // VisualElement.Triggers: a collection created pre-attached to its owning element (C#'s
    // TriggersPropertyKey defaultValueCreator runs collection.AttachTo(bindable)). Adding a trigger attaches
    // it to the owner immediately (AttachedCollectionChanged); removing it / destroying the collection drops
    // the trigger's RAII handle (unsubscribe + un-apply). Because the trigger types are unrelated (no shared
    // base — property_trigger<T> / multi_trigger / data_trigger<C,V> only share the shape attach(target) ->
    // trigger_handle), each entry TYPE-ERASES its trigger behind a shared_ptr<void> and owns the handle its
    // attach() returned. Non-copyable/non-movable, like behavior_collection — the handles hold back-references
    // to the owner and to the heap-stable trigger objects.
    class triggers_collection
    {
    public:
        triggers_collection() = default;
        explicit triggers_collection(maui::core::bindable_object& owner) : owner_(&owner)
        {
        }
        triggers_collection(const triggers_collection&) = delete;
        triggers_collection(triggers_collection&&) = delete;
        triggers_collection& operator=(const triggers_collection&) = delete;
        triggers_collection& operator=(triggers_collection&&) = delete;
        ~triggers_collection() = default;

        // Add a trigger — any type with `[[nodiscard]] trigger_handle attach(bindable_object&)` (the four
        // trigger types + event_trigger). It attaches to the owner now; the collection owns it (and its
        // handle) so it reverts when removed or when the collection dies.
        template <class Trigger> void add(Trigger trigger)
        {
            auto owned = std::make_shared<Trigger>(std::move(trigger));
            entry item;
            if (owner_ != nullptr)
            {
                item.handle = owned->attach(*owner_); // apply now / observe (AttachedCollectionChanged)
            }
            item.owned = std::static_pointer_cast<void>(std::move(owned));
            entries_.push_back(std::move(item));
        }

        // Remove every trigger, dropping each handle first (un-apply) then the trigger (ClearItems).
        void clear()
        {
            entries_.clear();
        }
        [[nodiscard]] std::size_t count() const
        {
            return entries_.size();
        }

    private:
        // Destruction order is reverse-declaration: `handle` dies FIRST (unsubscribe + un-apply while the
        // trigger is still alive), then `owned` frees the trigger. A moved entry (vector realloc) keeps both
        // valid — the heap trigger and the owner do not move.
        struct entry
        {
            std::shared_ptr<void> owned;
            trigger_handle handle;
        };
        maui::core::bindable_object* owner_ = nullptr;
        std::vector<entry> entries_;
    };
} // namespace maui::controls
