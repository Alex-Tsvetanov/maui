// Tests for maui::core::property_path + binding_expression + the bindable_object name->getter
// channel (W1-02). Ported from src/Controls/tests/Core.UnitTests/BindingExpressionTests.cs (Ctor /
// ApplyNull / InvalidPaths / ValidPaths / TryConvertWithNumbersAndCultures — the port parses
// invariantly, so the culture axis collapses to the invariant rows) plus port-specific pins for the
// getter channel that replaces C#'s reflection.
#include "maui/core/property_path.hpp"

#include <any>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/binding_expression.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/boxed_value.hpp"
#include "maui/core/property.hpp"
#include "maui/core/setter_specificity.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::core::bindable_object;
    using maui::core::bindable_property;
    using maui::core::binding_expression;
    using maui::core::binding_mode;
    using maui::core::binding_source_node;
    using maui::core::box_value;
    using maui::core::boxed_to_string;
    using maui::core::property;
    using maui::core::property_path;
    using maui::core::setter_specificity;
    using maui::core::try_unbox;

    // ---- property_path parsing (BindingExpressionTests.ValidPaths / InvalidPaths) ----

    TEST(property_path, parses_simple_and_complex_paths)
    {
        const property_path simple = property_path::parse("Foo");
        ASSERT_EQ(simple.parts().size(), 2U); // the seeded self part + "Foo"
        EXPECT_TRUE(simple.parts()[0].is_self);
        EXPECT_EQ(simple.parts()[1].content, "Foo");

        const property_path complex = property_path::parse("Foo.Bar[1]");
        ASSERT_EQ(complex.parts().size(), 4U);
        EXPECT_TRUE(complex.parts()[0].is_self);
        EXPECT_EQ(complex.parts()[1].content, "Foo");
        EXPECT_EQ(complex.parts()[2].content, "Bar");
        EXPECT_TRUE(complex.parts()[3].is_indexer);
        EXPECT_EQ(complex.parts()[3].content, "1");
    }

    TEST(property_path, valid_paths_parse_with_surrounding_whitespace)
    {
        // C# ValidPaths: every path x {space before, space after} combination parses.
        const char* paths[] = {".",   "[1]",     "[1 ]",     ".[1]",       ". [1]",
                               "Foo", "Foo.Bar", "Foo. Bar", "Foo.Bar[1]", "Foo.Bar [1]"};
        for (const char* path : paths)
        {
            for (const std::string& decorated :
                 {std::string{path}, " " + std::string{path}, std::string{path} + " ", " " + std::string{path} + " "})
            {
                EXPECT_NO_THROW((void)property_path::parse(decorated)) << decorated;
            }
        }
    }

    TEST(property_path, self_path_is_a_single_self_part)
    {
        const property_path self = property_path::parse(".");
        ASSERT_EQ(self.parts().size(), 1U);
        EXPECT_TRUE(self.parts()[0].is_self);

        const property_path self_indexed = property_path::parse(".[1]");
        ASSERT_EQ(self_indexed.parts().size(), 2U);
        EXPECT_TRUE(self_indexed.parts()[0].is_self);
        EXPECT_TRUE(self_indexed.parts()[1].is_indexer);
    }

    TEST(property_path, invalid_paths_throw)
    {
        // C# InvalidPaths: "Foo." / "Foo[]" / "Foo.Bar[]" / "Foo[1" throw FormatException.
        EXPECT_THROW((void)property_path::parse("Foo."), std::invalid_argument);
        EXPECT_THROW((void)property_path::parse("Foo[]"), std::invalid_argument);
        EXPECT_THROW((void)property_path::parse("Foo.Bar[]"), std::invalid_argument);
        EXPECT_THROW((void)property_path::parse("Foo[1"), std::invalid_argument);
        EXPECT_THROW((void)property_path::parse(""), std::invalid_argument);
        EXPECT_THROW((void)property_path::parse("Foo..Bar"), std::invalid_argument);
    }

    // ---- the boxed-value conversion lattice (BindingExpressionHelper.TryConvert) ----

    TEST(boxed_value, string_to_number_invariant)
    {
        EXPECT_EQ(try_unbox<double>(std::any{std::string{"4.2"}}), 4.2);
        EXPECT_EQ(try_unbox<double>(std::any{std::string{"-4.2"}}), -4.2);
        EXPECT_EQ(try_unbox<float>(std::any{std::string{"4.2"}}), 4.2F);
        EXPECT_EQ(try_unbox<int>(std::any{std::string{"42"}}), 42);
    }

    TEST(boxed_value, editing_guards_do_not_convert)
    {
        // bugzilla 32871: "4." and "-0" must not canonicalize while the user is typing.
        EXPECT_EQ(try_unbox<double>(std::any{std::string{"4."}}), std::nullopt);
        EXPECT_EQ(try_unbox<double>(std::any{std::string{"-0"}}), std::nullopt);
        EXPECT_EQ(try_unbox<double>(std::any{std::string{"-0.0"}}), std::nullopt);
        EXPECT_EQ(try_unbox<double>(std::any{std::string{"-0.5"}}).value_or(0), -0.5); // still converts
        EXPECT_EQ(try_unbox<double>(std::any{std::string{"0.5"}}).value_or(0), 0.5);
    }

    TEST(boxed_value, number_to_string_and_cross_numeric)
    {
        EXPECT_EQ(try_unbox<std::string>(std::any{0.9}), "0.9");
        EXPECT_EQ(try_unbox<std::string>(std::any{42}), "42");
        EXPECT_EQ(try_unbox<double>(std::any{1}), 1.0);
        EXPECT_EQ(try_unbox<int>(std::any{2.0}), 2);
        EXPECT_EQ(boxed_to_string(std::any{std::string{"x"}}), "x");
    }

    TEST(boxed_value, null_only_unboxes_to_nullable)
    {
        // C#: null is assignable to reference/nullable types only. std::string stands in for the
        // (reference-type) C# string, so it absorbs null as "" — but a value type like int fails.
        EXPECT_EQ(try_unbox<std::string>(std::any{}), std::string{});
        EXPECT_EQ(try_unbox<int>(std::any{}), std::nullopt);
        EXPECT_EQ(try_unbox<double>(std::any{}), std::nullopt);
        const auto null_ptr = try_unbox<std::shared_ptr<std::string>>(std::any{});
        ASSERT_TRUE(null_ptr.has_value());
        EXPECT_EQ(*null_ptr, nullptr);
        EXPECT_FALSE(box_value(std::shared_ptr<std::string>{}).has_value()); // null boxes as empty any
    }

    TEST(boxed_value, shared_ptr_unwraps_to_value)
    {
        const auto text = std::make_shared<std::string>("hello");
        EXPECT_EQ(try_unbox<std::string>(std::any{text}), "hello");
    }

    // ---- the name->getter channel ----

    const bindable_property<std::string>& text_prop()
    {
        static const bindable_property<std::string> descriptor{"text", std::string{"fallback-default"}};
        return descriptor;
    }
    const bindable_property<int>& count_prop()
    {
        static const bindable_property<int> descriptor{"count", 7};
        return descriptor;
    }

    struct child_object;

    const bindable_property<std::shared_ptr<child_object>>& child_prop()
    {
        static const bindable_property<std::shared_ptr<child_object>> descriptor{"child"};
        return descriptor;
    }

    struct child_object : bindable_object
    {
        property<std::string> text{*this, text_prop()};
    };

    struct mock_object : bindable_object
    {
        property<std::string> text{*this, text_prop()};
        property<int> count{*this, count_prop()};
        property<std::shared_ptr<child_object>> child{*this, child_prop()};
    };

    TEST(getter_channel, try_get_value_boxes_the_current_value)
    {
        mock_object source;
        source.text.set("abc");
        const auto boxed = source.try_get_value("text");
        ASSERT_TRUE(boxed.has_value());
        EXPECT_EQ(std::any_cast<std::string>(*boxed), "abc");
        EXPECT_EQ(source.try_get_value("missing"), std::nullopt);
        EXPECT_TRUE(source.has_property("text"));
        EXPECT_FALSE(source.has_property("missing"));
    }

    TEST(getter_channel, try_get_object_exposes_walkable_nodes)
    {
        mock_object source;
        EXPECT_EQ(source.try_get_object("child"), nullptr); // null value
        auto child = std::make_shared<child_object>();
        source.child.set(child);
        EXPECT_EQ(source.try_get_object("child"), child);
        EXPECT_EQ(source.try_get_object("text"), nullptr); // not an object-valued property
    }

    TEST(getter_channel, try_set_value_converts_through_the_lattice)
    {
        mock_object source;
        EXPECT_TRUE(source.try_set_value("count", std::any{std::string{"42"}}, setter_specificity::from_binding));
        EXPECT_EQ(source.count.get(), 42);
        EXPECT_FALSE(source.try_set_value("count", std::any{std::string{"nope"}}, setter_specificity::from_binding));
        EXPECT_EQ(source.count.get(), 42);
        EXPECT_FALSE(source.try_set_value("missing", std::any{1}, setter_specificity::from_binding));
    }

    TEST(getter_channel, metadata_and_default_value)
    {
        const mock_object source;
        EXPECT_EQ(source.property_default_binding_mode("text"), binding_mode::one_way);
        EXPECT_EQ(source.property_is_read_only("text"), false);
        const auto def = source.property_default_value("count");
        ASSERT_TRUE(def.has_value());
        EXPECT_EQ(std::any_cast<int>(*def), 7);
        EXPECT_EQ(source.property_default_binding_mode("missing"), std::nullopt);
    }

    TEST(getter_channel, binding_context_is_a_recognized_name)
    {
        mock_object source;
        auto context = std::make_shared<child_object>();
        source.set_binding_context(context);
        EXPECT_EQ(source.try_get_object("binding_context"), context);
        EXPECT_TRUE(source.has_property("binding_context"));
    }

    TEST(getter_channel, demote_value_to_binding_is_silent)
    {
        mock_object source;
        source.text.set("manual"); // manual_value_setter
        int changes = 0;
        const auto token =
            source.text.changed.connect([&changes](const std::string&, const std::string&) { ++changes; });
        source.demote_value_to_binding("text");
        EXPECT_EQ(changes, 0);
        EXPECT_EQ(source.text.get(), "manual");
        // the demoted value now sits at from_binding: a from_binding set replaces it
        source.text.set("bound", setter_specificity::from_binding);
        EXPECT_EQ(source.text.get(), "bound");
        source.text.changed.disconnect(token);
    }

    // ---- binding_expression basics (BindingExpressionTests.Ctor / ApplyNull) ----

    TEST(binding_expression, ctor_parses_and_exposes_path)
    {
        const binding_expression expression{"Foo.Bar"};
        EXPECT_EQ(expression.path(), "Foo.Bar");
        EXPECT_FALSE(expression.is_applied());
        EXPECT_THROW(binding_expression{"Foo[]"}, std::invalid_argument);
    }

    TEST(binding_expression, apply_with_null_source_applies_the_target_default)
    {
        // C# ApplyNull: applying over a null source must not throw; the target keeps its default.
        mock_object target;
        binding_expression expression{"Foo.Bar"};
        expression.apply(binding_source_node{}, target, "text", setter_specificity::from_binding, {});
        EXPECT_EQ(target.text.get(), "fallback-default");
        EXPECT_TRUE(expression.is_applied());
    }

    TEST(binding_expression, walks_a_chain_and_reresolves_on_intermediate_change)
    {
        // The heart of BindingExpression.cs: "child.text", re-resolved when the INTERMEDIATE hop is
        // swapped (the part subscription on the source fires and the chain re-walks).
        auto source = std::make_shared<mock_object>();
        auto first = std::make_shared<child_object>();
        first->text.set("one");
        source->child.set(first);

        mock_object target;
        binding_expression expression{"child.text"};
        binding_source_node node;
        node.boxed = source;
        node.object = source.get();
        node.alive = source;
        expression.apply(node, target, "text", setter_specificity::from_binding, {.mode = binding_mode::one_way});
        EXPECT_EQ(target.text.get(), "one");

        first->text.set("two"); // leaf change
        EXPECT_EQ(target.text.get(), "two");

        auto second = std::make_shared<child_object>();
        second->text.set("three");
        source->child.set(second); // intermediate change: the old hop is dropped, the new one walked
        EXPECT_EQ(target.text.get(), "three");

        first->text.set("stale"); // the old chain must be disconnected
        EXPECT_EQ(target.text.get(), "three");

        expression.unapply();
        second->text.set("after-unapply");
        EXPECT_EQ(target.text.get(), "three");
    }

    TEST(binding_expression, source_death_is_safe_after_disconnect)
    {
        // §8: the engine holds hops weakly; a source dying while applied must not dangle.
        mock_object target;
        binding_expression expression{"text"};
        {
            auto source = std::make_shared<mock_object>();
            source->text.set("alive");
            binding_source_node node;
            node.boxed = source;
            node.object = source.get();
            node.alive = source;
            expression.apply(node, target, "text", setter_specificity::from_binding, {.mode = binding_mode::one_way});
            EXPECT_EQ(target.text.get(), "alive");
        } // source destroyed while the expression is still applied
        expression.apply(false); // re-apply over a dead source: a guarded no-op
        EXPECT_EQ(target.text.get(), "alive");
        expression.unapply(); // guarded disconnect against the dead node
    }
} // namespace
