// Tests for the time_picker control + its headless handler seam. Ported from
// src/Controls/tests/Core.UnitTests/TimePickerUnitTest.cs (the [0, 24h) validation that silently
// rejects out-of-range values, the zero/null special cases, and the TimeSelected theories). The seam
// block drives the headless time_picker_platform (the TimePickerExtensions.UpdateTime mirror + the
// on_done commit, which drops seconds exactly like TimePickerHandler.SetVirtualViewTime).
#include "maui/controls/time_picker.hpp"

#include <array>
#include <memory>
#include <optional>
#include <string>

#include "maui/core/binding_mode.hpp"
#include "maui/core/date_time.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/time_picker_handler.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::time_picker;
    using maui::core::i_element_handler;
    using maui::core::time_picker_handler;
    using maui::core::time_span;

    // ---- defaults + validation ----

    TEST(time_picker, constructs_with_time_zero)
    {
        time_picker picker;
        EXPECT_EQ(picker.time(), std::optional<time_span>(time_span()));
        EXPECT_EQ(picker.format(), "t");
    }

    TEST(time_picker, out_of_range_time_is_silently_rejected)
    {
        time_picker picker;
        picker.set_time(time_span(1000, 0, 0)); // >= 24h -> invalid (C# logs a warning and keeps the value)
        EXPECT_EQ(picker.time(), std::optional<time_span>(time_span()));

        picker.set_time(time_span(8, 30, 0));
        EXPECT_EQ(picker.time(), std::optional<time_span>(time_span(8, 30, 0)));

        picker.set_time(time_span(-1, 0, 0)); // negative -> invalid
        EXPECT_EQ(picker.time(), std::optional<time_span>(time_span(8, 30, 0)));
    }

    TEST(time_picker, zero_time_is_valid)
    {
        time_picker picker;
        picker.set_time(time_span(0, 0, 0)); // issue #745
        EXPECT_EQ(picker.time(), std::optional<time_span>(time_span()));
    }

    TEST(time_picker, null_time_is_valid)
    {
        time_picker picker;
        picker.set_time(std::nullopt);
        EXPECT_FALSE(picker.time().has_value());
    }

    // ---- TimeSelected ----

    TEST(time_picker, time_selected_raises_once_per_change)
    {
        time_picker picker;
        int selected = 0;
        picker.time_selected.connect(
            [&selected](const std::optional<time_span>&, const std::optional<time_span>&) { ++selected; });

        picker.set_time(time_span(12, 30, 15));
        EXPECT_EQ(selected, 1);
    }

    TEST(time_picker, time_selected_carries_old_and_new_times)
    {
        struct case_t
        {
            std::optional<time_span> initial_value;
            std::optional<time_span> final_value;
        };
        const std::array cases{
            case_t{.initial_value = time_span(), .final_value = time_span(9, 0, 0)},
            case_t{.initial_value = time_span(9, 0, 0), .final_value = time_span(17, 30, 0)},
            case_t{.initial_value = time_span(23, 59, 59), .final_value = time_span(0, 0, 0)},
            case_t{.initial_value = time_span(23, 59, 59), .final_value = std::nullopt},
            case_t{.initial_value = std::nullopt, .final_value = time_span(23, 59, 59)},
        };
        for (const auto& test_case : cases)
        {
            time_picker picker;
            picker.set_time(test_case.initial_value);

            std::optional<time_span> old_time = time_span();
            std::optional<time_span> new_time = time_span();
            picker.time_selected.connect(
                [&](const std::optional<time_span>& old_value, const std::optional<time_span>& new_value) {
                    old_time = old_value;
                    new_time = new_value;
                });

            picker.set_time(test_case.final_value);

            EXPECT_EQ(old_time, test_case.initial_value);
            EXPECT_EQ(new_time, test_case.final_value);
        }
    }

    // ---- IsOpen + Opened/Closed (TimePicker.IsOpenProperty / OnIsOpenPropertyChanged) ----

    TEST(time_picker, is_open_default_false)
    {
        const time_picker picker;
        EXPECT_FALSE(picker.is_open());
    }

    TEST(time_picker, is_open_property_is_two_way)
    {
        EXPECT_EQ(time_picker::is_open_property().default_binding_mode(), maui::core::binding_mode::two_way);
    }

    TEST(time_picker, set_is_open_true_raises_opened)
    {
        time_picker picker;
        int opened = 0;
        int closed = 0;
        bool open_when_raised = false;
        picker.opened.connect([&] {
            ++opened;
            open_when_raised = picker.is_open(); // the value is stored before Opened fires
        });
        picker.closed.connect([&] { ++closed; });

        picker.set_is_open(true);
        EXPECT_EQ(opened, 1);
        EXPECT_EQ(closed, 0);
        EXPECT_TRUE(picker.is_open());
        EXPECT_TRUE(open_when_raised);

        picker.set_is_open(true); // unchanged -> silent
        EXPECT_EQ(opened, 1);
    }

    TEST(time_picker, set_is_open_false_raises_closed)
    {
        time_picker picker;
        picker.set_is_open(true);
        int opened = 0;
        int closed = 0;
        picker.opened.connect([&] { ++opened; });
        picker.closed.connect([&] { ++closed; });

        picker.set_is_open(false);
        EXPECT_EQ(closed, 1);
        EXPECT_EQ(opened, 0);
        EXPECT_FALSE(picker.is_open());
    }

    // ---- the headless handler seam (the TimePickerExtensions.UpdateTime mirror + on_done) ----

    TEST(time_picker_handler_seam, attaching_handler_mirrors_time_and_text)
    {
        time_picker picker;
        picker.set_time(time_span(17, 30, 0));
        auto handler = std::make_shared<time_picker_handler>();
        picker.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->time, time_span(17, 30, 0));
        EXPECT_EQ(platform->text, "5:30 PM"); // the "t" default in the en-US lean the iOS oracle hardcodes
    }

    TEST(time_picker_handler_seam, format_change_rerenders_the_text)
    {
        time_picker picker;
        picker.set_time(time_span(17, 30, 0));
        auto handler = std::make_shared<time_picker_handler>();
        picker.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        picker.set_format("HH:mm");
        EXPECT_EQ(platform->text, "17:30");
    }

    TEST(time_picker_handler_seam, null_time_renders_empty_text)
    {
        time_picker picker;
        picker.set_time(std::nullopt);
        auto handler = std::make_shared<time_picker_handler>();
        picker.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->text, "");
        EXPECT_EQ(platform->time, time_span()); // the native wheel falls back to zero
    }

    TEST(time_picker_handler_seam, native_done_commits_hours_and_minutes_dropping_seconds)
    {
        time_picker picker;
        auto handler = std::make_shared<time_picker_handler>();
        picker.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        int selected = 0;
        picker.time_selected.connect(
            [&selected](const std::optional<time_span>&, const std::optional<time_span>&) { ++selected; });

        platform->time = time_span(9, 45, 30); // the user spins the native wheel...
        platform->on_done();                   // ...and taps Done (SetVirtualViewTime drops the seconds)
        EXPECT_EQ(picker.time(), std::optional<time_span>(time_span(9, 45, 0)));
        EXPECT_EQ(selected, 1);
        EXPECT_EQ(platform->text, "9:45 AM");
    }

    TEST(time_picker_handler_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<time_picker>();
        ASSERT_NE(handler, nullptr);
        EXPECT_NE(dynamic_cast<time_picker_handler*>(handler.get()), nullptr);
    }
} // namespace
