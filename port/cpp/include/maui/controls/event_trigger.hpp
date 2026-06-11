#pragma once
// maui::controls::event_trigger             <=  Microsoft.Maui.Controls.EventTrigger
// maui::controls::trigger_action_base       <=  Microsoft.Maui.Controls.TriggerAction
// maui::controls::typed_trigger_action<T>   <=  Microsoft.Maui.Controls.TriggerAction<T>
//
// An event_trigger fires its actions whenever a NAMED event on the attached element raises. C# resolves
// the name with reflection (GetRuntimeEvent + CreateDelegate); the port's events are typed members, so
// the reflection-free seam is a NAMED-EVENT REGISTRAR on the element — the event analog of how property
// names route through bindable_object::apply_setter: a control registers each public event channel by
// name in its constructor (element::register_named_event("clicked", …)), and
// element::connect_named_event(name, handler) subscribes through it. An unknown (or empty) event name
// attaches nothing — C# logs a warning there; the port returns an empty connection.
//
// Sealing: C# seals a trigger on first attach and Event's setter then throws InvalidOperationException;
// the no-exceptions port refuses instead (set_event_name returns false and leaves the name).
//
// Actions come in two forms: the existing function-form `trigger_action` (the port's TriggerAction
// analog from trigger.hpp), and the class-form trigger_action_base / typed_trigger_action<T> for stateful
// actions (C# TriggerAction/TriggerAction<T>; the typed dispatch casts — the port no-ops on a type
// mismatch instead of throwing InvalidCastException). attach() returns the same RAII trigger_handle the
// other triggers use (the M5b contract): dropping it unsubscribes. The trigger must outlive the handle.

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/element.hpp"
#include "maui/controls/trigger.hpp"
#include "maui/core/bindable_object.hpp"

namespace maui::controls
{
    class trigger_action_base
    {
    public:
        trigger_action_base(const trigger_action_base&) = delete;
        trigger_action_base(trigger_action_base&&) = delete;
        trigger_action_base& operator=(const trigger_action_base&) = delete;
        trigger_action_base& operator=(trigger_action_base&&) = delete;
        virtual ~trigger_action_base() = default;

        // TriggerAction.DoInvoke — the public face the trigger calls with the sender.
        void invoke(maui::core::bindable_object& sender)
        {
            do_invoke(sender);
        }

    protected:
        trigger_action_base() = default;
        // TriggerAction.Invoke(object sender) — the override point.
        virtual void do_invoke(maui::core::bindable_object& sender) = 0;
    };

    // TriggerAction<T>: the typed action base. The erased dispatch casts the sender; a mismatch no-ops
    // (C# would throw InvalidCastException — there is none to throw at this seam).
    template <class T> class typed_trigger_action : public trigger_action_base
    {
    protected:
        typed_trigger_action() = default;

        void do_invoke(maui::core::bindable_object& sender) final
        {
            if (auto* typed = dynamic_cast<T*>(&sender))
            {
                invoke(*typed);
            }
        }
        // TriggerAction<T>.Invoke(T sender) — the typed override point.
        virtual void invoke(T& sender) = 0;
    };

    class event_trigger
    {
    public:
        event_trigger() = default;

        // EventTrigger.Event: the name of the watched event. Refused (returns false, name unchanged)
        // once the trigger has been attached — C#'s IsSealed throw.
        bool set_event_name(std::string value)
        {
            if (event_name_ == value)
            {
                return true; // C# short-circuits an unchanged name even when sealed
            }
            if (sealed_)
            {
                return false;
            }
            event_name_ = std::move(value);
            return true;
        }
        [[nodiscard]] std::string_view event_name() const
        {
            return event_name_;
        }
        // EventTrigger.IsSealed — set on first attach; the actions/event become immutable.
        [[nodiscard]] bool is_sealed() const
        {
            return sealed_;
        }

        // EventTrigger.Actions: function-form (fluent). Refused once sealed (SealedList.IsReadOnly).
        event_trigger& add_action(trigger_action action)
        {
            if (!sealed_ && action)
            {
                actions_.push_back(std::move(action));
            }
            return *this;
        }
        // Class-form (TriggerAction<T>): shared ownership, wrapped into the function form.
        event_trigger& add_action(std::shared_ptr<trigger_action_base> action)
        {
            if (action != nullptr)
            {
                add_action(
                    [action = std::move(action)](maui::core::bindable_object& sender) { action->invoke(sender); });
            }
            return *this;
        }

        // Attach to `target`: seal, then subscribe to the named-event channel; every raise invokes all
        // actions with the target (OnEventTriggered). An empty/unregistered name subscribes nothing.
        // The returned handle unsubscribes when dropped; trigger and target must outlive it.
        [[nodiscard]] trigger_handle attach(element& target)
        {
            sealed_ = true; // TriggerBase.IsSealed flips on AttachTo
            maui::core::scoped_connection connection;
            if (!event_name_.empty())
            {
                connection = target.connect_named_event(event_name_, [this, &target] {
                    for (const trigger_action& action : actions_)
                    {
                        action(target);
                    }
                });
            }
            return trigger_handle{std::move(connection), [] { /* no setters to un-apply */ }};
        }

    private:
        std::string event_name_;
        std::vector<trigger_action> actions_;
        bool sealed_ = false;
    };
} // namespace maui::controls
