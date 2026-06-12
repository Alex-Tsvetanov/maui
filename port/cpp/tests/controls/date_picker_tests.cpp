// Tests for the date_picker control + its headless handler seam. Ported from
// src/Controls/tests/Core.UnitTests/DatePickerUnitTest.cs (the Min/Max validation + the coercion
// ORDER — a Min/Max change clamps Date from inside the coercion, so the "date" notification fires
// BEFORE the bound's own — plus the DateSelected event theories). The seam block drives the headless
// date_picker_platform (the DatePickerExtensions.UpdateDate mirror + the on_done commit).
#include "maui/controls/date_picker.hpp"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "maui/core/date_picker_handler.hpp"
#include "maui/core/date_time.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::date_picker;
    using maui::core::date_picker_handler;
    using maui::core::date_time;
    using maui::core::i_element_handler;

    // ---- Minimum/Maximum validation (an invalid bound is silently rejected, like C#'s warning) ----

    TEST(date_picker, minimum_date_above_maximum_is_rejected)
    {
        date_picker picker;
        picker.set_minimum_date(date_time(1950, 1, 1));
        ASSERT_TRUE(picker.minimum_date().has_value());
        EXPECT_EQ(*picker.minimum_date(), date_time(1950, 1, 1));

        picker.set_minimum_date(date_time(2200, 1, 1)); // above the 2100-12-31 maximum -> invalid
        ASSERT_TRUE(picker.minimum_date().has_value());
        EXPECT_EQ(*picker.minimum_date(), date_time(1950, 1, 1));
    }

    TEST(date_picker, minimum_date_null_is_valid)
    {
        date_picker picker;
        picker.set_minimum_date(std::nullopt);
        EXPECT_FALSE(picker.minimum_date().has_value());
    }

    TEST(date_picker, maximum_date_below_minimum_is_rejected)
    {
        date_picker picker;
        picker.set_maximum_date(date_time(2050, 1, 1));
        ASSERT_TRUE(picker.maximum_date().has_value());
        EXPECT_EQ(*picker.maximum_date(), date_time(2050, 1, 1));

        picker.set_maximum_date(date_time(1800, 1, 1)); // below the 1900-1-1 minimum -> invalid
        ASSERT_TRUE(picker.maximum_date().has_value());
        EXPECT_EQ(*picker.maximum_date(), date_time(2050, 1, 1));
    }

    TEST(date_picker, maximum_date_null_is_valid)
    {
        date_picker picker;
        picker.set_maximum_date(std::nullopt);
        EXPECT_FALSE(picker.maximum_date().has_value());
    }

    // ---- the clamping ORDER: lowering a bound clamps Date from inside the bound's coercion, so the
    //      "date" change notifies BEFORE the bound's own change ----

    TEST(date_picker, lowering_maximum_clamps_date_first)
    {
        date_picker picker;
        picker.set_date(date_time(2050, 1, 1));
        ASSERT_TRUE(picker.date().has_value());
        EXPECT_EQ(*picker.date(), date_time(2050, 1, 1));

        bool date_changed = false;
        bool maximum_changed = false;
        picker.property_changed.connect([&](std::string_view name) {
            if (name == "maximum_date")
            {
                maximum_changed = true;
            }
            else if (name == "date")
            {
                date_changed = true;
                EXPECT_FALSE(maximum_changed); // Date clamps before MaximumDate notifies
            }
        });

        const date_time new_date(2000, 1, 1);
        picker.set_maximum_date(new_date);

        EXPECT_TRUE(maximum_changed);
        EXPECT_TRUE(date_changed);
        EXPECT_EQ(picker.maximum_date(), std::optional<date_time>(new_date));
        EXPECT_EQ(picker.date(), std::optional<date_time>(new_date));
        EXPECT_EQ(picker.maximum_date(), picker.date());
    }

    TEST(date_picker, raising_minimum_clamps_date_first)
    {
        date_picker picker;
        picker.set_date(date_time(1950, 1, 1));
        ASSERT_TRUE(picker.date().has_value());
        EXPECT_EQ(*picker.date(), date_time(1950, 1, 1));

        bool date_changed = false;
        bool minimum_changed = false;
        picker.property_changed.connect([&](std::string_view name) {
            if (name == "minimum_date")
            {
                minimum_changed = true;
            }
            else if (name == "date")
            {
                date_changed = true;
                EXPECT_FALSE(minimum_changed);
            }
        });

        const date_time new_date(2000, 1, 1);
        picker.set_minimum_date(new_date);

        EXPECT_TRUE(minimum_changed);
        EXPECT_TRUE(date_changed);
        EXPECT_EQ(picker.minimum_date(), std::optional<date_time>(new_date));
        EXPECT_EQ(picker.date(), std::optional<date_time>(new_date));
        EXPECT_EQ(picker.minimum_date(), picker.date());
    }

    TEST(date_picker, date_clamps_to_the_bounds)
    {
        date_picker picker;
        picker.set_date(date_time(1500, 1, 1));
        EXPECT_EQ(picker.date(), picker.minimum_date());

        picker.set_date(date_time(2500, 1, 1));
        EXPECT_EQ(picker.date(), picker.maximum_date());
    }

    // ---- DateSelected ----

    TEST(date_picker, date_selected_raises_on_change)
    {
        date_picker picker;
        bool selected = false;
        picker.date_selected.connect(
            [&selected](const std::optional<date_time>&, const std::optional<date_time>&) { selected = true; });

        picker.set_date(date_time(2008, 5, 5));
        EXPECT_TRUE(selected);
    }

    TEST(date_picker, date_selected_carries_old_and_new_dates)
    {
        struct case_t
        {
            std::optional<date_time> initial_value;
            std::optional<date_time> final_value;
        };
        const case_t cases[] = {
            {date_time(2006, 12, 20), date_time(2011, 11, 30)},
            {date_time(1900, 1, 1), date_time(1999, 1, 15)},    // minimum date
            {date_time(2006, 12, 20), date_time(2100, 12, 31)}, // maximum date
            {date_time(2006, 12, 20), std::nullopt},
            {std::nullopt, date_time(2006, 12, 20)},
        };
        for (const auto& test_case : cases)
        {
            date_picker picker;
            picker.set_date(test_case.initial_value);

            std::optional<date_time> old_date = date_time();
            std::optional<date_time> new_date = date_time();
            picker.date_selected.connect(
                [&](const std::optional<date_time>& old_value, const std::optional<date_time>& new_value) {
                    old_date = old_value;
                    new_date = new_value;
                });

            picker.set_date(test_case.final_value);

            EXPECT_EQ(old_date, test_case.initial_value);
            EXPECT_EQ(new_date, test_case.final_value);
        }
    }

    TEST(date_picker, date_selected_triggers_only_on_a_real_change)
    {
        struct case_t
        {
            std::optional<date_time> initial_value;
            std::optional<date_time> final_value;
            bool should_trigger;
        };
        const case_t cases[] = {
            {date_time(2006, 12, 20), date_time(2011, 11, 30), true},
            {date_time(1900, 1, 1), date_time(1999, 1, 15), true},
            {date_time(2006, 12, 20), date_time(2100, 12, 31), true},
            {date_time(2006, 12, 20), std::nullopt, true},
            {std::nullopt, date_time(2006, 12, 20), true},
            {date_time(2006, 12, 20), date_time(2006, 12, 20), false},
            {std::nullopt, std::nullopt, false},
        };
        for (const auto& test_case : cases)
        {
            date_picker picker;
            picker.set_date(test_case.initial_value);

            bool triggered = false;
            picker.date_selected.connect(
                [&triggered](const std::optional<date_time>&, const std::optional<date_time>&) { triggered = true; });

            picker.set_date(test_case.final_value);
            EXPECT_EQ(triggered, test_case.should_trigger);
        }
    }

    // ---- null handling + defaults ----

    TEST(date_picker, set_null_date_does_not_throw)
    {
        date_picker picker;
        picker.set_date(std::nullopt);
        EXPECT_FALSE(picker.date().has_value());
    }

    TEST(date_picker, set_nullable_date_stores_the_value)
    {
        date_picker picker;
        const date_time value(2015, 7, 21);
        picker.set_date(std::optional<date_time>(value));
        EXPECT_EQ(picker.date(), std::optional<date_time>(value));
    }

    TEST(date_picker, max_and_min_can_both_be_set_to_now)
    {
        // xamarin/Xamarin.Forms#5784: both bounds truncate to .Date, so today's time-of-day never
        // makes the pair mutually invalid.
        date_picker picker;
        picker.set_maximum_date(date_time::now());
        picker.set_minimum_date(date_time::now());
        EXPECT_EQ(picker.minimum_date(), std::optional<date_time>(date_time::today()));
        EXPECT_EQ(picker.maximum_date(), std::optional<date_time>(date_time::today()));
    }

    TEST(date_picker, defaults_match_the_csharp_descriptors)
    {
        date_picker picker;
        EXPECT_EQ(picker.format(), "d");
        EXPECT_EQ(picker.minimum_date(), std::optional<date_time>(date_time(1900, 1, 1)));
        EXPECT_EQ(picker.maximum_date(), std::optional<date_time>(date_time(2100, 12, 31)));
        EXPECT_EQ(picker.date(), std::optional<date_time>(date_time::today())); // defaultValueCreator
    }

    TEST(date_picker, date_truncates_the_time_of_day)
    {
        date_picker picker;
        picker.set_date(date_time{date_time(2015, 7, 21).days(), std::chrono::hours{13}});
        EXPECT_EQ(picker.date(), std::optional<date_time>(date_time(2015, 7, 21)));
    }

    // ---- the headless handler seam (the DatePickerExtensions.UpdateDate mirror + on_done) ----

    TEST(date_picker_handler_seam, attaching_handler_mirrors_date_bounds_and_text)
    {
        date_picker picker;
        picker.set_date(date_time(2008, 5, 5));
        auto handler = std::make_shared<date_picker_handler>();
        picker.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->date, date_time(2008, 5, 5));
        EXPECT_EQ(platform->minimum_date, std::optional<date_time>(date_time(1900, 1, 1)));
        EXPECT_EQ(platform->maximum_date, std::optional<date_time>(date_time(2100, 12, 31)));
        EXPECT_EQ(platform->text, "5/5/2008"); // the "d" default through the invariant short date
    }

    TEST(date_picker_handler_seam, format_change_rerenders_the_text)
    {
        date_picker picker;
        picker.set_date(date_time(2008, 5, 5));
        auto handler = std::make_shared<date_picker_handler>();
        picker.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        picker.set_format("D");
        EXPECT_EQ(platform->text, "Monday, May 5, 2008");

        picker.set_format("yyyy-MM-dd");
        EXPECT_EQ(platform->text, "2008-05-05");
    }

    TEST(date_picker_handler_seam, null_date_renders_empty_text)
    {
        date_picker picker;
        picker.set_date(std::nullopt);
        auto handler = std::make_shared<date_picker_handler>();
        picker.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->text, "");
        EXPECT_EQ(platform->date, date_time::today()); // the native wheel falls back to Today
    }

    TEST(date_picker_handler_seam, native_done_commits_the_picked_date)
    {
        date_picker picker;
        auto handler = std::make_shared<date_picker_handler>();
        picker.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        bool selected = false;
        picker.date_selected.connect(
            [&selected](const std::optional<date_time>&, const std::optional<date_time>&) { selected = true; });

        platform->date = date_time(2011, 11, 30); // the user spins the native wheel...
        platform->on_done();                      // ...and taps Done (OnDoneClicked -> SetVirtualViewDate)
        EXPECT_EQ(picker.date(), std::optional<date_time>(date_time(2011, 11, 30)));
        EXPECT_TRUE(selected);
        EXPECT_EQ(platform->text, "11/30/2011");
    }

    TEST(date_picker_handler_seam, committed_date_is_clamped_by_the_control)
    {
        date_picker picker;
        picker.set_maximum_date(date_time(2050, 1, 1));
        auto handler = std::make_shared<date_picker_handler>();
        picker.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        platform->date = date_time(2099, 1, 1);
        platform->on_done();
        EXPECT_EQ(picker.date(), std::optional<date_time>(date_time(2050, 1, 1)));
    }

    TEST(date_picker_handler_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<date_picker>();
        ASSERT_NE(handler, nullptr);
        EXPECT_NE(dynamic_cast<date_picker_handler*>(handler.get()), nullptr);
    }
} // namespace
