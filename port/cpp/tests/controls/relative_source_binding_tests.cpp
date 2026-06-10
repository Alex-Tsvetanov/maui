// Tests for maui::controls::relative_binding_source (W1-02). Ported from
// src/Controls/tests/Core.UnitTests/RelativeSourceBindingTests.cs over the REAL port controls
// (label / vertical_stack_layout / grid): Self, FindAncestor (+ live updates), TemplatedParent (the
// documented stub), and the big FindAncestor/FindAncestorBindingContext matrix across binding-context
// and parent changes. C#'s StyleId/BackgroundColor anchors become automation_id (the port's bindable
// string on every view); null-text asserts become empty-string (the port's std::string default).
#include "maui/controls/bindings/relative_binding_source.hpp"

#include <any>
#include <memory>
#include <string>

#include "maui/controls/bindings/binding.hpp"
#include "maui/controls/bindings/binding_diagnostics.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/property.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::binding;
    using maui::controls::grid;
    using maui::controls::label;
    using maui::controls::relative_binding_source;
    using maui::controls::relative_binding_source_mode;
    using maui::controls::vertical_stack_layout;
    using maui::core::bindable_object;
    using maui::core::bindable_property;
    using maui::core::binding_mode;
    using maui::core::property;

    const bindable_property<std::string>& name_prop()
    {
        static const bindable_property<std::string> descriptor{"name"};
        return descriptor;
    }
    struct person_view_model : bindable_object
    {
        property<std::string> name{*this, name_prop()};
    };

    [[nodiscard]] std::shared_ptr<person_view_model> make_person(std::string name)
    {
        auto person = std::make_shared<person_view_model>();
        person->name.set(std::move(name));
        return person;
    }

    [[nodiscard]] std::shared_ptr<binding> bind_to(std::string path, std::shared_ptr<relative_binding_source> source)
    {
        auto b = std::make_shared<binding>(std::move(path));
        b->set_source(std::move(source));
        return b;
    }

    TEST(relative_source_binding, modes_and_singletons)
    {
        EXPECT_EQ(relative_binding_source::self()->mode(), relative_binding_source_mode::self);
        EXPECT_EQ(relative_binding_source::self(), relative_binding_source::self()); // C# singleton
        EXPECT_EQ(relative_binding_source::templated_parent()->mode(), relative_binding_source_mode::templated_parent);
        const auto ancestor = relative_binding_source::find_ancestor<vertical_stack_layout>(2);
        EXPECT_EQ(ancestor->mode(), relative_binding_source_mode::find_ancestor);
        EXPECT_EQ(ancestor->ancestor_level(), 2);
    }

    TEST(relative_source_binding, relative_source_self_binding)
    {
        label target;
        target.set_automation_id("label1");
        target.set_binding("text", bind_to("automation_id", relative_binding_source::self()));
        EXPECT_EQ(target.text(), target.automation_id());

        target.set_automation_id("label2");
        EXPECT_EQ(target.text(), "label2"); // the self source is observed, too
    }

    TEST(relative_source_binding, relative_source_binding_find_ancestor)
    {
        vertical_stack_layout stack;
        stack.set_automation_id("stack1");
        label target;
        stack.add(target);

        target.set_binding("text",
                           bind_to("automation_id", relative_binding_source::find_ancestor<vertical_stack_layout>(1)));
        EXPECT_EQ(target.text(), "stack1");

        stack.set_automation_id("stack2");
        EXPECT_EQ(target.text(), "stack2");
    }

    TEST(relative_source_binding, templated_parent_is_a_documented_stub)
    {
        // No control templates in the port yet: the binding resolves no source (default/fallback)
        // and reports a diagnostic instead.
        int failures = 0;
        maui::controls::set_binding_failure_handler([&failures](const std::string&) { ++failures; });
        label target;
        target.set_text("untouched");
        auto b = bind_to("automation_id", relative_binding_source::templated_parent());
        b->set_fallback_value(std::any{std::string{"no-templated-parent"}});
        target.set_binding("text", b);
        EXPECT_EQ(target.text(), "no-templated-parent");
        EXPECT_GE(failures, 1);
        maui::controls::set_binding_failure_handler({});
    }

    TEST(relative_source_binding, relative_source_ancestor_type_binding)
    {
        // The C# RelativeSourceAncestorTypeBinding matrix: three nested stacks + a grid + three
        // labels binding text <- FindAncestorBindingContext(person, level N) and automation_id <-
        // FindAncestor(stack, level N), driven through binding-context and parent changes.
        grid host;
        vertical_stack_layout stack0;
        stack0.set_automation_id("red");
        vertical_stack_layout stack1;
        stack1.set_automation_id("green");
        vertical_stack_layout stack2;
        stack2.set_automation_id("blue");

        label label0;
        label label1;
        label label2;
        auto person0 = make_person("Person 0");
        auto person1 = make_person("Person 1");
        auto person2 = make_person("Person 2");

        stack2.add(stack1);
        stack1.add(stack0);
        stack0.add(host);

        label0.set_binding(
            "text", bind_to("name", relative_binding_source::find_ancestor_binding_context<person_view_model>(1)));
        label0.set_binding("automation_id",
                           bind_to("automation_id", relative_binding_source::find_ancestor<vertical_stack_layout>(1)));
        EXPECT_EQ(label0.text(), "");
        EXPECT_EQ(label0.automation_id(), "");

        label1.set_binding(
            "text", bind_to("name", relative_binding_source::find_ancestor_binding_context<person_view_model>(2)));
        label1.set_binding("automation_id",
                           bind_to("automation_id", relative_binding_source::find_ancestor<vertical_stack_layout>(2)));
        label2.set_binding(
            "text", bind_to("name", relative_binding_source::find_ancestor_binding_context<person_view_model>(3)));
        label2.set_binding("automation_id",
                           bind_to("automation_id", relative_binding_source::find_ancestor<vertical_stack_layout>(3)));

        host.add(label0);
        host.add(label1);
        host.add(label2);

        // ---- BindingContext changes ----
        // stack2 / stack1 / stack0 / host: no contexts yet.
        EXPECT_EQ(label0.automation_id(), stack0.automation_id());
        EXPECT_EQ(label1.automation_id(), stack1.automation_id());
        EXPECT_EQ(label2.automation_id(), stack2.automation_id());
        EXPECT_EQ(label0.text(), "");
        EXPECT_EQ(label1.text(), "");
        EXPECT_EQ(label2.text(), "");

        // stack2(person2) -> the same INHERITED context counts once for the level walk.
        stack2.set_binding_context(person2);
        EXPECT_EQ(label0.text(), person2->name.get());
        EXPECT_EQ(label1.text(), "");
        EXPECT_EQ(label2.text(), "");

        // stack2(person2) / stack1(person1)
        stack1.set_binding_context(person1);
        EXPECT_EQ(label0.text(), person1->name.get());
        EXPECT_EQ(label1.text(), person2->name.get());
        EXPECT_EQ(label2.text(), "");

        // stack2(person2) / stack1(person1) / stack0(person0)
        stack0.set_binding_context(person0);
        EXPECT_EQ(label0.text(), person0->name.get());
        EXPECT_EQ(label1.text(), person1->name.get());
        EXPECT_EQ(label2.text(), person2->name.get());
        EXPECT_EQ(label0.automation_id(), stack0.automation_id());
        EXPECT_EQ(label1.automation_id(), stack1.automation_id());
        EXPECT_EQ(label2.automation_id(), stack2.automation_id());

        // ---- Parent changes ----
        // Detach stack1 from stack2: label2's level-3 ancestor (and person2's context) vanish.
        stack2.clear();
        EXPECT_EQ(label0.text(), person0->name.get());
        EXPECT_EQ(label1.text(), person1->name.get());
        EXPECT_EQ(label2.text(), "");
        EXPECT_EQ(label0.automation_id(), stack0.automation_id());
        EXPECT_EQ(label1.automation_id(), stack1.automation_id());
        EXPECT_EQ(label2.automation_id(), "");

        stack1.clear();
        EXPECT_EQ(label0.text(), person0->name.get());
        EXPECT_EQ(label1.text(), "");
        EXPECT_EQ(label2.text(), "");
        EXPECT_EQ(label0.automation_id(), stack0.automation_id());
        EXPECT_EQ(label1.automation_id(), "");

        stack0.clear();
        EXPECT_EQ(label0.text(), "");
        EXPECT_EQ(label1.text(), "");
        EXPECT_EQ(label2.text(), "");
        EXPECT_EQ(label0.automation_id(), "");
        EXPECT_EQ(label1.automation_id(), "");
        EXPECT_EQ(label2.automation_id(), "");

        // Re-attach: the chain subscriptions resolve the ancestors again.
        stack0.add(host);
        stack1.add(stack0);
        EXPECT_EQ(label0.text(), person0->name.get());
        EXPECT_EQ(label0.automation_id(), stack0.automation_id());
        EXPECT_EQ(label1.text(), person1->name.get());
        EXPECT_EQ(label1.automation_id(), stack1.automation_id());
    }
} // namespace
