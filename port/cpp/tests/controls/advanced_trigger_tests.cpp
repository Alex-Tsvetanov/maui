// Tests for the advanced triggers (M5d): EnterActions/ExitActions + Setter.TargetName on property_trigger,
// MultiTrigger (multiple conditions), and data_trigger (a typed BindingContext condition). Ported from
// TriggerTests.cs (EnterAndExitActionsTriggered), MultiTriggerTests.cs, and DataTriggerTests.cs. A
// mock_object with public property<T> members stands in for a control (the binding/trigger test pattern).
#include "maui/controls/trigger.hpp"

#include <memory>
#include <string>

#include "maui/controls/setter.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/property.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::data_trigger;
    using maui::controls::multi_trigger;
    using maui::controls::property_trigger;
    using maui::controls::setter;
    using maui::core::bindable_object;
    using maui::core::bindable_property;
    using maui::core::property;

    const bindable_property<bool>& flag_prop()
    {
        static const bindable_property<bool> descriptor{"flag", false};
        return descriptor;
    }
    const bindable_property<std::string>& cond_prop()
    {
        static const bindable_property<std::string> descriptor{"cond"};
        return descriptor;
    }
    const bindable_property<std::string>& out_prop()
    {
        static const bindable_property<std::string> descriptor{"out"};
        return descriptor;
    }

    struct mock_object : bindable_object
    {
        property<bool> flag{*this, flag_prop()};
        property<std::string> cond{*this, cond_prop()};
        property<std::string> out{*this, out_prop()};
    };

    // ---- EnterActions / ExitActions (TriggerTests.EnterAndExitActionsTriggered) ----
    TEST(property_trigger, enter_and_exit_actions_fire_on_condition_change)
    {
        mock_object target;
        int enters = 0;
        int exits = 0;
        property_trigger<bool> trigger{target.flag, true};
        trigger.add_enter_action([&enters](bindable_object&) { ++enters; });
        trigger.add_exit_action([&exits](bindable_object&) { ++exits; });

        auto handle = trigger.attach(target);
        EXPECT_EQ(enters, 0); // flag is false at attach
        EXPECT_EQ(exits, 0);

        target.flag.set(true);
        EXPECT_EQ(enters, 1); // condition became true -> EnterActions
        EXPECT_EQ(exits, 0);

        target.flag.set(false);
        EXPECT_EQ(enters, 1);
        EXPECT_EQ(exits, 1); // condition became false -> ExitActions
    }

    // ---- Setter.TargetName: a trigger setter retargets another element ----
    TEST(property_trigger, target_name_setter_writes_to_another_element)
    {
        mock_object source;     // owns the watched condition
        mock_object retargeted; // the setter's TargetName target
        property_trigger<bool> trigger{source.flag, true};
        trigger.add(setter::of_for(retargeted, out_prop(), std::string("ON")));

        auto handle = trigger.attach(source);
        EXPECT_EQ(retargeted.out.get(), "");

        source.flag.set(true);
        EXPECT_EQ(retargeted.out.get(), "ON"); // applied to the retargeted element, not the source
        EXPECT_EQ(source.out.get(), "");       // the source is untouched

        source.flag.set(false);
        EXPECT_EQ(retargeted.out.get(), ""); // de-activation un-applies on the retargeted element
    }

    // ---- MultiTrigger (MultiTriggerTests) ----
    TEST(multi_trigger, applies_only_while_all_conditions_hold)
    {
        mock_object target;
        target.out.set("default");
        multi_trigger trigger;
        trigger.add_condition(target.cond, std::string("foobar"));
        trigger.add_condition(target.flag, true);
        trigger.add(setter::of(out_prop(), std::string("qux")));

        auto handle = trigger.attach(target);
        EXPECT_EQ(target.out.get(), "default"); // both false

        target.cond.set("foobar");
        EXPECT_EQ(target.out.get(), "default"); // one condition false

        target.flag.set(true);
        EXPECT_EQ(target.out.get(), "qux"); // both conditions true

        target.cond.set("");
        EXPECT_EQ(target.out.get(), "default"); // one condition false again
    }

    TEST(multi_trigger, applies_immediately_when_all_conditions_match_at_attach)
    {
        mock_object target;
        target.out.set("default");
        target.cond.set("foobar");
        target.flag.set(true);

        multi_trigger trigger;
        trigger.add_condition(target.cond, std::string("foobar"));
        trigger.add_condition(target.flag, true);
        trigger.add(setter::of(out_prop(), std::string("qux")));

        auto handle = trigger.attach(target);
        EXPECT_EQ(target.out.get(), "qux"); // both already true at attach
    }

    TEST(multi_trigger, teardown_unapplies_active_setters)
    {
        mock_object target;
        target.out.set("default");
        target.cond.set("foobar");
        target.flag.set(true);

        multi_trigger trigger;
        trigger.add_condition(target.cond, std::string("foobar"));
        trigger.add_condition(target.flag, true);
        trigger.add(setter::of(out_prop(), std::string("qux")));
        {
            auto handle = trigger.attach(target);
            EXPECT_EQ(target.out.get(), "qux");
        } // dropped -> torn down
        EXPECT_EQ(target.out.get(), "default");
    }

    // ---- data_trigger (DataTriggerTests): a typed BindingContext condition ----
    struct view_model
    {
        std::string foo;
    };

    TEST(data_trigger, applies_when_the_bound_value_matches)
    {
        mock_object target;
        target.out.set("default");
        // The DataTrigger reads `view_model::foo` off the binding context, matching "foobar".
        data_trigger<view_model, std::string> trigger{[](const view_model& vm) { return vm.foo; },
                                                      std::string("foobar")};
        trigger.add(setter::of(out_prop(), std::string("qux")));
        auto handle = trigger.attach(target);
        EXPECT_EQ(target.out.get(), "default"); // no context yet

        target.set_binding_context(std::make_shared<view_model>(view_model{.foo = "foobar"}));
        EXPECT_EQ(target.out.get(), "qux"); // context matches -> applied

        target.set_binding_context(std::make_shared<view_model>(view_model{.foo = ""}));
        EXPECT_EQ(target.out.get(), "default"); // context no longer matches -> un-applied
    }

    TEST(data_trigger, applies_on_attach_if_the_context_already_matches)
    {
        mock_object target;
        target.out.set("default");
        target.set_binding_context(std::make_shared<view_model>(view_model{.foo = "foobar"}));

        data_trigger<view_model, std::string> trigger{[](const view_model& vm) { return vm.foo; },
                                                      std::string("foobar")};
        trigger.add(setter::of(out_prop(), std::string("qux")));
        auto handle = trigger.attach(target);
        EXPECT_EQ(target.out.get(), "qux"); // matched at attach
    }
} // namespace
