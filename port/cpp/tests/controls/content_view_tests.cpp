// Tests for the content_view control — ported from ContentViewUnitTests.cs (src/Controls/tests/
// Core.UnitTests): the single-content surface (constructor defaults, set/replace child + logical
// parenting), the measure-includes-padding contract (TestFrameLayout's measure half — the arrange
// half keeps the port's established fill semantics, as content_page does), the ControlTemplate
// interplay (NullTemplateDirectlyHosts / TemplateInflates / PacksContent), the BindingContext rules
// (DoesInheritBindingContextToTemplate / ContentDoesGetBindingContext /
// NonTemplatedContentInheritsBindingContext), and the handler seam — content_view resolves the SAME
// content_page_handler C# resolves ContentViewHandler for, hosting PresentedContent (the template
// root when templated, else the content).
//
// NOT ported (documented deviations, both inherited from the merged template machinery):
//   - ContentParentIsNotInsideTemplate: the port's content_presenter attaches the presented content
//     as ITS logical child (content_presenter.cpp collapses C#'s ParentOverride redirection into the
//     push model), so a templated content's logical parent is the presenter, not the content_view.
//   - The LayoutOptions alignment tests (LayoutVertically*/Horizontally*): the port's ArrangeContent
//     fills the padded bounds (no LayoutOptions surface yet — same scope as content_page).
#include "maui/controls/content_view.hpp"

#include <memory>
#include <string>

#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/content_presenter.hpp"
#include "maui/controls/templates/control_template.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_safe_area_view.hpp"
#include "maui/core/safe_area_edges.hpp"
#include "maui/core/safe_area_regions.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "tests/layouts/layout_test_helpers.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::content_presenter;
    using maui::controls::content_view;
    using maui::controls::control_template;
    using maui::controls::label;
    using maui::controls::vertical_stack_layout;
    using maui::core::content_page_handler;
    using maui::core::i_element_handler;
    using maui::core::thickness;
    using maui::graphics::rect;
    using maui::graphics::size;
    using maui::layouts::testing::mock_view;

    // ContentViewUnitTests.SimpleTemplate: a StackLayout holding a Label and a ContentPresenter.
    class simple_template : public vertical_stack_layout
    {
    public:
        simple_template()
        {
            heading_ = std::make_shared<label>();
            presenter_ = std::make_shared<content_presenter>();
            add(*heading_);
            add(*presenter_);
        }

        [[nodiscard]] const std::shared_ptr<content_presenter>& presenter() const
        {
            return presenter_;
        }

    private:
        std::shared_ptr<label> heading_;
        std::shared_ptr<content_presenter> presenter_;
    };

    // ---- the control in isolation ----

    TEST(content_view, defaults_have_no_content_and_zero_padding) // C# TestConstructor
    {
        const content_view view;
        EXPECT_EQ(view.content_child(), nullptr);
        EXPECT_EQ(view.content(), nullptr); // PresentedContent: no template, no content
        EXPECT_EQ(view.padding(), thickness());
    }

    TEST(content_view, set_content_parents_the_child) // C# TestSetChild
    {
        content_view view;
        auto child = std::make_shared<mock_view>();

        view.set_content(child);

        EXPECT_EQ(view.content_child(), child);
        EXPECT_EQ(child->logical_parent(), &view);

        // Re-setting the same content is a no-op (C# asserts no second ChildAdded).
        view.set_content(child);
        EXPECT_EQ(view.content_child(), child);
        EXPECT_EQ(child->logical_parent(), &view);
    }

    TEST(content_view, replacing_content_unparents_the_old_child) // C# TestReplaceChild
    {
        content_view view;
        auto first = std::make_shared<mock_view>();
        auto second = std::make_shared<mock_view>();

        view.set_content(first);
        view.set_content(second);

        EXPECT_EQ(first->logical_parent(), nullptr);
        EXPECT_EQ(second->logical_parent(), &view);
        EXPECT_EQ(view.content_child(), second);
    }

    TEST(content_view, measure_includes_padding) // C# TestFrameLayout (the measure half)
    {
        content_view view;
        view.set_padding(thickness(10));
        auto child = std::make_shared<mock_view>();
        child->configure({100, 200});
        view.set_content(child);

        // content 100x200 + padding {10} on all sides -> 120x220.
        const size measured = view.measure(1000, 1000);
        EXPECT_EQ(measured.width, 120.0);
        EXPECT_EQ(measured.height, 220.0);
    }

    TEST(content_view, arrange_places_content_within_padding)
    {
        content_view view;
        view.set_padding(thickness(10));
        auto child = std::make_shared<mock_view>();
        child->configure({100, 200});
        view.set_content(child);

        view.measure(1000, 1000);
        view.arrange(rect(0, 0, 120, 220));

        EXPECT_EQ(child->last_arrange, rect(10, 10, 100, 200));
    }

    // ---- ControlTemplate interplay ----

    TEST(content_view, null_template_directly_hosts_the_content) // C# NullTemplateDirectlyHosts
    {
        content_view view;
        auto child = std::make_shared<mock_view>();

        view.set_content(child);

        ASSERT_EQ(view.internal_children().size(), 1U);
        EXPECT_EQ(view.internal_children()[0], child);
    }

    TEST(content_view, template_inflates) // C# TemplateInflates
    {
        content_view view;

        view.set_control_template(control_template::of<simple_template>());

        ASSERT_EQ(view.internal_children().size(), 1U);
        EXPECT_NE(dynamic_cast<simple_template*>(view.internal_children()[0].get()), nullptr);
    }

    TEST(content_view, template_packs_the_content) // C# PacksContent
    {
        content_view view;
        auto child = std::make_shared<mock_view>();

        view.set_control_template(control_template::of<simple_template>());
        view.set_content(child);

        // The template root stays the one logical child...
        ASSERT_EQ(view.internal_children().size(), 1U);
        auto* root = dynamic_cast<simple_template*>(view.internal_children()[0].get());
        ASSERT_NE(root, nullptr);
        // ...and the content is presented through the template's ContentPresenter (the C#
        // Descendants() containment, walked directly to the presenter here).
        EXPECT_EQ(root->presenter()->content_element(), child.get());
    }

    TEST(content_view, template_root_inherits_the_binding_context) // C# DoesInheritBindingContextToTemplate
    {
        content_view view;
        view.set_control_template(control_template::of<simple_template>());
        view.set_content(std::make_shared<mock_view>());

        const auto context = std::make_shared<std::string>("Test");
        view.set_binding_context(context);

        ASSERT_EQ(view.internal_children().size(), 1U);
        EXPECT_EQ(view.internal_children()[0]->binding_context<std::string>(), context);
    }

    TEST(content_view, templated_content_gets_the_outer_binding_context) // C# ContentDoesGetBindingContext
    {
        content_view view;
        auto child = std::make_shared<mock_view>();

        view.set_control_template(control_template::of<simple_template>());
        view.set_content(child);

        const auto context = std::make_shared<std::string>("Test");
        view.set_binding_context(context);

        EXPECT_EQ(child->binding_context<std::string>(), context);
    }

    TEST(content_view, untemplated_content_inherits_the_binding_context) // C# NonTemplatedContentInheritsBindingContext
    {
        content_view view;
        auto child = std::make_shared<mock_view>();

        view.set_content(child);
        const auto context = std::make_shared<std::string>("Foo");
        view.set_binding_context(context);

        EXPECT_EQ(child->binding_context<std::string>(), context);
    }

    // A template whose ContentPresenter is nested TWO layouts deep (root hstack -> column vstack ->
    // presenter), mirroring the templated_view gallery page's CardViewCompressed template. Catches the
    // depth-2 regression the depth-1 simple_template can't (the presenter pull + measure/arrange must
    // recurse through the intervening layout, not just a direct template-root child).
    class nested_template : public maui::controls::horizontal_stack_layout
    {
    public:
        nested_template()
        {
            icon_ = std::make_shared<mock_view>();
            icon_->configure({100, 100});
            heading_ = std::make_shared<mock_view>();
            heading_->configure({80, 20});
            presenter_ = std::make_shared<content_presenter>();
            column_.add(*heading_);
            column_.add(*presenter_);
            add(*icon_);
            add(column_);
        }

        [[nodiscard]] content_presenter* presenter() const
        {
            return presenter_.get();
        }

    private:
        std::shared_ptr<mock_view> icon_;
        std::shared_ptr<mock_view> heading_;
        maui::controls::vertical_stack_layout column_;
        std::shared_ptr<content_presenter> presenter_;
    };

    TEST(content_view, nested_presenter_packs_the_content) // depth-2 analog of PacksContent
    {
        content_view view;
        auto body = std::make_shared<mock_view>();
        body->configure({120, 40});

        view.set_control_template(control_template::of<nested_template>());
        view.set_content(body);

        auto* root = dynamic_cast<nested_template*>(view.internal_children()[0].get());
        ASSERT_NE(root, nullptr);
        // The deeply-nested presenter still resolves the templated parent and packs the developer
        // content (find_templated_parent walks past the intervening column + hstack to the card).
        EXPECT_EQ(root->presenter()->content_element(), body.get());
    }

    TEST(content_view, nested_presenter_content_contributes_to_measure_and_arrange)
    {
        content_view view;
        auto body = std::make_shared<mock_view>();
        body->configure({120, 40});

        view.set_control_template(control_template::of<nested_template>());
        view.set_content(body);

        auto* root = dynamic_cast<nested_template*>(view.internal_children()[0].get());
        ASSERT_NE(root, nullptr);

        // The card measures through the template root: the column (heading 20 + spacing? + body 40) sits
        // beside the 100-tall icon, so the row is at least the 100-tall icon high and the body was
        // measured (its content is reachable through the presenter).
        const size measured = view.measure(1000, 1000);
        EXPECT_GE(measured.height, 100.0);
        EXPECT_GT(body->measure_count, 0) << "the nested presenter must measure its packed content";

        view.arrange(rect(0, 0, measured.width, measured.height));
        // The body is hosted as a SUBVIEW of the presenter's native host, so it must be arranged
        // HOST-RELATIVE (a small offset from the presenter's origin), NOT at the body's absolute page
        // position — otherwise it double-offsets off-screen (the content_presenter::arrange fix).
        EXPECT_LT(body->last_arrange.x, 50.0)
            << "nested presenter content must arrange host-relative, not at its absolute page x";
        EXPECT_LT(body->last_arrange.y, 50.0)
            << "nested presenter content must arrange host-relative, not at its absolute page y";
    }

    // ---- the handler seam: the same content_page_handler hosts PresentedContent ----

    TEST(content_view_seam, handler_resolved_from_default_registry_hosts_the_content)
    {
        // content_view -> content_page_handler is self-registered (C#: ContentView -> ContentViewHandler).
        const std::shared_ptr<i_element_handler> handler =
            maui::core::default_handler_registry().create_handler<content_view>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<content_page_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        content_view view;
        auto child = std::make_shared<mock_view>();
        view.set_content(child);
        view.set_handler(handler);
        EXPECT_EQ(resolved->typed_platform_view()->hosted_content, child.get());
    }

    TEST(content_view_seam, templated_view_hosts_the_template_root_and_content_changes_rehost)
    {
        content_view view;
        auto handler = std::make_shared<content_page_handler>();
        view.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->hosted_content, nullptr);

        auto child = std::make_shared<mock_view>();
        view.set_content(child); // untemplated: the content IS the presented content
        EXPECT_EQ(platform->hosted_content, child.get());

        view.set_control_template(control_template::of<simple_template>());
        // The template swap re-hosts on the next content change (the C# mapper's Content entry runs on
        // the control-template property change too; the port funnels through set_content).
        auto replacement = std::make_shared<mock_view>();
        view.set_content(replacement);
        EXPECT_EQ(platform->hosted_content, view.content()); // the template root, not the content
        EXPECT_NE(platform->hosted_content, static_cast<maui::core::i_view*>(replacement.get()));

        view.set_content(nullptr);
        EXPECT_EQ(platform->hosted_content, view.content()); // still the template root
    }

    // ---- U-SA: SafeAreaEdges ----

    TEST(content_view_safe_area, safe_area_edges_defaults_to_none)
    {
        const maui::controls::content_view cv;
        EXPECT_EQ(cv.safe_area_edges(), maui::core::safe_area_edges::none());
    }

    TEST(content_view_safe_area, safe_area_edges_is_settable)
    {
        maui::controls::content_view cv;
        cv.set_safe_area_edges(maui::core::safe_area_edges::all());
        EXPECT_EQ(cv.safe_area_edges(), maui::core::safe_area_edges::all());
    }

    TEST(content_view_safe_area, get_regions_for_edge_translates_default_to_none)
    {
        maui::controls::content_view cv;
        cv.set_safe_area_edges(maui::core::safe_area_edges::default_edges());
        const maui::core::i_safe_area_view2& cv2 = cv;
        EXPECT_EQ(cv2.get_safe_area_regions_for_edge(0), maui::core::safe_area_regions::none);
        cv.set_safe_area_edges(maui::core::safe_area_edges::all());
        EXPECT_EQ(cv2.get_safe_area_regions_for_edge(0), maui::core::safe_area_regions::all);
    }
} // namespace
