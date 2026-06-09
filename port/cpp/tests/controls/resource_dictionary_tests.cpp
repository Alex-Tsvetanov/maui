// Tests for maui::controls::resource_dictionary (M5d) — the string-keyed resource store + merged
// dictionaries + the values_changed event. Ported from ResourceDictionaryTests.cs (the in-memory store
// behaviors; XAML Source / system resources / style sheets are out of scope). The element-tree resource
// lookup (TryGetResource walking parents) is exercised in dynamic_resource_tests / implicit_style_tests.
#include "maui/controls/resource_dictionary.hpp"

#include <any>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace
{
    using maui::controls::resource_change;
    using maui::controls::resource_dictionary;

    TEST(resource_dictionary, add_then_get)
    {
        resource_dictionary rd;
        EXPECT_TRUE(rd.add("foo", std::string("bar")));
        ASSERT_NE(rd.get<std::string>("foo"), nullptr);
        EXPECT_EQ(*rd.get<std::string>("foo"), "bar");
    }

    TEST(resource_dictionary, set_overwrites)
    {
        resource_dictionary rd;
        rd.add("foo", std::string("FOO"));
        rd.set("foo", std::string("BAR")); // the indexer setter overwrites
        EXPECT_EQ(*rd.get<std::string>("foo"), "BAR");
    }

    TEST(resource_dictionary, add_rejects_a_duplicate_key)
    {
        resource_dictionary rd;
        EXPECT_TRUE(rd.add("foo", std::string("Foo")));
        EXPECT_FALSE(rd.add("foo", std::string("Bar"))); // duplicate add rejected (C# throws ArgumentException)
        EXPECT_EQ(*rd.get<std::string>("foo"), "Foo");   // original kept
    }

    TEST(resource_dictionary, triggers_values_changed_on_add)
    {
        resource_dictionary rd;
        bool fired = false;
        auto token = rd.values_changed.connect([&fired](const std::vector<resource_change>& values) {
            ASSERT_EQ(values.size(), 1U);
            EXPECT_EQ(values.front().key, "foo");
            EXPECT_EQ(*std::any_cast<std::string>(values.front().value), "FOO");
            fired = true;
        });
        rd.add("foo", std::string("FOO"));
        EXPECT_TRUE(fired);
        rd.values_changed.disconnect(token);
    }

    TEST(resource_dictionary, triggers_values_changed_on_change)
    {
        resource_dictionary rd;
        rd.add("foo", std::string("FOO"));
        bool fired = false;
        auto token = rd.values_changed.connect([&fired](const std::vector<resource_change>& values) {
            ASSERT_EQ(values.size(), 1U);
            EXPECT_EQ(*std::any_cast<std::string>(values.front().value), "BAR");
            fired = true;
        });
        rd.set("foo", std::string("BAR"));
        EXPECT_TRUE(fired);
        rd.values_changed.disconnect(token);
    }

    TEST(resource_dictionary, missing_key_returns_null)
    {
        resource_dictionary rd;
        rd.add("foo", std::string("bar"));
        EXPECT_EQ(rd.try_get("nope"), nullptr);
        EXPECT_EQ(rd.get<std::string>("nope"), nullptr);
    }

    TEST(resource_dictionary, typed_get_returns_null_for_a_mismatched_type)
    {
        resource_dictionary rd;
        rd.add("n", 42);
        EXPECT_NE(rd.get<int>("n"), nullptr);
        EXPECT_EQ(rd.get<std::string>("n"), nullptr); // stored as int, asked for string
    }

    TEST(resource_dictionary, merged_dictionary_resources_are_found)
    {
        resource_dictionary rd0;
        resource_dictionary merged;
        merged.add("foo", std::string("bar"));
        rd0.add_merged_dictionary(merged);
        ASSERT_NE(rd0.get<std::string>("foo"), nullptr);
        EXPECT_EQ(*rd0.get<std::string>("foo"), "bar");
    }

    TEST(resource_dictionary, last_merged_dictionary_takes_priority)
    {
        resource_dictionary rd0;
        resource_dictionary a;
        resource_dictionary b;
        resource_dictionary c;
        a.add("foo", std::string("bar"));
        b.add("foo", std::string("bar1"));
        c.add("foo", std::string("bar2"));
        rd0.add_merged_dictionary(a);
        rd0.add_merged_dictionary(b);
        rd0.add_merged_dictionary(c);
        EXPECT_EQ(*rd0.get<std::string>("foo"), "bar2"); // last merge wins
    }

    TEST(resource_dictionary, count_does_not_include_merged_dictionaries)
    {
        resource_dictionary rd;
        rd.add("baz", std::string("Baz"));
        rd.add("qux", std::string("Qux"));
        resource_dictionary merged;
        merged.add("foo", std::string("bar"));
        rd.add_merged_dictionary(merged);
        EXPECT_EQ(rd.count(), 2U); // merged dictionaries excluded from Count
    }

    TEST(resource_dictionary, adding_a_merged_dictionary_fires_values_changed_for_its_contents)
    {
        resource_dictionary rd;
        resource_dictionary merged;
        merged.add("foo", std::string("Foo"));
        bool fired = false;
        auto token = rd.values_changed.connect([&fired](const std::vector<resource_change>&) { fired = true; });
        rd.add_merged_dictionary(merged); // surfaces the merged dictionary's existing contents
        EXPECT_TRUE(fired);
        rd.values_changed.disconnect(token);
    }

    TEST(resource_dictionary, a_change_in_a_merged_dictionary_propagates_up)
    {
        resource_dictionary rd;
        resource_dictionary merged;
        rd.add_merged_dictionary(merged);
        bool fired = false;
        auto token = rd.values_changed.connect([&fired](const std::vector<resource_change>&) { fired = true; });
        merged.add("foo", std::string("Foo")); // a downstream add re-propagates through the outer dict
        EXPECT_TRUE(fired);
        EXPECT_EQ(*rd.get<std::string>("foo"), "Foo");
        rd.values_changed.disconnect(token);
    }

    TEST(resource_dictionary, clearing_merged_dictionaries_does_not_fire)
    {
        resource_dictionary rd;
        resource_dictionary merged;
        merged.add("foo", std::string("Foo"));
        rd.add_merged_dictionary(merged);
        ASSERT_EQ(*rd.get<std::string>("foo"), "Foo");

        bool fired = false;
        auto token = rd.values_changed.connect([&fired](const std::vector<resource_change>&) { fired = true; });
        rd.clear_merged_dictionaries(); // Reset deliberately does not fire (keeps resolved values aligned)
        EXPECT_FALSE(fired);
        EXPECT_EQ(rd.try_get("foo"), nullptr); // but the merged value is no longer resolvable
        rd.values_changed.disconnect(token);
    }
} // namespace
