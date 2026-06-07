// Tests for maui::core::bindable_property + bindable_object (value precedence + notification).
// Derived from the C# Core.UnitTests (BindablePropertyUnitTests + the value-path BindableObject
// behavior). Properties are created as test locals so callbacks can capture per-test state.
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/setter_specificity.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace
{
    using maui::core::bindable_object;
    using maui::core::bindable_property;
    using maui::core::setter_specificity;

    struct test_bindable : bindable_object
    {
    };

    TEST(bindable, default_value_when_unset)
    {
        const bindable_property prop = bindable_property::create<std::string>("Text", "default");
        test_bindable obj;
        EXPECT_EQ(obj.get_value<std::string>(prop), "default");
    }

    TEST(bindable, value_type_default_is_zero_initialized)
    {
        const bindable_property prop = bindable_property::create<int>("Count"); // no explicit default
        test_bindable obj;
        EXPECT_EQ(obj.get_value<int>(prop), 0);
    }

    TEST(bindable, set_get_roundtrip)
    {
        const bindable_property prop = bindable_property::create<int>("Count", 0);
        test_bindable obj;
        obj.set_value<int>(prop, 7);
        EXPECT_EQ(obj.get_value<int>(prop), 7);
    }

    TEST(bindable, set_overrides_default)
    {
        const bindable_property prop = bindable_property::create<std::string>("Text", "default");
        test_bindable obj;
        obj.set_value<std::string>(prop, "value");
        EXPECT_EQ(obj.get_value<std::string>(prop), "value");
    }

    TEST(bindable, clear_reverts_to_default)
    {
        const bindable_property prop = bindable_property::create<std::string>("Text", "default");
        test_bindable obj;
        obj.set_value<std::string>(prop, "value");
        obj.clear_value(prop);
        EXPECT_EQ(obj.get_value<std::string>(prop), "default");
    }

    TEST(bindable, higher_specificity_wins_and_clear_reverts_to_lower)
    {
        const bindable_property prop = bindable_property::create<std::string>("Text", "default");
        test_bindable obj;
        obj.set_value(prop, std::any(std::string("binding")), setter_specificity::from_binding);
        obj.set_value(prop, std::any(std::string("manual")), setter_specificity::manual_value_setter);
        EXPECT_EQ(obj.get_value<std::string>(prop), "manual");

        // a lower-specificity set is kept but does not change the effective value
        obj.set_value(prop, std::any(std::string("binding2")), setter_specificity::from_binding);
        EXPECT_EQ(obj.get_value<std::string>(prop), "manual");

        // clearing the manual value reverts to the (updated) binding value
        obj.clear_value(prop, setter_specificity::manual_value_setter);
        EXPECT_EQ(obj.get_value<std::string>(prop), "binding2");
    }

    TEST(bindable, lower_specificity_set_does_not_notify)
    {
        const bindable_property prop = bindable_property::create<std::string>("Text", "default");
        test_bindable obj;
        obj.set_value(prop, std::any(std::string("manual")), setter_specificity::manual_value_setter);

        int changes = 0;
        obj.property_changed.connect([&](std::string_view) { ++changes; });
        obj.set_value(prop, std::any(std::string("binding")), setter_specificity::from_binding); // below manual
        EXPECT_EQ(changes, 0);
        EXPECT_EQ(obj.get_value<std::string>(prop), "manual");
    }

    TEST(bindable, changing_fires_before_changed_with_old_and_new)
    {
        bool changing_fired = false;
        bool changed_fired = false;
        std::string seen_old;
        std::string seen_new;
        const bindable_property prop = bindable_property::create<std::string>(
            "Foo", "Foo",
            {.property_changed =
                 [&](bindable_object &, const std::string &old_value, const std::string &new_value) {
                     EXPECT_TRUE(changing_fired);
                     changed_fired = true;
                     seen_old = old_value;
                     seen_new = new_value;
                 },
             .property_changing =
                 [&](bindable_object &, const std::string &, const std::string &) {
                     EXPECT_FALSE(changed_fired);
                     changing_fired = true;
                 }});
        test_bindable obj;
        obj.set_value<std::string>(prop, "Bar");
        EXPECT_TRUE(changing_fired);
        EXPECT_TRUE(changed_fired);
        EXPECT_EQ(seen_old, "Foo");
        EXPECT_EQ(seen_new, "Bar");
    }

    TEST(bindable, setting_same_value_does_not_notify)
    {
        const bindable_property prop = bindable_property::create<std::string>("Text", "default");
        test_bindable obj;
        obj.set_value<std::string>(prop, "value");

        int changes = 0;
        obj.property_changed.connect([&](std::string_view) { ++changes; });
        obj.set_value<std::string>(prop, "value"); // identical
        EXPECT_EQ(changes, 0);
    }

    TEST(bindable, coerce_is_applied_on_set)
    {
        const bindable_property prop = bindable_property::create<int>(
            "Count", 0, {.coerce_value = [](bindable_object &, int value) { return std::clamp(value, 0, 10); }});
        test_bindable obj;
        obj.set_value<int>(prop, 50);
        EXPECT_EQ(obj.get_value<int>(prop), 10);
    }

    TEST(bindable, validate_rejects_invalid_value)
    {
        const bindable_property prop = bindable_property::create<int>(
            "Count", 5, {.validate_value = [](bindable_object &, int value) { return value >= 0; }});
        test_bindable obj;
        obj.set_value<int>(prop, -1); // rejected
        EXPECT_EQ(obj.get_value<int>(prop), 5);
        obj.set_value<int>(prop, 7); // accepted
        EXPECT_EQ(obj.get_value<int>(prop), 7);
    }

    TEST(bindable, default_value_creator_runs_once_and_caches)
    {
        int creations = 0;
        const bindable_property prop =
            bindable_property::create<std::string>("Text", "", {.default_value_creator = [&](const bindable_object &) {
                                                       ++creations;
                                                       return std::string("created");
                                                   }});
        test_bindable obj;
        EXPECT_EQ(obj.get_value<std::string>(prop), "created");
        EXPECT_EQ(obj.get_value<std::string>(prop), "created");
        EXPECT_EQ(creations, 1); // materialized once, then cached in the context
    }

    TEST(bindable, handler_value_is_overridden_by_a_manual_set)
    {
        const bindable_property prop = bindable_property::create<std::string>("Text", "default");
        test_bindable obj;
        obj.set_value(prop, std::any(std::string("from-handler")), setter_specificity::from_handler);
        EXPECT_EQ(obj.get_value<std::string>(prop), "from-handler");

        obj.set_value(prop, std::any(std::string("manual")), setter_specificity::manual_value_setter);
        EXPECT_EQ(obj.get_value<std::string>(prop), "manual");

        // the handler value was removed, so clearing manual reverts to default, not the handler value
        obj.clear_value(prop, setter_specificity::manual_value_setter);
        EXPECT_EQ(obj.get_value<std::string>(prop), "default");
    }

    TEST(bindable, property_changed_event_carries_the_property_name)
    {
        const bindable_property prop = bindable_property::create<std::string>("Text", "default");
        test_bindable obj;
        std::vector<std::string> changing_names;
        std::vector<std::string> changed_names;
        obj.property_changing.connect([&](std::string_view name) { changing_names.emplace_back(name); });
        obj.property_changed.connect([&](std::string_view name) { changed_names.emplace_back(name); });
        obj.set_value<std::string>(prop, "value");
        EXPECT_EQ(changing_names, (std::vector<std::string>{"Text"}));
        EXPECT_EQ(changed_names, (std::vector<std::string>{"Text"}));
    }
} // namespace
