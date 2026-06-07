// Tests for the typed bindable stack: bindable_property<T> + property<T> + bindable_object.
// Derived from the C# Core.UnitTests (BindablePropertyUnitTests + the BindableObject value path),
// but exercised through the strongly-typed property<T> API (no std::any). Each test owns its
// descriptor as a local so callbacks can capture per-test state; a generic test_view<T> wires one
// property<T> member to it.
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/property.hpp"
#include "maui/core/setter_specificity.hpp"

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

namespace
{
    using maui::core::bindable_object;
    using maui::core::bindable_property;
    using maui::core::property;
    using maui::core::setter_specificity;

    template <class T> struct test_view : bindable_object
    {
        property<T> value;
        explicit test_view(const bindable_property<T>& descriptor) : value(*this, descriptor)
        {
        }
    };

    TEST(bindable, default_value_when_unset)
    {
        const bindable_property<std::string> prop("Text", "default");
        test_view<std::string> const view(prop);
        EXPECT_EQ(view.value.get(), "default");
    }

    TEST(bindable, value_type_default_is_zero_initialized)
    {
        const bindable_property<int> prop("Count"); // default int{} == 0
        test_view<int> const view(prop);
        EXPECT_EQ(view.value.get(), 0);
    }

    TEST(bindable, set_get_roundtrip)
    {
        const bindable_property<int> prop("Count", 0);
        test_view<int> view(prop);
        view.value.set(7);
        EXPECT_EQ(view.value.get(), 7);
    }

    TEST(bindable, set_overrides_default)
    {
        const bindable_property<std::string> prop("Text", "default");
        test_view<std::string> view(prop);
        view.value.set("value");
        EXPECT_EQ(view.value.get(), "value");
    }

    TEST(bindable, clear_reverts_to_default)
    {
        const bindable_property<std::string> prop("Text", "default");
        test_view<std::string> view(prop);
        view.value.set("value");
        view.value.clear();
        EXPECT_EQ(view.value.get(), "default");
    }

    TEST(bindable, higher_specificity_wins_and_clear_reverts_to_lower)
    {
        const bindable_property<std::string> prop("Text", "default");
        test_view<std::string> view(prop);
        view.value.set("binding", setter_specificity::from_binding);
        view.value.set("manual", setter_specificity::manual_value_setter);
        EXPECT_EQ(view.value.get(), "manual");

        view.value.set("binding2", setter_specificity::from_binding); // below manual -> kept silently
        EXPECT_EQ(view.value.get(), "manual");

        view.value.clear(setter_specificity::manual_value_setter); // reverts to the updated binding value
        EXPECT_EQ(view.value.get(), "binding2");
    }

    TEST(bindable, lower_specificity_set_does_not_notify)
    {
        const bindable_property<std::string> prop("Text", "default");
        test_view<std::string> view(prop);
        view.value.set("manual", setter_specificity::manual_value_setter);

        int changes = 0;
        view.property_changed.connect([&](std::string_view) { ++changes; });
        view.value.set("binding", setter_specificity::from_binding); // below manual
        EXPECT_EQ(changes, 0);
        EXPECT_EQ(view.value.get(), "manual");
    }

    TEST(bindable, changing_fires_before_changed_with_old_and_new)
    {
        bool changing_fired = false;
        bool changed_fired = false;
        std::string seen_old;
        std::string seen_new;
        const bindable_property<std::string> prop(
            "Foo", "Foo",
            {.property_changed =
                 [&](bindable_object&, const std::string& old_value, const std::string& new_value) {
                     EXPECT_TRUE(changing_fired);
                     changed_fired = true;
                     seen_old = old_value;
                     seen_new = new_value;
                 },
             .property_changing =
                 [&](bindable_object&, const std::string&, const std::string&) {
                     EXPECT_FALSE(changed_fired);
                     changing_fired = true;
                 }});
        test_view<std::string> view(prop);
        view.value.set("Bar");
        EXPECT_TRUE(changing_fired);
        EXPECT_TRUE(changed_fired);
        EXPECT_EQ(seen_old, "Foo");
        EXPECT_EQ(seen_new, "Bar");
    }

    TEST(bindable, setting_same_value_does_not_notify)
    {
        const bindable_property<std::string> prop("Text", "default");
        test_view<std::string> view(prop);
        view.value.set("value");

        int changes = 0;
        view.property_changed.connect([&](std::string_view) { ++changes; });
        view.value.set("value"); // identical
        EXPECT_EQ(changes, 0);
    }

    TEST(bindable, coerce_is_applied_on_set)
    {
        const bindable_property<int> prop(
            "Count", 0, {.coerce_value = [](bindable_object&, int value) { return std::clamp(value, 0, 10); }});
        test_view<int> view(prop);
        view.value.set(50);
        EXPECT_EQ(view.value.get(), 10);
    }

    TEST(bindable, validate_rejects_invalid_value)
    {
        const bindable_property<int> prop("Count", 5,
                                          {.validate_value = [](bindable_object&, int value) { return value >= 0; }});
        test_view<int> view(prop);
        view.value.set(-1); // rejected
        EXPECT_EQ(view.value.get(), 5);
        view.value.set(7); // accepted
        EXPECT_EQ(view.value.get(), 7);
    }

    TEST(bindable, default_value_creator_runs_once_and_caches)
    {
        int creations = 0;
        const bindable_property<std::string> prop("Text", "", {.default_value_creator = [&](const bindable_object&) {
                                                      ++creations;
                                                      return std::string("created");
                                                  }});
        test_view<std::string> const view(prop);
        EXPECT_EQ(view.value.get(), "created");
        EXPECT_EQ(view.value.get(), "created");
        EXPECT_EQ(creations, 1);
    }

    TEST(bindable, handler_value_is_overridden_by_a_manual_set)
    {
        const bindable_property<std::string> prop("Text", "default");
        test_view<std::string> view(prop);
        view.value.set("from-handler", setter_specificity::from_handler);
        EXPECT_EQ(view.value.get(), "from-handler");

        view.value.set("manual", setter_specificity::manual_value_setter);
        EXPECT_EQ(view.value.get(), "manual");

        // the handler value was removed, so clearing manual reverts to default, not the handler value
        view.value.clear(setter_specificity::manual_value_setter);
        EXPECT_EQ(view.value.get(), "default");
    }

    TEST(bindable, notification_events_carry_the_property_name)
    {
        const bindable_property<std::string> prop("Text", "default");
        test_view<std::string> view(prop);
        std::vector<std::string> changing_names;
        std::vector<std::string> changed_names;
        view.property_changing.connect([&](std::string_view name) { changing_names.emplace_back(name); });
        view.property_changed.connect([&](std::string_view name) { changed_names.emplace_back(name); });
        view.value.set("value");
        EXPECT_EQ(changing_names, (std::vector<std::string>{"Text"}));
        EXPECT_EQ(changed_names, (std::vector<std::string>{"Text"}));
    }

    TEST(bindable, per_property_changed_event_delivers_typed_old_and_new)
    {
        const bindable_property<int> prop("Count", 0);
        test_view<int> view(prop);
        int seen_old = -1;
        int seen_new = -1;
        view.value.changed.connect([&](int old_value, int new_value) {
            seen_old = old_value;
            seen_new = new_value;
        });
        view.value.set(42);
        EXPECT_EQ(seen_old, 0);
        EXPECT_EQ(seen_new, 42);
    }
} // namespace
