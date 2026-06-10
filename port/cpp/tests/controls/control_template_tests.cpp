// Tests for maui::controls::control_template / templated_view / templated_page / content_presenter /
// template_binding (W1-09), ported from src/Controls/tests/Core.UnitTests/ControlTemplateTests.cs,
// TemplatedViewUnitTests.cs (the code-first cases — the XAML-loader ones wait for the element-side
// name scope) and TemplatedPageUnitTests.cs.
//
// The C# suites lean on ContentView (TemplatedView + a Content property); content_view itself is a
// later unit, so a faithful test double (test_content_view) supplies exactly ContentView's template
// wiring: Content routed through template_utilities::on_content_changed, templated_content()
// returning it, the always-propagate SetChildInheritedBindingContext, and the
// OnBindingContextChanged / OnControlTemplateChanged content pushes. The C# string-path bindings map
// onto the typed-accessor template_binding factories (PROFILE §6 — no reflection).
#include "maui/controls/templates/control_template.hpp"

#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/element.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/content_presenter.hpp"
#include "maui/controls/templates/i_control_templated.hpp"
#include "maui/controls/templates/template_binding.hpp"
#include "maui/controls/templates/template_utilities.hpp"
#include "maui/controls/templates/templated_page.hpp"
#include "maui/controls/templates/templated_view.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/binding.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/property.hpp"
#include "maui/core/setter_specificity.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::content_presenter;
    using maui::controls::control_template;
    using maui::controls::element;
    using maui::controls::entry;
    using maui::controls::i_control_templated;
    using maui::controls::label;
    using maui::controls::template_binding;
    using maui::controls::template_utilities;
    using maui::controls::templated_page;
    using maui::controls::templated_view;
    using maui::controls::vertical_stack_layout;
    using maui::core::bindable_object;
    using maui::core::bindable_property;
    using maui::core::property;

    // The TestView.NameProperty / TestPage.NameProperty stand-in.
    const bindable_property<std::string>& name_prop()
    {
        static const bindable_property<std::string> descriptor{"name"};
        return descriptor;
    }

    // ---- the ContentView test double (see file comment) ----
    class test_content_view : public templated_view
    {
    public:
        void set_content(std::shared_ptr<element> value)
        {
            if (content_ == value)
            {
                return;
            }
            content_ = std::move(value);
            template_utilities::on_content_changed(*this, content_);
        }
        [[nodiscard]] std::shared_ptr<element> templated_content() const override
        {
            return content_;
        }

    protected:
        // ContentView.SetChildInheritedBindingContext: ALWAYS propagate (unlike TemplatedView).
        void set_child_inherited_binding_context(
            element& child, const maui::core::bindable_object::binding_context_box& context) override
        {
            child.set_inherited_binding_context(context);
        }
        // ContentView.OnBindingContextChanged / OnControlTemplateChanged: push the context into Content.
        void on_binding_context_changed() override
        {
            templated_view::on_binding_context_changed();
            if (content_ != nullptr)
            {
                content_->set_inherited_binding_context(raw_binding_context());
            }
        }

    public:
        void on_control_template_changed(maui::controls::control_template* old_value,
                                         maui::controls::control_template* new_value) override
        {
            templated_view::on_control_template_changed(old_value, new_value);
            if (content_ != nullptr)
            {
                content_->set_inherited_binding_context(raw_binding_context());
            }
        }

    private:
        std::shared_ptr<element> content_; // ContentView.Content (owned alongside the test)
    };

    // ---- ControlTemplateTests.TestView (a ContentView with a Name property + the ctor template) ----
    class test_view : public test_content_view
    {
    public:
        test_view(); // sets ControlTemplate = ContentControl (defined after content_control below)

        [[nodiscard]] std::string_view name() const
        {
            return name_.get();
        }
        void set_name(std::string value)
        {
            name_.set(std::move(value));
        }

    private:
        property<std::string> name_{*this, name_prop()};
    };

    // ControlTemplateTests.ContentControl: a StackLayout holding a label bound to
    // TemplatedParent.Name plus a ContentPresenter.
    class content_control : public vertical_stack_layout
    {
    public:
        content_control()
        {
            label_ = std::make_shared<label>();
            // C#: label.SetBinding(TextProperty, new Binding("Name", source: TemplatedParent)).
            label_->set_template_binding(template_binding::one_way<std::string, test_view>(
                label::text_property(), name_prop(),
                [](const test_view& parent) { return std::string{parent.name()}; }));
            presenter_ = std::make_shared<content_presenter>();
            add(*label_);
            add(*presenter_);
        }

        [[nodiscard]] const std::shared_ptr<label>& label_child() const
        {
            return label_;
        }
        [[nodiscard]] const std::shared_ptr<content_presenter>& presenter_child() const
        {
            return presenter_;
        }

    private:
        std::shared_ptr<label> label_;
        std::shared_ptr<content_presenter> presenter_;
    };

    test_view::test_view()
    {
        set_control_template(control_template::of<content_control>());
    }

    // ControlTemplateTests.PresenterWrapper: a ContentView whose content IS a ContentPresenter.
    class presenter_wrapper : public test_content_view
    {
    public:
        presenter_wrapper()
        {
            set_content(std::make_shared<content_presenter>());
        }
    };

    // ---- ControlTemplateTests ----

    TEST(control_template, null_constructor)
    {
        EXPECT_THROW(control_template{control_template::loader{}}, std::invalid_argument);
    }

    TEST(control_template, must_create_a_view)
    {
        // TemplateUtilities.OnControlTemplateChanged: NotSupportedException when the template content
        // is not derived from View (a bare bindable_object here).
        templated_view sut;
        EXPECT_THROW(sut.set_control_template(
                         std::make_shared<control_template>([] { return std::make_shared<bindable_object>(); })),
                     std::runtime_error);
    }

    TEST(control_template, resetting_control_template_nulls_presenter_content)
    {
        test_view sut;
        sut.set_control_template(control_template::of<presenter_wrapper>());

        const auto content_label = std::make_shared<label>();
        sut.set_content(content_label);

        ASSERT_EQ(sut.internal_children().size(), 1U);
        auto* wrapper = dynamic_cast<presenter_wrapper*>(sut.internal_children()[0].get());
        ASSERT_NE(wrapper, nullptr);
        ASSERT_EQ(wrapper->internal_children().size(), 1U);
        // Keep shared ownership so the original presenter outlives the template swap (the C# test's
        // GC-rooted local reference).
        const auto original_presenter = std::dynamic_pointer_cast<content_presenter>(wrapper->internal_children()[0]);
        ASSERT_NE(original_presenter, nullptr);

        EXPECT_EQ(original_presenter->content_element(), content_label.get());

        sut.set_control_template(control_template::of<presenter_wrapper>());

        EXPECT_EQ(original_presenter->content_element(), nullptr);
        // ...and the NEW presenter picked the content up.
        auto* new_wrapper = dynamic_cast<presenter_wrapper*>(sut.internal_children()[0].get());
        ASSERT_NE(new_wrapper, nullptr);
        const auto new_presenter = std::dynamic_pointer_cast<content_presenter>(new_wrapper->internal_children()[0]);
        ASSERT_NE(new_presenter, nullptr);
        EXPECT_EQ(new_presenter->content_element(), content_label.get());
    }

    TEST(control_template, nested_template_bindings)
    {
        test_view sut;
        ASSERT_EQ(sut.internal_children().size(), 1U);
        auto* root = dynamic_cast<content_control*>(sut.internal_children()[0].get());
        ASSERT_NE(root, nullptr);
        const auto& bound_label = root->label_child();

        EXPECT_EQ(bound_label->text(), ""); // C# Assert.Null(label.Text) — the unset default

        sut.set_name("Bar");
        EXPECT_EQ(bound_label->text(), "Bar");
    }

    TEST(control_template, leaving_the_template_scope_clears_the_bound_value)
    {
        // Port-mechanics pin: a one-way template binding un-applies (from_binding value cleared) when
        // its element's subtree is detached from the templated parent (C# unapplies on Unapply).
        test_view sut;
        const auto old_root = sut.internal_children()[0]; // keep the old template subtree alive
        auto* root = dynamic_cast<content_control*>(old_root.get());
        ASSERT_NE(root, nullptr);
        sut.set_name("Bar");
        EXPECT_EQ(root->label_child()->text(), "Bar");

        sut.set_control_template(control_template::of<presenter_wrapper>());
        EXPECT_EQ(root->label_child()->text(), ""); // out of scope -> back to the default
    }

    TEST(control_template, parent_control_template_does_not_clear_child_template)
    {
        test_view parent_view;
        const auto child_view = std::make_shared<test_view>();

        parent_view.set_content(child_view);
        const auto child_content = std::make_shared<label>();
        child_view->set_content(child_content);

        auto* child_root = dynamic_cast<content_control*>(child_view->internal_children()[0].get());
        ASSERT_NE(child_root, nullptr);
        const auto& child_presenter = child_root->presenter_child();
        ASSERT_EQ(child_presenter->content_element(), child_content.get());

        parent_view.set_control_template(control_template::of<content_control>());
        EXPECT_NE(child_presenter->content_element(), nullptr); // the child scope was not walked into
    }

    // ---- ControlTemplateTests.DoubleTwoWayBindingWorks ----

    // TestPage : ContentPage — the page root with a Name property; the page's own data binding is the
    // M5 typed bind() (the C# SetBinding(NameProperty, "Name") to the view model).
    class test_page : public templated_page
    {
    public:
        property<std::string> name{*this, name_prop()}; // public: the test binds it directly
    };

    // The view model (C# ViewModel : INotifyPropertyChanged).
    struct view_model : bindable_object
    {
        property<std::string> name{*this, name_prop()};
    };

    // TestContent : ContentView { Content = Entry bound TwoWay to TemplatedParent.Name }.
    class test_content : public test_content_view
    {
    public:
        test_content()
        {
            entry_ = std::make_shared<entry>();
            entry_->set_template_binding(template_binding::two_way<std::string, entry, test_page>(
                entry::text_property(), name_prop(),
                [](const test_page& parent) { return std::string{parent.name.get()}; },
                [](const entry& target) { return std::string{target.text()}; }));
            set_content(entry_);
        }
        [[nodiscard]] const std::shared_ptr<entry>& entry_child() const
        {
            return entry_;
        }

    private:
        std::shared_ptr<entry> entry_;
    };

    TEST(control_template, double_two_way_binding_works)
    {
        test_page page;
        view_model model;
        model.name.set("Jason");

        page.set_control_template(control_template::of<test_content>());
        // page.SetBinding(NameProperty, "Name") — OneWay against the view model.
        auto handle = maui::core::bind(page.name, model.name, maui::core::binding_mode::one_way);
        EXPECT_EQ(page.name.get(), "Jason");

        ASSERT_EQ(page.internal_children().size(), 1U);
        auto* root = dynamic_cast<test_content*>(page.internal_children()[0].get());
        ASSERT_NE(root, nullptr);
        const auto& bound_entry = root->entry_child();
        EXPECT_EQ(bound_entry->text(), "Jason"); // pushed through the two-way template binding

        // ((IElementController)entry).SetValueFromRenderer(TextProperty, "Bar") — a from_handler set.
        bound_entry->apply_setter("text", std::any{std::string{"Bar"}}, maui::core::setter_specificity::from_handler);
        EXPECT_EQ(page.name.get(), "Bar"); // written back to the templated parent (from_handler)

        model.name.set("Raz"); // the page's own binding overrides the handler value...
        EXPECT_EQ(page.name.get(), "Raz");
        EXPECT_EQ(bound_entry->text(), "Raz"); // ...and flows on into the template
    }

    // ---- TemplatedViewUnitTests ----

    struct plain_element : element // the VisualElement stand-in for AddLogicalChild
    {
    };
    struct expected_view : label // a minimal View the template creates
    {
    };

    TEST(templated_view_tests, internal_children_replaced_when_control_template_changes)
    {
        templated_view sut;
        auto& templated = static_cast<i_control_templated&>(sut);
        templated.add_logical_child(std::make_shared<plain_element>());
        templated.add_logical_child(std::make_shared<plain_element>());
        templated.add_logical_child(std::make_shared<plain_element>());

        sut.set_control_template(control_template::of<expected_view>());

        ASSERT_EQ(templated.internal_children().size(), 1U);
        EXPECT_NE(dynamic_cast<expected_view*>(templated.internal_children()[0].get()), nullptr);
    }

    TEST(templated_view_tests, should_have_template_root_set)
    {
        templated_view sut;
        auto& templated = static_cast<i_control_templated&>(sut);
        EXPECT_EQ(templated.template_root(), nullptr);

        sut.set_control_template(control_template::of<expected_view>());

        ASSERT_EQ(templated.internal_children().size(), 1U);
        EXPECT_EQ(templated.template_root(), templated.internal_children()[0].get());
        EXPECT_TRUE(templated.internal_children()[0]->is_template_root());
        // i_content_view::content() carries C# PresentedContent — the template root as a view.
        EXPECT_EQ(sut.content(), dynamic_cast<maui::core::i_view*>(templated.template_root()));
    }

    // OnApplyTemplate (the code-first slice of OnTemplatedViewApplyTemplateShouldBeCalled —
    // GetTemplateChild is deferred with the element-side name scope).
    class observing_templated_view : public templated_view
    {
    public:
        bool was_on_apply_template_called = false;

        void on_apply_template() override
        {
            was_on_apply_template_called = true;
            root_at_apply = template_root();
        }
        element* root_at_apply = nullptr;
    };

    TEST(templated_view_tests, on_apply_template_called_after_root_is_set)
    {
        observing_templated_view sut;
        sut.set_control_template(control_template::of<expected_view>());
        EXPECT_TRUE(sut.was_on_apply_template_called);
        EXPECT_EQ(sut.root_at_apply, sut.template_root()); // C# order: TemplateRoot before OnApplyTemplate
        EXPECT_NE(sut.root_at_apply, nullptr);
    }

    TEST(templated_view_tests, bindings_applied_on_template_change)
    {
        // TemplatedViewUnitTests.BindingsShouldBeAppliedOnTemplateChange. MyTemplate is a StackLayout
        // holding a ContentPresenter; the label's `Binding(".")` is the context self-binding,
        // hand-wired here (live-element runtime bindings belong to the runtime-binding unit).
        class my_template : public vertical_stack_layout
        {
        public:
            my_template()
            {
                presenter_ = std::make_shared<content_presenter>();
                add(*presenter_);
            }

        private:
            std::shared_ptr<content_presenter> presenter_;
        };

        const auto template0 = control_template::of<my_template>();
        const auto template1 = control_template::of<my_template>();

        const auto bound_label = std::make_shared<label>();
        const auto apply_context = [](label& target) {
            if (const auto context = target.binding_context<std::string>())
            {
                target.apply_setter("text", std::any{*context}, maui::core::setter_specificity::from_binding);
            }
            else
            {
                target.clear_setter("text", maui::core::setter_specificity::from_binding);
            }
        };
        bound_label->binding_context_changed.connect(
            [target = bound_label.get(), apply_context] { apply_context(*target); });

        test_content_view sut;
        sut.set_control_template(template0);
        sut.set_content(bound_label);
        sut.set_binding_context(std::make_shared<std::string>("Foo"));

        EXPECT_EQ(bound_label->text(), "Foo");
        sut.set_control_template(template1);
        EXPECT_EQ(bound_label->text(), "Foo");
    }

    // ---- TemplatedPageUnitTests ----

    TEST(templated_page_tests, internal_children_replaced_when_control_template_changes)
    {
        templated_page sut;
        auto& templated = static_cast<i_control_templated&>(sut);
        templated.add_logical_child(std::make_shared<plain_element>());
        templated.add_logical_child(std::make_shared<plain_element>());
        templated.add_logical_child(std::make_shared<plain_element>());

        sut.set_control_template(control_template::of<expected_view>());

        ASSERT_EQ(templated.internal_children().size(), 1U);
        EXPECT_NE(dynamic_cast<expected_view*>(templated.internal_children()[0].get()), nullptr);
    }

    TEST(templated_page_tests, add_logical_child_is_idempotent)
    {
        // TemplatedPage.AddLogicalChild: only add when absent.
        templated_page sut;
        auto& templated = static_cast<i_control_templated&>(sut);
        const auto child = std::make_shared<plain_element>();
        templated.add_logical_child(child);
        templated.add_logical_child(child);
        EXPECT_EQ(templated.internal_children().size(), 1U);
    }

    TEST(control_template, externally_owned_template_child_survives_the_templated_parent)
    {
        // Deterministic-teardown pin (PROFILE §8): destroying the templated parent detaches its
        // template subtree, so a child kept alive by an external owner re-resolves its template
        // binding to "out of scope" (value cleared, subscriptions dropped) instead of keeping a
        // dangling connection into the destroyed parent. ASan validates the no-dangling claim.
        std::shared_ptr<element> root_keepalive;
        std::shared_ptr<label> label_keepalive;
        {
            test_view sut;
            auto* root = dynamic_cast<content_control*>(sut.internal_children()[0].get());
            ASSERT_NE(root, nullptr);
            root_keepalive = sut.internal_children()[0];
            label_keepalive = root->label_child();
            sut.set_name("Bar");
            EXPECT_EQ(label_keepalive->text(), "Bar");
        }
        EXPECT_EQ(label_keepalive->text(), ""); // detached at parent teardown -> binding un-applied
    }

    // ---- the BindingContext suppression (TemplatedView.SetChildInheritedBindingContext) ----

    TEST(templated_view_tests, template_subtree_does_not_inherit_the_binding_context)
    {
        templated_view sut;
        sut.set_control_template(control_template::of<expected_view>());
        sut.set_binding_context(std::make_shared<std::string>("data"));
        // While a ControlTemplate is set, the template root binds to the TEMPLATED PARENT, not to the
        // data context — no inheritance (C# TemplatedView.SetChildInheritedBindingContext guard).
        EXPECT_FALSE(sut.internal_children()[0]->has_binding_context());
    }
} // namespace
