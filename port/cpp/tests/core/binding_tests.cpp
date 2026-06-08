// Tests for maui::core::bind — the typed-accessor data binding (M5a). A binding connects a target
// property<T> to a source property<U>, observing the source's `.changed` event and pushing at
// from_binding specificity. Two mock_objects stand in for a view and a view-model.
#include "maui/core/binding.hpp"

#include <string>

#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/property.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::core::bind;
    using maui::core::bindable_object;
    using maui::core::bindable_property;
    using maui::core::binding_mode;
    using maui::core::property;

    const bindable_property<std::string>& text_prop()
    {
        static const bindable_property<std::string> descriptor{"text"};
        return descriptor;
    }
    const bindable_property<std::string>& name_prop()
    {
        static const bindable_property<std::string> descriptor{"name"};
        return descriptor;
    }
    const bindable_property<int>& count_prop()
    {
        static const bindable_property<int> descriptor{"count", 0};
        return descriptor;
    }
    // A property whose DefaultBindingMode is TwoWay (like a user-editable control value).
    const bindable_property<std::string>& two_way_prop()
    {
        static const bindable_property<std::string> descriptor{
            "two_way", std::string{}, {.default_binding_mode = binding_mode::two_way}};
        return descriptor;
    }

    struct mock_object : bindable_object
    {
        property<std::string> text{*this, text_prop()};
        property<std::string> name{*this, name_prop()};
        property<int> count{*this, count_prop()};
        property<std::string> two_way{*this, two_way_prop()};
    };

    TEST(binding, one_way_propagates_source_to_target)
    {
        mock_object view;
        mock_object view_model;
        view_model.name.set("Ada");

        auto handle = bind(view.text, view_model.name, binding_mode::one_way);
        EXPECT_EQ(view.text.get(), "Ada"); // initial push at bind time

        view_model.name.set("Lovelace");
        EXPECT_EQ(view.text.get(), "Lovelace"); // source change propagates to target

        view.text.set("typed-in-view");
        EXPECT_EQ(view_model.name.get(), "Lovelace"); // one_way: target change does NOT flow back
    }

    TEST(binding, one_time_pushes_once_then_ignores_changes)
    {
        mock_object view;
        mock_object view_model;
        view_model.name.set("once");

        auto handle = bind(view.text, view_model.name, binding_mode::one_time);
        EXPECT_EQ(view.text.get(), "once");

        view_model.name.set("changed");
        EXPECT_EQ(view.text.get(), "once"); // no subscription — stays at the one-time value
    }

    TEST(binding, one_way_to_source_pushes_target_to_source)
    {
        mock_object view;
        mock_object view_model;
        view.text.set("from-view");

        auto handle = bind(view.text, view_model.name, binding_mode::one_way_to_source);
        EXPECT_EQ(view_model.name.get(), "from-view"); // initial push target -> source

        view.text.set("changed");
        EXPECT_EQ(view_model.name.get(), "changed");

        view_model.name.set("source-change");
        EXPECT_EQ(view.text.get(), "changed"); // source change does NOT flow to target
    }

    TEST(binding, two_way_flows_both_directions_without_looping)
    {
        mock_object view;
        mock_object view_model;
        view_model.name.set("init");

        auto handle = bind(view.text, view_model.name, binding_mode::two_way);
        EXPECT_EQ(view.text.get(), "init");

        view_model.name.set("from-source");
        EXPECT_EQ(view.text.get(), "from-source");

        view.text.set("from-target");
        EXPECT_EQ(view_model.name.get(), "from-target");
        // The test completing (no hang) proves the re-entrancy guard breaks the feedback loop.
    }

    TEST(binding, converter_applies_in_both_directions)
    {
        mock_object view;
        mock_object view_model;
        view_model.count.set(7);

        auto handle = bind(
            view.text, view_model.count, binding_mode::two_way,
            [](const int& number) { return std::to_string(number); },
            [](const std::string& text) { return text.empty() ? 0 : std::stoi(text); });
        EXPECT_EQ(view.text.get(), "7");

        view_model.count.set(42);
        EXPECT_EQ(view.text.get(), "42"); // convert

        view.text.set("100");
        EXPECT_EQ(view_model.count.get(), 100); // convert_back
    }

    TEST(binding, manual_set_overrides_binding_then_clear_restores_bound_value)
    {
        mock_object view;
        mock_object view_model;
        view_model.name.set("bound");

        auto handle = bind(view.text, view_model.name, binding_mode::one_way);
        EXPECT_EQ(view.text.get(), "bound");

        view.text.set("manual"); // manual_value_setter outranks from_binding
        EXPECT_EQ(view.text.get(), "manual");

        view_model.name.set("bound-2"); // the binding updates its (lower) from_binding slot silently
        EXPECT_EQ(view.text.get(), "manual");

        view.text.clear(); // clearing the manual value restores the bound value beneath it
        EXPECT_EQ(view.text.get(), "bound-2");
    }

    TEST(binding, default_mode_resolves_to_the_descriptor_default)
    {
        mock_object view;
        mock_object view_model;
        view_model.two_way.set("a");

        // No explicit mode -> the target's default_binding_mode (TwoWay for two_way_prop).
        auto handle = bind(view.two_way, view_model.two_way);
        EXPECT_EQ(view.two_way.get(), "a");

        view.two_way.set("b");
        EXPECT_EQ(view_model.two_way.get(), "b"); // proves the resolved mode is TwoWay
    }

    TEST(binding, reset_tears_down_the_binding)
    {
        mock_object view;
        mock_object view_model;
        view_model.name.set("x");

        auto handle = bind(view.text, view_model.name, binding_mode::one_way);
        EXPECT_EQ(view.text.get(), "x");
        EXPECT_TRUE(handle.active());

        handle.reset();
        EXPECT_FALSE(handle.active());

        view_model.name.set("y");
        EXPECT_EQ(view.text.get(), "x"); // no longer subscribed
    }
} // namespace
