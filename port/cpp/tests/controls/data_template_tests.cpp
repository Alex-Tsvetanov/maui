// Tests for maui::controls::data_template / data_template_selector (W1-09), ported from
// src/Controls/tests/Core.UnitTests/DataTemplateTests.cs and DataTemplateSelectorTests.cs (incl. the
// DataTemplateRecycleTests, whose ListView+RecycleElementAndDataTemplate container maps onto the
// i_template_recycling_container seam). A mock_bindable with a text property stands in for C#'s
// MockBindable; the cell types collapse into two distinct mock element types (the assertions are
// about template identity/created type, not about cells).
#include "maui/controls/templates/data_template.hpp"

#include <any>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "maui/controls/element.hpp"
#include "maui/controls/templates/data_template_selector.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/property.hpp"
#include "maui/core/type_tag.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::data_template;
    using maui::controls::data_template_selector;
    using maui::controls::i_template_recycling_container;
    using maui::core::bindable_object;
    using maui::core::bindable_property;
    using maui::core::property;
    using maui::core::type_tag;

    const bindable_property<std::string>& text_prop()
    {
        static const bindable_property<std::string> descriptor{"text"};
        return descriptor;
    }

    // C# MockBindable: a BindableObject with a TextProperty.
    struct mock_bindable : bindable_object
    {
        property<std::string> text{*this, text_prop()};
    };

    // An element-derived content type (to observe the IsTemplateRoot marking).
    struct mock_element : maui::controls::element
    {
    };

    // ---- DataTemplateTests ----

    TEST(data_template, ctor_invalid)
    {
        // C#: new DataTemplate((Func<object>)null) -> ArgumentNullException. (The (Type)null overload
        // cannot arise — the type ctor is the compile-time of<TControl>() factory.)
        EXPECT_THROW(data_template{data_template::loader{}}, std::invalid_argument);
    }

    TEST(data_template, create_content)
    {
        data_template tmpl{[] { return std::static_pointer_cast<bindable_object>(std::make_shared<mock_bindable>()); }};
        const auto content = tmpl.create_content();
        ASSERT_NE(content, nullptr);
        EXPECT_NE(std::dynamic_pointer_cast<mock_bindable>(content), nullptr);
        EXPECT_FALSE(tmpl.can_recycle()); // a Func-activated template is not declarative
    }

    TEST(data_template, create_content_type)
    {
        const auto tmpl = data_template::of<mock_bindable>();
        const auto content = tmpl->create_content();
        ASSERT_NE(content, nullptr);
        EXPECT_NE(std::dynamic_pointer_cast<mock_bindable>(content), nullptr);
        EXPECT_TRUE(tmpl->can_recycle()); // the Type ctor marks the template declarative
    }

    TEST(data_template, create_content_marks_an_element_root_as_template_root)
    {
        // ElementTemplate.CreateContent: `if (item is Element elem) elem.IsTemplateRoot = true`.
        const auto tmpl = data_template::of<mock_element>();
        const auto content = std::dynamic_pointer_cast<mock_element>(tmpl->create_content());
        ASSERT_NE(content, nullptr);
        EXPECT_TRUE(content->is_template_root());
    }

    TEST(data_template, create_content_values)
    {
        const auto tmpl = data_template::of<mock_bindable>();
        tmpl->set_value(text_prop(), std::string{"value"});
        const auto content = std::dynamic_pointer_cast<mock_bindable>(tmpl->create_content());
        ASSERT_NE(content, nullptr);
        EXPECT_EQ(content->text.get(), "value");
    }

    TEST(data_template, create_content_bindings)
    {
        // C#: Bindings = { { TextProperty, new Binding(".") } } — the self-path context binding.
        data_template tmpl{[] { return std::static_pointer_cast<bindable_object>(std::make_shared<mock_bindable>()); }};
        tmpl.set_binding(text_prop());
        const auto content = std::dynamic_pointer_cast<mock_bindable>(tmpl.create_content());
        ASSERT_NE(content, nullptr);
        content->set_binding_context(std::make_shared<std::string>("text"));
        EXPECT_EQ(content->text.get(), "text");
    }

    TEST(data_template, set_binding_overrides_value)
    {
        const auto tmpl = data_template::of<mock_bindable>();
        tmpl->set_value(text_prop(), std::string{"value"});
        tmpl->set_binding(text_prop()); // SetBinding removes the staged value

        const auto content = std::dynamic_pointer_cast<mock_bindable>(tmpl->create_content());
        ASSERT_NE(content, nullptr);
        // C#: GetValue == BindingContext (both null). Port: no context -> the property default.
        EXPECT_FALSE(content->has_binding_context());
        EXPECT_EQ(content->text.get(), "");

        content->set_binding_context(std::make_shared<std::string>("binding"));
        EXPECT_EQ(content->text.get(), "binding");
    }

    TEST(data_template, set_value_overrides_binding)
    {
        const auto tmpl = data_template::of<mock_bindable>();
        tmpl->set_binding(text_prop());
        tmpl->set_value(text_prop(), std::string{"value"}); // SetValue removes the staged binding

        const auto content = std::dynamic_pointer_cast<mock_bindable>(tmpl->create_content());
        ASSERT_NE(content, nullptr);
        EXPECT_EQ(content->text.get(), "value");
        content->set_binding_context(std::make_shared<std::string>("binding"));
        EXPECT_EQ(content->text.get(), "value");
    }

    TEST(data_template, set_value_and_binding_throws_on_create)
    {
        // The raw dictionary Adds (the C# collection initializers) stage BOTH for one property —
        // CreateContent throws InvalidOperationException ("Binding and Value found for ...").
        const auto tmpl = data_template::of<mock_bindable>();
        tmpl->add_binding(text_prop());
        tmpl->add_value(text_prop(), std::string{"Text"});
        EXPECT_THROW((void)tmpl->create_content(), std::runtime_error);
    }

    TEST(data_template, hot_reload_transition_does_not_crash)
    {
        // C#: a loader-less template's CreateContent returns a Label instead of throwing.
        const data_template tmpl;
        EXPECT_NE(tmpl.create_content(), nullptr);
    }

    TEST(data_template, id_surface_mirrors_i_data_template_controller)
    {
        // Ids increment process-wide from above 100 (the C# idCounter); the type factory shares ONE
        // id_string per TControl (C# type.FullName) while Func-activated templates get distinct ones.
        const data_template first{
            [] { return std::static_pointer_cast<bindable_object>(std::make_shared<mock_bindable>()); }};
        const data_template second{
            [] { return std::static_pointer_cast<bindable_object>(std::make_shared<mock_bindable>()); }};
        EXPECT_GT(first.id(), 100);
        EXPECT_GT(second.id(), first.id());
        EXPECT_NE(first.id_string(), second.id_string());

        const auto typed_a = data_template::of<mock_bindable>();
        const auto typed_b = data_template::of<mock_bindable>();
        EXPECT_NE(typed_a->id(), typed_b->id());               // ids stay per-instance
        EXPECT_EQ(typed_a->id_string(), typed_b->id_string()); // the reuse-identifier is per-type
        const auto typed_other = data_template::of<mock_element>();
        EXPECT_NE(typed_a->id_string(), typed_other->id_string());
    }

    // ---- DataTemplateSelectorTests ----

    // Two distinct content types so the selected templates are distinguishable (C# ViewCell/EntryCell).
    struct cell_one : maui::controls::element
    {
    };
    struct cell_two : maui::controls::element
    {
    };

    [[nodiscard]] data_template_selector::item_box make_item_box(auto value)
    {
        using value_type = decltype(value);
        return {.value = std::make_shared<value_type>(std::move(value)), .type = type_tag::of<value_type>()};
    }

    // C# TestDTS: double -> templateOne; byte -> a NEW selector (the nesting violation); else templateTwo.
    class test_dts : public data_template_selector
    {
    public:
        std::shared_ptr<data_template> template_one = data_template::of<cell_one>();
        std::shared_ptr<data_template> template_two = data_template::of<cell_two>();

    protected:
        std::shared_ptr<data_template> on_select_template(const item_box& item, bindable_object* /*container*/) override
        {
            if (item.type == type_tag::of<double>())
            {
                return template_one;
            }
            if (item.type == type_tag::of<std::uint8_t>())
            {
                return std::make_shared<test_dts>();
            }
            return template_two;
        }
    };

    TEST(data_template_selector, returns_correct_template)
    {
        test_dts selector;
        EXPECT_EQ(selector.select_template(make_item_box(1.0), nullptr), selector.template_one);
        EXPECT_EQ(selector.select_template(make_item_box(std::string{"test"}), nullptr), selector.template_two);
    }

    TEST(data_template_selector, nesting_throws_exception)
    {
        // C# NotSupportedException when OnSelectTemplate returns another selector.
        test_dts selector;
        EXPECT_THROW((void)selector.select_template(make_item_box(std::uint8_t{0}), nullptr), std::runtime_error);
    }

    TEST(data_template_selector, create_content_directly_throws)
    {
        // ElementTemplate.CreateContent: InvalidOperationException on a selector.
        test_dts selector;
        selector.set_load_template(
            [] { return std::static_pointer_cast<bindable_object>(std::make_shared<mock_bindable>()); });
        EXPECT_THROW((void)selector.create_content(), std::runtime_error);
    }

    // ---- DataTemplateRecycleTests (the RecycleElementAndDataTemplate semantics) ----

    // The container seam standing in for `new ListView(ListViewCachingStrategy.RecycleElementAndDataTemplate)`.
    struct recycling_container : bindable_object, i_template_recycling_container
    {
        [[nodiscard]] bool recycles_data_templates() const override
        {
            return true;
        }
    };

    // C# TestDataTemplateSelector: counts selections; string -> the declarative (type-activated)
    // template, anything else -> the procedural (Func-activated) one.
    class counting_selector : public data_template_selector
    {
    public:
        int counter = 0;
        std::shared_ptr<data_template> declarative_template = data_template::of<cell_one>();
        std::shared_ptr<data_template> procedural_template = std::make_shared<data_template>(
            [] { return std::static_pointer_cast<bindable_object>(std::make_shared<cell_two>()); });

    protected:
        std::shared_ptr<data_template> on_select_template(const item_box& item, bindable_object* /*container*/) override
        {
            ++counter;
            if (item.type == type_tag::of<std::string>())
            {
                return declarative_template;
            }
            return procedural_template;
        }
    };

    TEST(data_template_selector, recycling_caches_per_item_type_and_rejects_procedural_templates)
    {
        recycling_container container;
        counting_selector selector;
        EXPECT_EQ(selector.counter, 0);

        // "foo" -> select once...
        EXPECT_EQ(selector.select_template(make_item_box(std::string{"foo"}), &container),
                  selector.declarative_template);
        EXPECT_EQ(selector.counter, 1);

        // ..."bar" (same item type) -> served from the cache, OnSelectTemplate NOT called again.
        EXPECT_EQ(selector.select_template(make_item_box(std::string{"bar"}), &container),
                  selector.declarative_template);
        EXPECT_EQ(selector.counter, 1);

        // an int item selects the procedural template -> NotSupportedException (not recyclable).
        EXPECT_THROW((void)selector.select_template(make_item_box(0), &container), std::runtime_error);
    }

    TEST(data_template_selector, no_recycling_without_a_recycling_container)
    {
        // C#: recycle is false for a null / non-ListView container — every call selects afresh.
        counting_selector selector;
        (void)selector.select_template(make_item_box(std::string{"foo"}), nullptr);
        (void)selector.select_template(make_item_box(std::string{"bar"}), nullptr);
        EXPECT_EQ(selector.counter, 2);
        // and a procedural template is fine when not recycling.
        EXPECT_EQ(selector.select_template(make_item_box(0), nullptr), selector.procedural_template);
    }
} // namespace
