// Tests for the entry control + its headless handler seam — the first editable, inbound-text control:
// properties flow virtual→native, and a simulated native edit / end-of-edit flows native→virtual through
// the headless entry_platform's inbound hooks. The Apple backend (.mm) is the real-native twin verified
// separately.
#include "maui/controls/entry.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/core/clear_button_visibility.hpp"
#include "maui/core/entry_handler.hpp"
#include "maui/core/font.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_entry.hpp"
#include "maui/core/i_text.hpp"
#include "maui/core/return_type.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::entry;
    using maui::core::clear_button_visibility;
    using maui::core::entry_handler;
    using maui::core::i_element_handler;
    using maui::core::i_entry;
    using maui::core::i_text;
    using maui::core::return_type;
    using maui::core::text_alignment;

    // ---- the control in isolation ----

    TEST(entry, text_and_placeholder_default_empty_and_are_settable)
    {
        entry control;
        EXPECT_EQ(control.text(), "");
        EXPECT_EQ(control.placeholder(), "");
        EXPECT_FALSE(control.is_password());
        EXPECT_FALSE(control.is_read_only());

        control.set_text("Hello");
        control.set_placeholder("Type here");
        EXPECT_EQ(control.text(), "Hello");
        EXPECT_EQ(control.placeholder(), "Type here");
    }

    TEST(entry, set_text_truncates_to_max_length)
    {
        entry control;
        control.set_max_length(3);
        control.set_text("abcdef");
        EXPECT_EQ(control.text(), "abc");
    }

    TEST(entry, lowering_max_length_truncates_existing_text)
    {
        entry control;
        control.set_text("abcdef");
        EXPECT_EQ(control.text(), "abcdef"); // default max_length is int.MaxValue (no cap)
        control.set_max_length(2);
        EXPECT_EQ(control.text(), "ab");
    }

    TEST(entry, return_type_and_clear_button_defaults_and_settable)
    {
        entry control;
        // C# defaults: ReturnType.Default, ClearButtonVisibility.Never.
        EXPECT_EQ(control.return_type(), return_type::default_);
        EXPECT_EQ(control.clear_button_visibility(), clear_button_visibility::never);

        control.set_return_type(return_type::next);
        control.set_clear_button_visibility(clear_button_visibility::while_editing);
        EXPECT_EQ(control.return_type(), return_type::next);
        EXPECT_EQ(control.clear_button_visibility(), clear_button_visibility::while_editing);
    }

    TEST(entry, prediction_and_spellcheck_default_true_and_settable)
    {
        entry control;
        // C# InputView defaults: both true.
        EXPECT_TRUE(control.is_text_prediction_enabled());
        EXPECT_TRUE(control.is_spell_check_enabled());

        control.set_is_text_prediction_enabled(false);
        control.set_is_spell_check_enabled(false);
        EXPECT_FALSE(control.is_text_prediction_enabled());
        EXPECT_FALSE(control.is_spell_check_enabled());
    }

    TEST(entry, cursor_and_selection_default_zero_and_clamp_floor)
    {
        entry control;
        EXPECT_EQ(control.cursor_position(), 0);
        EXPECT_EQ(control.selection_length(), 0);

        control.set_cursor_position(3);
        control.set_selection_length(2);
        EXPECT_EQ(control.cursor_position(), 3);
        EXPECT_EQ(control.selection_length(), 2);

        // C# validateValue rejects negatives; the control clamps the floor to 0.
        control.set_cursor_position(-5);
        control.set_selection_length(-1);
        EXPECT_EQ(control.cursor_position(), 0);
        EXPECT_EQ(control.selection_length(), 0);
    }

    TEST(entry, send_completed_raises_completed_when_enabled)
    {
        entry control;
        int completes = 0;
        control.completed.connect([&completes] { ++completes; });
        control.send_completed();
        EXPECT_EQ(completes, 1);
    }

    TEST(entry, disabled_entry_suppresses_completed)
    {
        entry control;
        control.set_is_enabled(false);
        int completes = 0;
        control.completed.connect([&completes] { ++completes; });
        control.send_completed();
        EXPECT_EQ(completes, 0);
    }

    TEST(entry, send_text_changed_raises_text_changed_with_old_and_new)
    {
        entry control;
        std::vector<std::pair<std::string, std::string>> changes;
        control.text_changed.connect([&changes](const std::string& old_value, const std::string& new_value) {
            changes.emplace_back(old_value, new_value);
        });

        control.send_text_changed("old", "new");
        ASSERT_EQ(changes.size(), 1U);
        EXPECT_EQ(changes[0].first, "old");
        EXPECT_EQ(changes[0].second, "new");
    }

    TEST(entry, usable_through_interface_references)
    {
        entry control;
        control.set_text("Caption");
        i_entry& as_entry = control;
        i_text& as_text = control;
        EXPECT_EQ(as_entry.text(), "Caption");
        EXPECT_EQ(as_text.text(), "Caption");
        EXPECT_EQ(as_entry.horizontal_text_alignment(), text_alignment::start);
    }

    // ---- the handler seam (control <-> handler <-> headless platform) ----

    TEST(entry_seam, attaching_handler_maps_initial_properties)
    {
        entry control;
        control.set_text("Start");
        control.set_placeholder("Hint");
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), &control);
        auto* platform = handler->typed_platform_view();
        EXPECT_EQ(platform->text, "Start");
        EXPECT_EQ(platform->placeholder, "Hint");
    }

    TEST(entry_seam, setting_properties_maps_to_platform)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        control.set_text("Changed");
        EXPECT_EQ(platform->text, "Changed");

        control.set_placeholder("Placeholder");
        EXPECT_EQ(platform->placeholder, "Placeholder");

        control.set_is_password(true);
        EXPECT_TRUE(platform->is_password);

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

        control.set_return_type(return_type::go);
        EXPECT_EQ(platform->entry_return_type, return_type::go);

        control.set_clear_button_visibility(clear_button_visibility::while_editing);
        EXPECT_EQ(platform->clear_button, clear_button_visibility::while_editing);

        control.set_is_text_prediction_enabled(false);
        EXPECT_FALSE(platform->is_text_prediction_enabled);

        control.set_is_spell_check_enabled(false);
        EXPECT_FALSE(platform->is_spell_check_enabled);

        control.set_cursor_position(4);
        EXPECT_EQ(platform->cursor_position, 4);

        control.set_selection_length(2);
        EXPECT_EQ(platform->selection_length, 2);
    }

    TEST(entry_seam, simulated_native_edit_fires_text_changed)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        std::vector<std::pair<std::string, std::string>> changes;
        control.text_changed.connect([&changes](const std::string& old_value, const std::string& new_value) {
            changes.emplace_back(old_value, new_value);
        });

        // Simulate the native field reporting an edit from "" to "ab".
        platform->on_text_changed("", "ab");
        ASSERT_EQ(changes.size(), 1U);
        EXPECT_EQ(changes[0].first, "");
        EXPECT_EQ(changes[0].second, "ab");
        EXPECT_EQ(platform->last_known_text, "ab"); // the hook tracks the latest value
    }

    TEST(entry_seam, simulated_end_of_edit_fires_completed)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        int completes = 0;
        control.completed.connect([&completes] { ++completes; });
        platform->on_completed();
        EXPECT_EQ(completes, 1);
    }

    TEST(entry_seam, clearing_handler_disconnects)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(control.handler(), nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST(entry_seam, handler_resolved_from_default_registry)
    {
        // entry -> entry_handler is self-registered in entry.cpp (MAUI_REGISTER_HANDLER).
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<entry>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<entry_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        entry control;
        control.set_text("Registered");
        control.set_handler(handler);
        EXPECT_EQ(resolved->typed_platform_view()->text, "Registered");
    }
} // namespace
