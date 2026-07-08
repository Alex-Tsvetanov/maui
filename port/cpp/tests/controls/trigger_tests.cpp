// Tests for maui::controls::property_trigger (M5b) — a trigger watches a typed property and applies a
// setter bundle while it equals a target value, un-applying it (and restoring the value beneath) when it
// stops matching. A mock_object with public property<T> members stands in for a control (the same pattern
// the binding tests use), so the watched property is directly observable.
#include "maui/controls/trigger.hpp"

#include <string>

#include "maui/controls/setter.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/property.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::property_trigger;
    using maui::controls::setter;
    using maui::core::bindable_object;
    using maui::core::bindable_property;
    using maui::core::property;

    const bindable_property<bool>& is_on_prop()
    {
        static const bindable_property<bool> descriptor{"is_on", false};
        return descriptor;
    }
    const bindable_property<std::string>& label_prop()
    {
        static const bindable_property<std::string> descriptor{"label"};
        return descriptor;
    }

    struct mock_object : bindable_object
    {
        property<bool> is_on{*this, is_on_prop()};
        property<std::string> label{*this, label_prop()};
    };

    TEST(property_trigger, applies_setters_while_the_watched_value_matches)
    {
        mock_object target;
        property_trigger<bool> trigger{target.is_on, true};
        trigger.add(setter::of(label_prop(), std::string("ON")));

        auto handle = trigger.attach(target);
        EXPECT_EQ(target.label.get(), ""); // is_on is false at attach -> not applied

        target.is_on.set(true);
        EXPECT_EQ(target.label.get(), "ON"); // condition met -> setters applied

        target.is_on.set(false);
        EXPECT_EQ(target.label.get(), ""); // condition lost -> setters un-applied
    }

    TEST(property_trigger, applies_immediately_when_already_matching_at_attach)
    {
        mock_object target;
        target.is_on.set(true);

        property_trigger<bool> trigger{target.is_on, true};
        trigger.add(setter::of(label_prop(), std::string("ON")));

        auto handle = trigger.attach(target);
        EXPECT_EQ(target.label.get(), "ON"); // already matching at attach -> applied right away
    }

    TEST(property_trigger, trigger_outranks_a_manual_value_then_restores_it)
    {
        mock_object target;
        target.label.set("manual"); // a manual set (manual_value_setter)
        property_trigger<bool> trigger{target.is_on, true};
        trigger.add(setter::of(label_prop(), std::string("ON")));
        auto handle = trigger.attach(target);

        target.is_on.set(true);
        EXPECT_EQ(target.label.get(), "ON"); // trigger specificity outranks the manual value

        target.is_on.set(false);
        EXPECT_EQ(target.label.get(), "manual"); // de-activating restores the manual value beneath
    }

    TEST(property_trigger, teardown_unapplies_active_setters)
    {
        mock_object target;
        property_trigger<bool> trigger{target.is_on, true};
        trigger.add(setter::of(label_prop(), std::string("ON")));

        {
            auto handle = trigger.attach(target);
            target.is_on.set(true);
            EXPECT_EQ(target.label.get(), "ON");
        } // handle dropped -> trigger torn down -> setters un-applied

        EXPECT_EQ(target.label.get(), "");
    }

    // VisualElement.Triggers collection: adding a trigger attaches it to the owner now; clearing / destroying
    // the collection drops each trigger's RAII handle (un-apply). Type-erased over the unrelated trigger types.
    TEST(triggers_collection, add_attaches_now_and_clear_reverts)
    {
        using maui::controls::triggers_collection;
        mock_object target;
        triggers_collection coll{target};

        property_trigger<bool> trigger{target.is_on, true};
        trigger.add(setter::of(label_prop(), std::string("ON")));
        coll.add(std::move(trigger)); // attaches to target now (is_on false -> not yet applied)
        EXPECT_EQ(coll.count(), 1U);
        EXPECT_EQ(target.label.get(), "");

        target.is_on.set(true);
        EXPECT_EQ(target.label.get(), "ON"); // the collection-attached trigger applies

        coll.clear(); // drops the handle -> un-apply
        EXPECT_EQ(coll.count(), 0U);
        EXPECT_EQ(target.label.get(), "");
    }

    TEST(triggers_collection, destroying_the_collection_reverts_active_triggers)
    {
        mock_object target;
        {
            maui::controls::triggers_collection coll{target};
            property_trigger<bool> trigger{target.is_on, true};
            trigger.add(setter::of(label_prop(), std::string("ON")));
            coll.add(std::move(trigger));
            target.is_on.set(true);
            EXPECT_EQ(target.label.get(), "ON");
        } // coll destructs -> handle dropped -> setters un-applied
        EXPECT_EQ(target.label.get(), "");
    }
} // namespace
