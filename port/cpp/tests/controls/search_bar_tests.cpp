// Tests for the search_bar control + its headless handler seam, ported from
// src/Controls/tests/Core.UnitTests/SearchBarUnitTests.cs (constructor defaults, TestContentsChanged,
// TestSearchButtonPressed, TestSearchCommandParameter — collapsed to the command stand-in —,
// SearchBarTextChangedEventArgs) plus the seam suite every text control carries. The Apple backend
// (.mm) is the real-native twin verified separately.
#include "maui/controls/search_bar.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/core/font.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_search_bar.hpp"
#include "maui/core/return_type.hpp"
#include "maui/core/search_bar_handler.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::search_bar;
    using maui::core::i_element_handler;
    using maui::core::i_search_bar;
    using maui::core::return_type;
    using maui::core::search_bar_handler;
    using maui::core::text_alignment;

    // ---- the control in isolation ----

    // SearchBarUnitTests.TestConstructor: Placeholder and Text are unset (empty in the string port).
    TEST(search_bar, constructor_defaults)
    {
        search_bar control;
        EXPECT_EQ(control.text(), "");
        EXPECT_EQ(control.placeholder(), "");
        // SearchBar defaults: ReturnType.Search; default cancel/search-icon colors (platform default).
        EXPECT_EQ(control.return_type(), return_type::search);
        EXPECT_EQ(control.cancel_button_color(), maui::graphics::color{});
        EXPECT_EQ(control.search_icon_color(), maui::graphics::color{});
        EXPECT_EQ(control.horizontal_text_alignment(), text_alignment::start);
        EXPECT_EQ(control.vertical_text_alignment(), text_alignment::center);
    }

    // SearchBarUnitTests.TestContentsChanged: a programmatic Text set raises TextChanged.
    TEST(search_bar, programmatic_text_change_raises_text_changed)
    {
        search_bar control;
        bool thrown = false;
        control.text_changed.connect([&thrown](const std::string&, const std::string&) { thrown = true; });
        control.set_text("Foo");
        EXPECT_TRUE(thrown);
    }

    // SearchBarUnitTests.SearchBarTextChangedEventArgs: the event carries the (old, new) pair.
    TEST(search_bar, text_changed_event_args_carry_old_and_new)
    {
        search_bar control;
        control.set_text("Initial Text");

        std::vector<std::pair<std::string, std::string>> changes;
        control.text_changed.connect([&changes](const std::string& old_value, const std::string& new_value) {
            changes.emplace_back(old_value, new_value);
        });

        control.set_text("Text Changed");
        ASSERT_EQ(changes.size(), 1U);
        EXPECT_EQ(changes[0].first, "Initial Text");
        EXPECT_EQ(changes[0].second, "Text Changed");
    }

    // SearchBarUnitTests.TestSearchButtonPressed.
    TEST(search_bar, send_search_button_pressed_raises_event)
    {
        search_bar control;
        bool thrown = false;
        control.search_button_pressed.connect([&thrown] { thrown = true; });
        control.send_search_button_pressed();
        EXPECT_TRUE(thrown);
    }

    // SearchBarUnitTests.TestSearchCommandParameter, collapsed to the port's command stand-in: the
    // command runs before the event (C# executes SearchCommand then raises SearchButtonPressed).
    TEST(search_bar, search_command_runs_before_event)
    {
        search_bar control;
        std::vector<std::string> order;
        control.search_command = [&order] { order.emplace_back("command"); };
        control.search_button_pressed.connect([&order] { order.emplace_back("event"); });

        control.send_search_button_pressed();
        ASSERT_EQ(order.size(), 2U);
        EXPECT_EQ(order[0], "command");
        EXPECT_EQ(order[1], "event");
    }

    TEST(search_bar, set_text_truncates_to_max_length)
    {
        search_bar control;
        control.set_max_length(3);
        control.set_text("abcdef");
        EXPECT_EQ(control.text(), "abc");
    }

    TEST(search_bar, usable_through_interface_references)
    {
        search_bar control;
        control.set_text("query");
        i_search_bar& as_bar = control;
        EXPECT_EQ(as_bar.text(), "query");
        EXPECT_EQ(as_bar.return_type(), return_type::search);
    }

    // ---- the handler seam (control <-> handler <-> headless platform) ----

    TEST(search_bar_seam, attaching_handler_maps_initial_properties)
    {
        search_bar control;
        control.set_text("Start");
        control.set_placeholder("Search…");
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), &control);
        auto* platform = handler->typed_platform_view();
        EXPECT_EQ(platform->text, "Start");
        EXPECT_EQ(platform->placeholder, "Search…");
        EXPECT_EQ(platform->bar_return_type, return_type::search);
    }

    TEST(search_bar_seam, setting_properties_maps_to_platform)
    {
        search_bar control;
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        control.set_text("Changed");
        EXPECT_EQ(platform->text, "Changed");

        control.set_placeholder("Placeholder");
        EXPECT_EQ(platform->placeholder, "Placeholder");

        control.set_cancel_button_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        EXPECT_EQ(platform->cancel_button_color, maui::graphics::color(1.0F, 0.0F, 0.0F));

        control.set_search_icon_color(maui::graphics::color(0.0F, 0.0F, 1.0F));
        EXPECT_EQ(platform->search_icon_color, maui::graphics::color(0.0F, 0.0F, 1.0F));

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
        EXPECT_EQ(platform->bar_return_type, return_type::go);

        control.set_is_text_prediction_enabled(false);
        EXPECT_FALSE(platform->is_text_prediction_enabled);

        control.set_is_spell_check_enabled(false);
        EXPECT_FALSE(platform->is_spell_check_enabled);

        control.set_cursor_position(4);
        EXPECT_EQ(platform->cursor_position, 4);

        control.set_selection_length(2);
        EXPECT_EQ(platform->selection_length, 2);
    }

    TEST(search_bar_seam, simulated_native_edit_fires_text_changed)
    {
        search_bar control;
        auto handler = std::make_shared<search_bar_handler>();
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

    TEST(search_bar_seam, simulated_search_press_fires_event)
    {
        search_bar control;
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        int presses = 0;
        control.search_button_pressed.connect([&presses] { ++presses; });
        platform->on_search_button_pressed();
        EXPECT_EQ(presses, 1);
    }

    TEST(search_bar_seam, clearing_handler_disconnects)
    {
        search_bar control;
        auto handler = std::make_shared<search_bar_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(control.handler(), nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST(search_bar_seam, handler_resolved_from_default_registry)
    {
        // search_bar -> search_bar_handler is self-registered in search_bar.cpp (MAUI_REGISTER_HANDLER).
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<search_bar>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<search_bar_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        search_bar control;
        control.set_text("Registered");
        control.set_handler(handler);
        EXPECT_EQ(resolved->typed_platform_view()->text, "Registered");
    }
} // namespace
