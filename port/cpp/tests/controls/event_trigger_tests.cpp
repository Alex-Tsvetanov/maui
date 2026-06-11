// Tests for maui::controls::event_trigger + typed_trigger_action<T> (W1-15). Ported from
// EventTriggerTest.cs: TestTriggerActionInvoked, TestChangeEventOnEventTrigger (the wrong event does not
// fire; changing Event after attach is refused — C# throws on the sealed trigger, the port returns
// false). The mock element registers its events through the NAMED-EVENT seam (element::
// register_named_event), the port's reflection-free substitute for C#'s GetRuntimeEvent.
#include "maui/controls/event_trigger.hpp"

#include <functional>
#include <memory>
#include <string>
#include <utility>

#include "maui/controls/button.hpp"
#include "maui/controls/element.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/event.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::element;
    using maui::controls::event_trigger;
    using maui::controls::trigger_handle;
    using maui::controls::typed_trigger_action;
    using maui::core::bindable_object;

    // MockTriggerAction (TriggerAction<BindableObject>): records the invocation.
    struct mock_trigger_action : typed_trigger_action<bindable_object>
    {
        bool invoked = false;

    protected:
        void invoke(bindable_object& /*sender*/) override
        {
            invoked = true;
        }
    };

    // MockBindableWithEvent: a control-side element with two named events.
    struct mock_element_with_event : element
    {
        maui::core::event<> mock_event;
        maui::core::event<> mock_event2;

        mock_element_with_event()
        {
            register_named_event("mock_event", [this](std::function<void()> handler) {
                return maui::core::connect_scoped(mock_event, std::move(handler));
            });
            register_named_event("mock_event2", [this](std::function<void()> handler) {
                return maui::core::connect_scoped(mock_event2, std::move(handler));
            });
        }

        void fire_event()
        {
            mock_event.raise();
        }
        void fire_event2()
        {
            mock_event2.raise();
        }
    };

    TEST(event_trigger, action_invoked_when_the_named_event_raises)
    {
        mock_element_with_event bindable;
        auto action = std::make_shared<mock_trigger_action>();
        event_trigger trigger;
        EXPECT_TRUE(trigger.set_event_name("mock_event"));
        trigger.add_action(action);

        trigger_handle handle = trigger.attach(bindable);
        EXPECT_FALSE(action->invoked);
        bindable.fire_event();
        EXPECT_TRUE(action->invoked);
    }

    TEST(event_trigger, the_wrong_event_does_not_invoke_and_renaming_after_attach_is_refused)
    {
        mock_element_with_event bindable;
        auto action = std::make_shared<mock_trigger_action>();
        event_trigger trigger;
        trigger.set_event_name("mock_event");
        trigger.add_action(action);
        trigger_handle handle = trigger.attach(bindable);

        action->invoked = false;
        bindable.fire_event();
        EXPECT_TRUE(action->invoked);

        action->invoked = false;
        bindable.fire_event2(); // the OTHER event — not subscribed
        EXPECT_FALSE(action->invoked);

        // C#: `eventtrigger.Event = "MockEvent2"` throws InvalidOperationException once sealed.
        EXPECT_TRUE(trigger.is_sealed());
        EXPECT_FALSE(trigger.set_event_name("mock_event2"));
        EXPECT_EQ(trigger.event_name(), "mock_event");
    }

    TEST(event_trigger, dropping_the_handle_unsubscribes)
    {
        mock_element_with_event bindable;
        int fired = 0;
        event_trigger trigger;
        trigger.set_event_name("mock_event");
        trigger.add_action([&fired](bindable_object&) { ++fired; });
        {
            trigger_handle handle = trigger.attach(bindable);
            bindable.fire_event();
            EXPECT_EQ(fired, 1);
        }
        bindable.fire_event(); // the RAII handle tore the subscription down
        EXPECT_EQ(fired, 1);
    }

    TEST(event_trigger, an_unregistered_event_name_attaches_nothing)
    {
        // C# logs a warning when the event cannot be resolved and attaches no handler.
        mock_element_with_event bindable;
        int fired = 0;
        event_trigger trigger;
        trigger.set_event_name("does_not_exist");
        trigger.add_action([&fired](bindable_object&) { ++fired; });
        trigger_handle handle = trigger.attach(bindable);
        bindable.fire_event();
        EXPECT_EQ(fired, 0);
    }

    TEST(event_trigger, attaches_to_a_real_control_event_by_name)
    {
        // The control-side registration: button registers "clicked"/"pressed"/"released" channels.
        maui::controls::button btn;
        int clicks = 0;
        event_trigger trigger;
        trigger.set_event_name("clicked");
        trigger.add_action([&clicks](bindable_object&) { ++clicks; });
        trigger_handle handle = trigger.attach(btn);

        btn.send_clicked();
        EXPECT_EQ(clicks, 1);
    }

    TEST(event_trigger, typed_trigger_action_no_ops_on_a_sender_type_mismatch)
    {
        // TriggerAction<T>'s typed dispatch casts the sender; the port no-ops instead of throwing.
        struct button_only_action : typed_trigger_action<maui::controls::button>
        {
            int typed_invocations = 0;

        protected:
            void invoke(maui::controls::button& /*sender*/) override
            {
                ++typed_invocations;
            }
        };

        auto action = std::make_shared<button_only_action>();
        mock_element_with_event not_a_button;
        event_trigger trigger;
        trigger.set_event_name("mock_event");
        trigger.add_action(action);
        trigger_handle handle = trigger.attach(not_a_button);

        not_a_button.fire_event();
        EXPECT_EQ(action->typed_invocations, 0); // mismatched sender — silently skipped
    }
} // namespace
