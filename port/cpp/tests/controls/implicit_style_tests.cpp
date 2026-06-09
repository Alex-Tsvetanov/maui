// Tests for implicit styles, style classes, and BasedOn-by-resource-key (M5d). Ported from StyleTests.cs
// (the implicit-style, ClassStyles, DynamicStyle, and UnapplyingStyleDefaultToImplicit families). Implicit
// styles are TargetType-keyed (resource_dictionary::add(style)); a control selects class styles via
// style_class; a style's base_resource_key resolves its base from the element's resource chain. Layering:
// implicit (style_implicit) < class (style_class) < local (style_local) < manual.
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/style.hpp"
#include "maui/controls/vertical_stack_layout.hpp"

#include <memory>
#include <string>
#include <utility>

#include "maui/controls/setter.hpp"
#include "maui/core/setter_specificity.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::content_page;
    using maui::controls::label;
    using maui::controls::setter;
    using maui::controls::style;
    using maui::controls::vertical_stack_layout;

    std::shared_ptr<style> label_text_style(std::string text)
    {
        auto sheet = std::make_shared<style>(style::of<label>());
        sheet->add(setter::of(label::text_property(), std::move(text)));
        return sheet;
    }

    TEST(implicit_style, applied_when_setting_the_resource_dictionary_with_a_present_child)
    {
        label lbl;
        vertical_stack_layout layout;
        layout.add(lbl);
        EXPECT_EQ(lbl.text(), "");

        layout.resources().add(label_text_style("implicit")); // implicit style for `label`
        EXPECT_EQ(lbl.text(), "implicit");
    }

    TEST(implicit_style, applied_when_attaching_a_child_to_a_layout_that_has_the_resource)
    {
        vertical_stack_layout layout;
        layout.resources().add(label_text_style("implicit"));

        label lbl;
        EXPECT_EQ(lbl.text(), "");
        layout.add(lbl); // attaching resolves the implicit style from the parent
        EXPECT_EQ(lbl.text(), "implicit");
    }

    TEST(implicit_style, overridden_by_a_local_style)
    {
        label lbl;
        vertical_stack_layout layout;
        layout.resources().add(label_text_style("implicit"));
        layout.add(lbl);
        EXPECT_EQ(lbl.text(), "implicit");

        lbl.set_style(label_text_style("local")); // local style (style_local) outranks implicit
        EXPECT_EQ(lbl.text(), "local");
    }

    TEST(implicit_style, reemerges_when_the_local_style_is_unset)
    {
        label lbl;
        vertical_stack_layout layout;
        layout.resources().add(label_text_style("implicit"));
        layout.add(lbl);
        lbl.set_style(label_text_style("local"));
        EXPECT_EQ(lbl.text(), "local");

        lbl.set_style(nullptr); // clearing the local style falls back to the implicit one beneath
        EXPECT_EQ(lbl.text(), "implicit");
    }

    TEST(implicit_style, applies_to_the_exact_target_type)
    {
        content_page page;
        page.resources().add(label_text_style("implicit"));
        label lbl;
        page.set_content(lbl);
        EXPECT_EQ(lbl.text(), "implicit"); // the label picks it up (exact type-tag match)
    }

    TEST(implicit_style, class_styles_are_applied_when_selected)
    {
        auto class_style = std::make_shared<style>(style::of<label>());
        class_style->set_style_class("fooClass");
        class_style->add(setter::of(label::text_property(), std::string("Foo")));

        content_page page;
        page.resources().add(class_style);

        label lbl;
        lbl.set_style_class({"fooClass"});
        page.set_content(lbl);
        EXPECT_EQ(lbl.text(), "Foo"); // the class style selected via style_class is applied
    }

    TEST(implicit_style, base_resource_key_resolves_the_base_style_from_the_chain)
    {
        // Style.DynamicStyle: a style with BaseResourceKey resolves its base from the resource chain; the
        // derived style's own setters win, base-only setters apply.
        auto base = label_text_style("base");

        auto derived = std::make_shared<style>(style::of<label>());
        derived->set_base_resource_key("basestyle");
        derived->add(setter::of(label::character_spacing_property(), 3.0)); // a derived-only setter

        label lbl;
        lbl.set_style(derived);
        EXPECT_DOUBLE_EQ(lbl.character_spacing(), 3.0);
        EXPECT_EQ(lbl.text(), ""); // base not yet resolvable -> base setter not applied

        vertical_stack_layout layout;
        layout.resources().add("basestyle", base);
        layout.add(lbl); // now the chain holds "basestyle" -> the base style's `text` applies
        EXPECT_EQ(lbl.text(), "base");
        EXPECT_DOUBLE_EQ(lbl.character_spacing(), 3.0);
    }

    TEST(implicit_style, unapplying_local_style_falls_back_to_implicit_then_to_default)
    {
        // UnapplyingStyleDefaultToImplicit1: implicit < local < manual; clearing the manual then the local
        // peels back to implicit.
        vertical_stack_layout layout;
        layout.resources().add(label_text_style("implicit"));
        label lbl;
        EXPECT_EQ(lbl.text(), "");

        layout.add(lbl);
        EXPECT_EQ(lbl.text(), "implicit");

        lbl.set_style(label_text_style("style"));
        EXPECT_EQ(lbl.text(), "style");

        lbl.set_text("value"); // manual outranks the local style
        EXPECT_EQ(lbl.text(), "value");

        lbl.clear_setter("text", maui::core::setter_specificity::manual_value_setter);
        EXPECT_EQ(lbl.text(), "style"); // back to the local style

        lbl.set_style(nullptr);
        EXPECT_EQ(lbl.text(), "implicit"); // back to the implicit style
    }
} // namespace
