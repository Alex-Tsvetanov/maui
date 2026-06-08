// Tests for maui::controls::style (M5b) — a setter bundle applied at the style specificity, exercising
// the value-precedence ladder end-to-end through a real control (button) plus the property<T>
// self-registration seam. A manual set outranks a style; clearing it restores the style value; replacing
// a style un-applies the old; a based_on style is overridden by the derived one.
#include "maui/controls/style.hpp"

#include <memory>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/setter.hpp"
#include "maui/core/setter_specificity.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::button;
    using maui::controls::setter;
    using maui::controls::style;
    using maui::core::setter_specificity;

    TEST(style, applies_setters_to_the_target)
    {
        button target;
        style sheet = style::of<button>();
        sheet.add(setter::of(button::text_property(), std::string("Styled")));

        sheet.apply(target);
        EXPECT_EQ(target.text(), "Styled");
    }

    TEST(style, manual_set_overrides_style_then_clear_restores_it)
    {
        button target;
        style sheet = style::of<button>();
        sheet.add(setter::of(button::text_property(), std::string("Styled")));
        sheet.apply(target);
        EXPECT_EQ(target.text(), "Styled");

        target.set_text("Manual"); // manual_value_setter outranks the style specificity
        EXPECT_EQ(target.text(), "Manual");

        // Clearing the manual value falls back to the style value still sitting beneath it.
        target.clear_setter("text", setter_specificity::manual_value_setter);
        EXPECT_EQ(target.text(), "Styled");

        sheet.unapply(target); // removing the style falls back to the descriptor default
        EXPECT_EQ(target.text(), "");
    }

    TEST(style, multiple_setters_apply_each_property)
    {
        button target;
        style sheet = style::of<button>();
        sheet.add(setter::of(button::text_property(), std::string("Hi")));
        sheet.add(setter::of(button::character_spacing_property(), 4.0));

        sheet.apply(target);
        EXPECT_EQ(target.text(), "Hi");
        EXPECT_DOUBLE_EQ(target.character_spacing(), 4.0);
    }

    TEST(style, set_style_applies_and_replacing_unapplies_the_old)
    {
        button target;
        auto first = std::make_shared<style>(style::of<button>());
        first->add(setter::of(button::text_property(), std::string("A")));
        auto second = std::make_shared<style>(style::of<button>());
        second->add(setter::of(button::text_property(), std::string("B")));

        target.set_style(first);
        EXPECT_EQ(target.text(), "A");

        target.set_style(second); // un-applies `first`, applies `second`
        EXPECT_EQ(target.text(), "B");

        target.set_style(nullptr); // clearing the style falls back to the default
        EXPECT_EQ(target.text(), "");
    }

    TEST(style, based_on_setters_are_overridden_by_the_derived_style)
    {
        button target;
        auto base = std::make_shared<style>(style::of<button>());
        base->add(setter::of(button::text_property(), std::string("Base")));
        base->add(setter::of(button::character_spacing_property(), 9.0));

        style derived = style::of<button>();
        derived.add(setter::of(button::text_property(), std::string("Derived")));
        derived.set_based_on(base);

        derived.apply(target);
        EXPECT_EQ(target.text(), "Derived");               // derived wins over based_on for the shared property
        EXPECT_DOUBLE_EQ(target.character_spacing(), 9.0); // a based_on-only property still applies
    }
} // namespace
