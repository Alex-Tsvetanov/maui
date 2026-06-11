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

    // ---- W1-15: Style.ApplyToDerivedTypes over the DECLARED base-type chain ----
    // C# walks Type.BaseType (MergedStyle.RegisterImplicitStyles); the reflection-free port has the
    // derived control declare its chain: set_style_target_type<my_label, label>().

    struct my_label : label
    {
        my_label()
        {
            this->set_style_target_type<my_label, label>();
        }
    };

    TEST(implicit_style, not_applied_to_derived_types_by_default)
    {
        // ImplicitStylesNotAppliedToDerivedTypesByDefault: a label-targeted implicit style without the
        // flag does not reach a my_label.
        my_label lbl;
        vertical_stack_layout layout;
        layout.resources().add(label_text_style("Foo"));
        layout.add(lbl);
        EXPECT_EQ(lbl.text(), "");
    }

    TEST(implicit_style, applied_to_derived_types_if_specified)
    {
        // ImplicitStylesAreAppliedToDerivedIfSpecified: with ApplyToDerivedTypes the base-type style
        // applies to the derived control.
        auto sheet = label_text_style("Foo");
        sheet->set_apply_to_derived_types(true);

        my_label lbl;
        vertical_stack_layout layout;
        layout.resources().add(sheet);
        layout.add(lbl);
        EXPECT_EQ(lbl.text(), "Foo");
    }

    TEST(implicit_style, the_exact_type_implicit_style_outranks_a_flagged_base_type_one)
    {
        // MergedStyle.OnImplicitStyleChanged walks the chain most-derived first: an implicit style keyed
        // on the EXACT type wins over a flagged base-type style.
        auto base_style = label_text_style("base");
        base_style->set_apply_to_derived_types(true);
        auto exact = std::make_shared<style>(style::of<my_label>());
        exact->add(setter::of(label::text_property(), std::string("exact")));

        my_label lbl;
        vertical_stack_layout layout;
        layout.resources().add(base_style);
        layout.resources().add(exact);
        layout.add(lbl);
        EXPECT_EQ(lbl.text(), "exact");
    }

    TEST(implicit_style, class_styles_honor_apply_to_derived_types)
    {
        // MultipleStylesCanShareTheSameClassName (the derived-control rows): a flagged base-type class
        // style applies to the derived control; an unflagged one does not (CanBeAppliedTo).
        auto flagged = std::make_shared<style>(style::of<label>());
        flagged->set_style_class("pink");
        flagged->set_apply_to_derived_types(true);
        flagged->add(setter::of(label::text_property(), std::string("pink")));

        {
            my_label lbl;
            lbl.set_style_class({"pink"});
            vertical_stack_layout layout;
            layout.resources().add(flagged);
            layout.add(lbl);
            EXPECT_EQ(lbl.text(), "pink"); // flagged class style reaches the derived control
        }

        auto unflagged = std::make_shared<style>(style::of<label>());
        unflagged->set_style_class("plain");
        unflagged->add(setter::of(label::text_property(), std::string("plain")));

        {
            my_label lbl;
            lbl.set_style_class({"plain"});
            vertical_stack_layout layout;
            layout.resources().add(unflagged);
            layout.add(lbl);
            EXPECT_EQ(lbl.text(), ""); // unflagged class style skips the derived control
        }
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
