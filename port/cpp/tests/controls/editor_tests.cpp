// Tests for the editor control + its headless handler seam, ported from
// src/Controls/tests/Core.UnitTests/EditorTests.cs (the TextChanged-args + IsReadOnly oracles) plus the
// seam suite every text control carries (properties flow virtual→native; a simulated native edit /
// end-of-edit flows native→virtual through the headless editor_platform's inbound hooks). The Apple
// backend (.mm) is the real-native twin verified separately.
#include "maui/controls/editor.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/editor_auto_size_option.hpp"
#include "maui/core/editor_handler.hpp"
#include "maui/core/font.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_editor.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_text.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::editor;
    using maui::controls::editor_auto_size_option;
    using maui::core::editor_handler;
    using maui::core::i_editor;
    using maui::core::i_element_handler;
    using maui::core::i_text;
    using maui::core::text_alignment;

    // ---- the control in isolation ----

    // EditorTests.EditorTextChangedEventArgs: a programmatic Text change raises TextChanged with the
    // (old, new) pair (C# nulls collapse to empty strings in the string-typed port).
    TEST(editor, programmatic_text_change_raises_text_changed_with_old_and_new)
    {
        editor control;
        control.set_text("Hi");

        std::vector<std::pair<std::string, std::string>> changes;
        control.text_changed.connect([&changes](const std::string& old_value, const std::string& new_value) {
            changes.emplace_back(old_value, new_value);
        });

        control.set_text("My text has changed");
        ASSERT_EQ(changes.size(), 1U);
        EXPECT_EQ(changes[0].first, "Hi");
        EXPECT_EQ(changes[0].second, "My text has changed");

        // The (initial, empty) case ("Hi" -> null collapses to "").
        control.set_text("");
        ASSERT_EQ(changes.size(), 2U);
        EXPECT_EQ(changes[1].first, "My text has changed");
        EXPECT_EQ(changes[1].second, "");
    }

    TEST(editor, unchanged_text_does_not_raise_text_changed)
    {
        editor control;
        control.set_text("Same");
        int raised = 0;
        control.text_changed.connect([&raised](const std::string&, const std::string&) { ++raised; });
        control.set_text("Same");
        EXPECT_EQ(raised, 0);
    }

    // EditorTests.IsReadOnlyTest / IsReadOnlyDefaultValueTest.
    TEST(editor, is_read_only_defaults_false_and_is_settable)
    {
        editor control;
        EXPECT_FALSE(control.is_read_only());
        control.set_is_read_only(true);
        EXPECT_TRUE(control.is_read_only());
    }

    TEST(editor, defaults_mirror_input_view)
    {
        editor control;
        EXPECT_EQ(control.text(), "");
        EXPECT_EQ(control.placeholder(), "");
        EXPECT_TRUE(control.is_text_prediction_enabled());
        EXPECT_TRUE(control.is_spell_check_enabled());
        EXPECT_EQ(control.cursor_position(), 0);
        EXPECT_EQ(control.selection_length(), 0);
        // Editor-specific defaults: AutoSize disabled, Start/Start alignment (Editor.cs).
        EXPECT_EQ(control.auto_size(), editor_auto_size_option::disabled);
        EXPECT_EQ(control.horizontal_text_alignment(), text_alignment::start);
        EXPECT_EQ(control.vertical_text_alignment(), text_alignment::start);
    }

    TEST(editor, set_text_truncates_to_max_length_and_lowering_retrims)
    {
        editor control;
        control.set_max_length(3);
        control.set_text("abcdef");
        EXPECT_EQ(control.text(), "abc");

        editor other;
        other.set_text("abcdef");
        other.set_max_length(2);
        EXPECT_EQ(other.text(), "ab");
    }

    // Editor.SendCompleted has NO IsEnabled gate (unlike Entry) — completed fires even when disabled.
    TEST(editor, send_completed_raises_completed_even_when_disabled)
    {
        editor control;
        int completes = 0;
        control.completed.connect([&completes] { ++completes; });
        control.send_completed();
        EXPECT_EQ(completes, 1);

        control.set_is_enabled(false);
        control.send_completed();
        EXPECT_EQ(completes, 2);
    }

    TEST(editor, send_text_changed_raises_text_changed_with_old_and_new)
    {
        editor control;
        std::vector<std::pair<std::string, std::string>> changes;
        control.text_changed.connect([&changes](const std::string& old_value, const std::string& new_value) {
            changes.emplace_back(old_value, new_value);
        });

        control.send_text_changed("old", "new");
        ASSERT_EQ(changes.size(), 1U);
        EXPECT_EQ(changes[0].first, "old");
        EXPECT_EQ(changes[0].second, "new");
    }

    TEST(editor, cursor_and_selection_default_zero_and_clamp_floor)
    {
        editor control;
        control.set_cursor_position(3);
        control.set_selection_length(2);
        EXPECT_EQ(control.cursor_position(), 3);
        EXPECT_EQ(control.selection_length(), 2);

        control.set_cursor_position(-5);
        control.set_selection_length(-1);
        EXPECT_EQ(control.cursor_position(), 0);
        EXPECT_EQ(control.selection_length(), 0);
    }

    TEST(editor, usable_through_interface_references)
    {
        editor control;
        control.set_text("Caption");
        i_editor& as_editor = control;
        i_text& as_text = control;
        EXPECT_EQ(as_editor.text(), "Caption");
        EXPECT_EQ(as_text.text(), "Caption");
        EXPECT_EQ(as_editor.vertical_text_alignment(), text_alignment::start);
    }

    // ---- the handler seam (control <-> handler <-> headless platform) ----

    TEST(editor_seam, attaching_handler_maps_initial_properties)
    {
        editor control;
        control.set_text("Start");
        control.set_placeholder("Hint");
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), &control);
        auto* platform = handler->typed_platform_view();
        EXPECT_EQ(platform->text, "Start");
        EXPECT_EQ(platform->placeholder, "Hint");
        EXPECT_EQ(platform->vertical_alignment, text_alignment::start);
    }

    TEST(editor_seam, setting_properties_maps_to_platform)
    {
        editor control;
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        control.set_text("Changed");
        EXPECT_EQ(platform->text, "Changed");

        control.set_placeholder("Placeholder");
        EXPECT_EQ(platform->placeholder, "Placeholder");

        control.set_placeholder_color(maui::graphics::color(0.0F, 1.0F, 0.0F));
        EXPECT_EQ(platform->placeholder_color, maui::graphics::color(0.0F, 1.0F, 0.0F));

        control.set_is_read_only(true);
        EXPECT_TRUE(platform->is_read_only);

        control.set_max_length(10);
        EXPECT_EQ(platform->max_length, 10);

        control.set_text_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        EXPECT_EQ(platform->text_color, maui::graphics::color(1.0F, 0.0F, 0.0F));

        control.set_font(maui::core::font::of_size("Arial", 14));
        EXPECT_EQ(platform->text_font.family(), "Arial");
        EXPECT_EQ(platform->text_font.size(), 14.0);

        control.set_character_spacing(2.5);
        EXPECT_EQ(platform->character_spacing, 2.5);

        control.set_horizontal_text_alignment(text_alignment::center);
        EXPECT_EQ(platform->horizontal_alignment, text_alignment::center);

        control.set_vertical_text_alignment(text_alignment::end);
        EXPECT_EQ(platform->vertical_alignment, text_alignment::end);

        control.set_is_text_prediction_enabled(false);
        EXPECT_FALSE(platform->is_text_prediction_enabled);

        control.set_is_spell_check_enabled(false);
        EXPECT_FALSE(platform->is_spell_check_enabled);

        control.set_cursor_position(4);
        EXPECT_EQ(platform->cursor_position, 4);

        control.set_selection_length(2);
        EXPECT_EQ(platform->selection_length, 2);
    }

    TEST(editor_seam, simulated_native_edit_fires_text_changed)
    {
        editor control;
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        std::vector<std::pair<std::string, std::string>> changes;
        control.text_changed.connect([&changes](const std::string& old_value, const std::string& new_value) {
            changes.emplace_back(old_value, new_value);
        });

        platform->on_text_changed("", "ab");
        ASSERT_EQ(changes.size(), 1U);
        EXPECT_EQ(changes[0].first, "");
        EXPECT_EQ(changes[0].second, "ab");
        EXPECT_EQ(platform->last_known_text, "ab");
    }

    TEST(editor_seam, simulated_end_of_edit_fires_completed)
    {
        editor control;
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        int completes = 0;
        control.completed.connect([&completes] { ++completes; });
        platform->on_completed();
        EXPECT_EQ(completes, 1);
    }

    TEST(editor_seam, clearing_handler_disconnects)
    {
        editor control;
        auto handler = std::make_shared<editor_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(control.handler(), nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST(editor_seam, handler_resolved_from_default_registry)
    {
        // editor -> editor_handler is self-registered in editor.cpp (MAUI_REGISTER_HANDLER).
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<editor>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<editor_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        editor control;
        control.set_text("Registered");
        control.set_handler(handler);
        EXPECT_EQ(resolved->typed_platform_view()->text, "Registered");
    }
} // namespace
