// Tests for DynamicResource (M5d) — binding a property to a resource KEY so it re-applies when the
// resource changes or the element reparents. Ported from DynamicResourceTests.cs, using `label` (its
// `text` property is a std::string, matching the resource value type) + `vertical_stack_layout` /
// `content_page` parent chains. The DynamicResource value lands at setter_specificity::dynamic_resource,
// above a binding but below a manual set. Application.Current fallback is out of scope here.
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"

#include <string>

#include <gtest/gtest.h>

namespace
{
    using maui::controls::content_page;
    using maui::controls::label;
    using maui::controls::vertical_stack_layout;

    TEST(dynamic_resource, resolves_from_own_resources_when_the_key_is_set_after)
    {
        label lbl;
        lbl.set_dynamic_resource("text", "foo");
        EXPECT_EQ(lbl.text(), ""); // unresolved -> the descriptor default

        lbl.resources().add("foo", std::string("FOO"));
        EXPECT_EQ(lbl.text(), "FOO"); // adding the resource re-applies the bound property
    }

    TEST(dynamic_resource, resolves_when_the_key_already_exists_at_bind_time)
    {
        label lbl;
        lbl.resources().add("foo", std::string("FOO"));
        EXPECT_EQ(lbl.text(), "");

        lbl.set_dynamic_resource("text", "foo");
        EXPECT_EQ(lbl.text(), "FOO"); // resolved immediately at bind time
    }

    TEST(dynamic_resource, resolves_from_a_parent_resource_set_after_attach)
    {
        label lbl;
        vertical_stack_layout layout;
        layout.add(lbl);
        lbl.set_dynamic_resource("text", "foo");
        EXPECT_EQ(lbl.text(), "");

        layout.resources().add("foo", std::string("FOO")); // a parent resource change reaches the child
        EXPECT_EQ(lbl.text(), "FOO");
    }

    TEST(dynamic_resource, resolves_from_a_parent_when_attached_after_the_resource_exists)
    {
        label lbl;
        lbl.set_dynamic_resource("text", "foo");
        vertical_stack_layout layout;
        layout.resources().add("foo", std::string("FOO"));
        EXPECT_EQ(lbl.text(), ""); // not yet a child

        layout.add(lbl); // attaching re-resolves against the new parent chain
        EXPECT_EQ(lbl.text(), "FOO");
    }

    TEST(dynamic_resource, changing_the_resource_value_updates_the_property)
    {
        label lbl;
        lbl.resources().add("foo", std::string("FOO"));
        lbl.set_dynamic_resource("text", "foo");
        EXPECT_EQ(lbl.text(), "FOO");

        lbl.resources().set("foo", std::string("BAR"));
        EXPECT_EQ(lbl.text(), "BAR");
    }

    TEST(dynamic_resource, remove_stops_updating)
    {
        label lbl;
        lbl.resources().add("foo", std::string("FOO"));
        lbl.set_dynamic_resource("text", "foo");
        EXPECT_EQ(lbl.text(), "FOO");

        lbl.remove_dynamic_resource("text");
        lbl.resources().set("foo", std::string("BAR"));
        EXPECT_EQ(lbl.text(), "FOO"); // the value is kept; future changes no longer apply
    }

    TEST(dynamic_resource, reparent_resubscribes_to_the_new_chain)
    {
        content_page page0;
        content_page page1;
        page0.resources().add("foo", std::string("FOO"));
        page1.resources().add("foo", std::string("BAR"));

        label lbl;
        lbl.set_dynamic_resource("text", "foo");
        EXPECT_EQ(lbl.text(), "");

        page0.set_content(lbl);
        EXPECT_EQ(lbl.text(), "FOO");

        page0.set_content(nullptr); // detach
        page1.set_content(lbl);     // reparent -> re-resolve against page1's resources
        EXPECT_EQ(lbl.text(), "BAR");
    }

    TEST(dynamic_resource, cleared_resources_do_not_clear_values)
    {
        content_page page;
        page.resources().add("foo", std::string("FOO"));
        label lbl;
        lbl.set_dynamic_resource("text", "foo");
        page.set_content(lbl);
        EXPECT_EQ(lbl.text(), "FOO");

        page.resources().clear_merged_dictionaries(); // clearing the merged set does not push a change
        EXPECT_EQ(lbl.text(), "FOO");                 // value retained
    }

    TEST(dynamic_resource, a_manual_set_outranks_a_dynamic_resource)
    {
        label lbl;
        lbl.resources().add("foo", std::string("FOO"));
        lbl.set_dynamic_resource("text", "foo");
        EXPECT_EQ(lbl.text(), "FOO");

        lbl.set_text("manual"); // manual_value_setter > dynamic_resource_setter
        EXPECT_EQ(lbl.text(), "manual");
    }
} // namespace
