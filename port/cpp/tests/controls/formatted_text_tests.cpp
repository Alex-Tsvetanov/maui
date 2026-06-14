// Tests for the rich per-span label text — span + formatted_string + the label.formatted_text integration
// (gap-closure G1). Backend-agnostic: ported from the C# oracle (FormattedStringTests / SpanTests) plus
// the per-span attribute-flow assertions against the headless label_platform run mirror.
#include "maui/controls/formatted_string.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/span.hpp"

#include <memory>
#include <stdexcept>
#include <string>

#include "maui/controls/gestures/pan_gesture_recognizer.hpp"
#include "maui/controls/gestures/tap_gesture_recognizer.hpp"
#include "maui/core/font.hpp"
#include "maui/core/font_attributes.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/text_decorations.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::formatted_string;
    using maui::controls::label;
    using maui::controls::pan_gesture_recognizer;
    using maui::controls::span;
    using maui::controls::tap_gesture_recognizer;
    using maui::core::font;
    using maui::core::font_attributes;
    using maui::core::label_handler;
    using maui::core::text_decorations;
    using maui::graphics::color;

    // ---- span ----

    TEST(span, text_defaults_empty_and_is_settable)
    {
        span s;
        EXPECT_EQ(s.text(), "");
        s.set_text("Hi");
        EXPECT_EQ(s.text(), "Hi");
    }

    TEST(span, font_fields_default_unset_and_track_is_set)
    {
        span s;
        EXPECT_FALSE(s.is_font_family_set());
        EXPECT_FALSE(s.is_font_size_set());
        EXPECT_FALSE(s.is_font_attributes_set());
        EXPECT_EQ(s.font_attributes(), font_attributes::none);

        s.set_font_family("Helvetica");
        s.set_font_size(20);
        s.set_font_attributes(font_attributes::bold);
        EXPECT_TRUE(s.is_font_family_set());
        EXPECT_TRUE(s.is_font_size_set());
        EXPECT_TRUE(s.is_font_attributes_set());
        EXPECT_EQ(s.font_family(), "Helvetica");
        EXPECT_EQ(s.font_size(), 20);
    }

    TEST(span, get_effective_font_uses_span_when_set_else_default)
    {
        const font default_font = font::of_size("Default", 12);

        span s;
        // Nothing set: inherits the default (family + size + attributes).
        const font inherited = s.get_effective_font(12, default_font);
        EXPECT_EQ(inherited.family(), "Default");
        EXPECT_EQ(inherited.size(), 12);

        // Span sets size + bold + italic: those override; family still inherits.
        s.set_font_size(30);
        s.set_font_attributes(font_attributes::bold | font_attributes::italic);
        const font effective = s.get_effective_font(12, default_font);
        EXPECT_EQ(effective.family(), "Default");
        EXPECT_EQ(effective.size(), 30);
        EXPECT_EQ(effective.weight(), maui::core::font_weight::bold);
        EXPECT_EQ(effective.slant(), maui::core::font_slant::italic);
    }

    TEST(span, only_tap_gesture_recognizer_is_allowed)
    {
        span s;
        auto tap = std::make_shared<tap_gesture_recognizer>();
        s.gesture_recognizers().add(tap); // allowed
        EXPECT_EQ(s.gesture_recognizers().count(), 1U);

        auto pan = std::make_shared<pan_gesture_recognizer>();
        EXPECT_THROW(s.gesture_recognizers().add(pan), std::runtime_error); // ValidateGesture
        EXPECT_EQ(s.gesture_recognizers().count(), 1U);                     // the rejected one never entered
    }

    // ---- formatted_string ----

    TEST(formatted_string, null_spans_not_allowed)
    {
        formatted_string fs;
        EXPECT_THROW(fs.add_span(nullptr), std::runtime_error);

        fs.add_span(std::make_shared<span>());
        EXPECT_THROW(fs.set_span(0, nullptr), std::runtime_error);
    }

    TEST(formatted_string, to_string_concatenates_span_text)
    {
        formatted_string fs;
        auto a = std::make_shared<span>();
        a->set_text("Hello, ");
        auto b = std::make_shared<span>();
        b->set_text("world");
        fs.add_span(a);
        fs.add_span(b);
        EXPECT_EQ(fs.to_string(), "Hello, world");
    }

    TEST(formatted_string, from_string_makes_a_single_span)
    {
        const auto fs = formatted_string::from_string("fubar");
        ASSERT_EQ(fs->span_count(), 1U);
        EXPECT_EQ(fs->span_at(0)->text(), "fubar");
        EXPECT_EQ(fs->to_string(), "fubar");
    }

    // C# FormattedStringTests.AddingSpanTriggersSpansPropertyChange.
    TEST(formatted_string, adding_span_triggers_changed)
    {
        formatted_string fs;
        bool changed = false;
        const auto token = fs.changed.connect([&] { changed = true; });
        fs.add_span(std::make_shared<span>());
        EXPECT_TRUE(changed);
        fs.changed.disconnect(token);
    }

    // C# FormattedStringTests.SpanChangeTriggersSpansPropertyChange.
    TEST(formatted_string, span_change_triggers_changed)
    {
        auto s = std::make_shared<span>();
        formatted_string fs;
        fs.add_span(s);

        bool changed = false;
        const auto token = fs.changed.connect([&] { changed = true; });
        s->set_text("New text");
        EXPECT_TRUE(changed);
        fs.changed.disconnect(token);
    }

    // C# FormattedStringTests.SpanChangesUnsubscribes — a removed span no longer notifies the owner.
    TEST(formatted_string, removed_span_unsubscribes)
    {
        auto s = std::make_shared<span>();
        formatted_string fs;
        fs.add_span(s);
        EXPECT_TRUE(fs.remove_span(s));

        bool changed = false;
        const auto token = fs.changed.connect([&] { changed = true; });
        s->set_text("New text");
        EXPECT_FALSE(changed);
        fs.changed.disconnect(token);
    }

    TEST(formatted_string, clear_unsubscribes_every_span)
    {
        auto s = std::make_shared<span>();
        formatted_string fs;
        fs.add_span(s);
        fs.clear_spans();
        EXPECT_EQ(fs.span_count(), 0U);

        bool changed = false;
        const auto token = fs.changed.connect([&] { changed = true; });
        s->set_text("changed");
        EXPECT_FALSE(changed);
        fs.changed.disconnect(token);
    }

    // ---- label integration ----

    TEST(label_formatted_text, setting_formatted_text_clears_text)
    {
        label control;
        control.set_text("plain");
        EXPECT_EQ(control.text(), "plain");

        control.set_formatted_text(formatted_string::from_string("rich"));
        EXPECT_EQ(control.text(), ""); // FormattedText and Text are mutually exclusive (Label.cs)
        ASSERT_NE(control.formatted_text(), nullptr);
        EXPECT_EQ(control.formatted_text()->to_string(), "rich");
    }

    TEST(label_formatted_text, setting_text_clears_formatted_text)
    {
        label control;
        control.set_formatted_text(formatted_string::from_string("rich"));
        ASSERT_NE(control.formatted_text(), nullptr);

        control.set_text("plain");
        EXPECT_EQ(control.formatted_text(), nullptr);
        EXPECT_EQ(control.text(), "plain");
    }

    TEST(label_formatted_text, runs_resolve_per_span_attributes)
    {
        label control;
        control.set_font(font::of_size("Default", 14));
        control.set_text_color(color::from_rgb(0.0F, 0.0F, 0.0F)); // label default text color

        auto fs = std::make_shared<formatted_string>();
        auto red = std::make_shared<span>();
        red->set_text("red");
        red->set_text_color(color::from_rgb(1.0F, 0.0F, 0.0F));
        red->set_font_size(20);
        red->set_font_attributes(font_attributes::bold);
        red->set_character_spacing(3);
        red->set_text_decorations(text_decorations::underline);
        auto plain = std::make_shared<span>();
        plain->set_text("plain");
        fs->add_span(red);
        fs->add_span(plain);
        control.set_formatted_text(fs);

        const auto& runs = control.formatted_text_runs();
        ASSERT_EQ(runs.size(), 2U);

        // Run 0: the span's own color / bold / size / kerning / underline.
        EXPECT_EQ(runs[0].text, "red");
        ASSERT_TRUE(runs[0].text_color.has_value());
        EXPECT_EQ(*runs[0].text_color, color::from_rgb(1.0F, 0.0F, 0.0F));
        EXPECT_EQ(runs[0].run_font.size(), 20);
        EXPECT_EQ(runs[0].run_font.weight(), maui::core::font_weight::bold);
        EXPECT_EQ(runs[0].character_spacing, 3);
        EXPECT_EQ(runs[0].decorations, text_decorations::underline);

        // Run 1: unset span inherits the label defaults (color, font size 14, no decorations).
        EXPECT_EQ(runs[1].text, "plain");
        ASSERT_TRUE(runs[1].text_color.has_value());
        EXPECT_EQ(*runs[1].text_color, color::from_rgb(0.0F, 0.0F, 0.0F));
        EXPECT_EQ(runs[1].run_font.size(), 14);
        EXPECT_EQ(runs[1].decorations, text_decorations::none);
    }

    TEST(label_formatted_text, negative_character_spacing_is_clamped_to_zero)
    {
        label control;
        auto fs = std::make_shared<formatted_string>();
        auto s = std::make_shared<span>();
        s->set_text("x");
        s->set_character_spacing(-5);
        fs->add_span(s);
        control.set_formatted_text(fs);

        ASSERT_EQ(control.formatted_text_runs().size(), 1U);
        EXPECT_EQ(control.formatted_text_runs()[0].character_spacing, 0); // Math.Max(0, ...)
    }

    // ---- the handler seam: per-span attributes flow to the platform mirror (HEADLESS ONLY) ----
    // label_platform::formatted_text_runs is the headless mirror; the apple/ios builds turn the runs into
    // an NSAttributedString instead (asserted by the *_apple/_ios twins), so this run-mirror assertion is
    // headless-specific. The apple/ios builds compile this file too, so guard it off there.
#if !defined(MAUI_PLATFORM_APPLE) && !defined(MAUI_PLATFORM_IOS)
    TEST(label_formatted_text_seam, runs_flow_to_platform_mirror_and_clearing_reverts)
    {
        label control;
        control.set_font(font::of_size("Default", 14));
        auto fs = std::make_shared<formatted_string>();
        auto a = std::make_shared<span>();
        a->set_text("AA");
        a->set_text_color(color::from_rgb(0.0F, 1.0F, 0.0F));
        auto b = std::make_shared<span>();
        b->set_text("BB");
        fs->add_span(a);
        fs->add_span(b);
        control.set_formatted_text(fs);

        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        ASSERT_EQ(platform->formatted_text_runs.size(), 2U);
        EXPECT_EQ(platform->formatted_text_runs[0].text, "AA");
        ASSERT_TRUE(platform->formatted_text_runs[0].text_color.has_value());
        EXPECT_EQ(*platform->formatted_text_runs[0].text_color, color::from_rgb(0.0F, 1.0F, 0.0F));
        EXPECT_EQ(platform->formatted_text_runs[1].text, "BB");

        // A later span change re-resolves + re-maps through the handler.
        a->set_text("ZZ");
        ASSERT_EQ(platform->formatted_text_runs.size(), 2U);
        EXPECT_EQ(platform->formatted_text_runs[0].text, "ZZ");

        // Clearing reverts to the plain text path (the run mirror empties).
        control.set_text("plain");
        EXPECT_TRUE(platform->formatted_text_runs.empty());
        EXPECT_EQ(platform->text, "plain");
    }
#endif // headless-only
} // namespace
