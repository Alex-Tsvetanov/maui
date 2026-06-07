// Tests for maui::core::setter_specificity + setter_specificity_list<T>.
// The list tests are a port of src/Controls/tests/Core.UnitTests/SetterSpecificityListTests.cs
// (string values stand in for the C# object values; shared_ptr/weak_ptr stand in for the
// WeakReference no-leak checks). The ordering tests characterize the packed-uint64 precedence.
#include "maui/core/setter_specificity.hpp"
#include "maui/core/setter_specificity_list.hpp"

#include <memory>
#include <string>

#include <gtest/gtest.h>

namespace
{
    using maui::core::setter_specificity;
    using maui::core::setter_specificity_list;

    // ---- setter_specificity precedence ordering (the packed-uint64 contract) ----
    TEST(specificity, ascending_precedence_order)
    {
        EXPECT_TRUE(setter_specificity::default_value < setter_specificity::from_binding);
        EXPECT_TRUE(setter_specificity::from_binding < setter_specificity::dynamic_resource_setter);
        EXPECT_TRUE(setter_specificity::dynamic_resource_setter < setter_specificity::manual_value_setter);
        EXPECT_TRUE(setter_specificity::manual_value_setter < setter_specificity::trigger);
        EXPECT_TRUE(setter_specificity::trigger < setter_specificity::visual_state_setter);
        EXPECT_TRUE(setter_specificity::visual_state_setter < setter_specificity::from_handler);
    }

    TEST(specificity, classification_and_default)
    {
        EXPECT_TRUE(setter_specificity::default_value.is_default());
        EXPECT_TRUE(setter_specificity::from_handler.is_handler());
        EXPECT_FALSE(setter_specificity::manual_value_setter.is_default());
        EXPECT_FALSE(setter_specificity::manual_value_setter.is_handler());
        EXPECT_EQ(setter_specificity{}, setter_specificity::default_value); // default ctor == DefaultValue
    }

    // ---- setter_specificity_list<T> ----
    TEST(specificity_list, no_values)
    {
        setter_specificity_list<std::string> const list;
        auto const pair = list.specificity_and_value();
        EXPECT_EQ(pair.second, std::string{});
        EXPECT_EQ(pair.first, setter_specificity{});
    }

    TEST(specificity_list, overrides_value_with_same_specificity)
    {
        setter_specificity_list<std::string> list;
        list.set(setter_specificity::manual_value_setter, "initial");
        list.set(setter_specificity::manual_value_setter, "new");
        EXPECT_EQ(list.count(), 1U);
        EXPECT_EQ(list.value(), "new");
        EXPECT_EQ(list.specificity(), setter_specificity::manual_value_setter);
    }

    TEST(specificity_list, removing_value_releases_it)
    {
        setter_specificity_list<std::shared_ptr<int>> list;
        list.set(setter_specificity::default_value, std::make_shared<int>(1));
        list.set(setter_specificity::from_handler, std::make_shared<int>(2));
        std::weak_ptr<int> weak;
        {
            auto held = std::make_shared<int>(3);
            weak = held;
            list.set(setter_specificity::from_binding, held);
        }
        EXPECT_FALSE(weak.expired()); // list still holds it
        list.remove(setter_specificity::from_binding);
        EXPECT_TRUE(weak.expired()); // removed -> destroyed, no leak
    }

    TEST(specificity_list, removing_last_value_releases_it)
    {
        setter_specificity_list<std::shared_ptr<int>> list;
        std::weak_ptr<int> weak;
        {
            auto held = std::make_shared<int>(1);
            weak = held;
            list.set(setter_specificity::manual_value_setter, held);
        }
        EXPECT_FALSE(weak.expired());
        list.remove(setter_specificity::manual_value_setter);
        EXPECT_TRUE(weak.expired());
    }

    TEST(specificity_list, get_value_for_specificity)
    {
        setter_specificity_list<std::string> list;
        list.set(setter_specificity::default_value, "default");
        list.set(setter_specificity::manual_value_setter, "manual");
        EXPECT_EQ(list.get(setter_specificity::default_value), "default");
    }

    TEST(specificity_list, default_when_no_value_matches_specificity)
    {
        setter_specificity_list<std::string> list;
        list.set(setter_specificity::default_value, "default");
        list.set(setter_specificity::manual_value_setter, "manual");
        EXPECT_EQ(list.get(setter_specificity::from_handler), std::string{});
    }

    TEST(specificity_list, one_value)
    {
        setter_specificity_list<std::string> list;
        list.set(setter_specificity::manual_value_setter, "manual");
        EXPECT_EQ(list.value(), "manual");
        EXPECT_EQ(list.specificity(), setter_specificity::manual_value_setter);

        list.set(setter_specificity::default_value, "default"); // lower, doesn't win
        EXPECT_EQ(list.value(), "manual");
        EXPECT_EQ(list.specificity(), setter_specificity::manual_value_setter);
    }

    TEST(specificity_list, two_values_remove_top_reverts)
    {
        setter_specificity_list<std::string> list;
        list.set(setter_specificity::default_value, "default");
        list.set(setter_specificity::manual_value_setter, "manual");
        EXPECT_EQ(list.value(), "manual");

        list.remove(setter_specificity::manual_value_setter);
        EXPECT_EQ(list.value(), "default");
        EXPECT_EQ(list.specificity(), setter_specificity::default_value);
    }

    TEST(specificity_list, three_values_remove_top_reverts_to_next)
    {
        setter_specificity_list<std::string> list;
        list.set(setter_specificity::default_value, "default");
        list.set(setter_specificity::from_binding, "binding");
        list.set(setter_specificity::manual_value_setter, "manual");
        EXPECT_EQ(list.value(), "manual");

        list.remove(setter_specificity::manual_value_setter);
        EXPECT_EQ(list.value(), "binding");
        EXPECT_EQ(list.specificity(), setter_specificity::from_binding);
    }

    TEST(specificity_list, many_values_highest_wins)
    {
        setter_specificity_list<std::string> list;
        list.set(setter_specificity::default_value, "default");
        list.set(setter_specificity::from_binding, "binding");
        list.set(setter_specificity::dynamic_resource_setter, "dynamic");
        list.set(setter_specificity::manual_value_setter, "manual");
        list.set(setter_specificity::trigger, "trigger");
        EXPECT_EQ(list.value(), "trigger");

        list.remove(setter_specificity::manual_value_setter); // still below trigger
        EXPECT_EQ(list.value(), "trigger");
        EXPECT_EQ(list.specificity(), setter_specificity::trigger);
    }

    TEST(specificity_list, get_cleared_value)
    {
        setter_specificity_list<std::string> list;
        list.set(setter_specificity::default_value, "default");
        EXPECT_EQ(list.cleared_value(), std::string{}); // only one entry
        EXPECT_EQ(list.cleared_specificity(), setter_specificity{});

        list.set(setter_specificity::manual_value_setter, "manual");
        EXPECT_EQ(list.cleared_value(), "default"); // removing the top would revert to default
        EXPECT_EQ(list.cleared_specificity(), setter_specificity::default_value);
    }

    TEST(specificity_list, get_cleared_value_for_specificity)
    {
        setter_specificity_list<std::string> list;
        list.set(setter_specificity::default_value, "default");
        EXPECT_EQ(list.cleared_value(setter_specificity::default_value), std::string{});

        list.set(setter_specificity::manual_value_setter, "manual");
        // removing manual (the top) reverts to default
        EXPECT_EQ(list.cleared_value(setter_specificity::manual_value_setter), "default");
        // handler isn't present and isn't the top, so the effective value is unchanged (manual)
        EXPECT_EQ(list.cleared_value(setter_specificity::from_handler), "manual");
    }
} // namespace
