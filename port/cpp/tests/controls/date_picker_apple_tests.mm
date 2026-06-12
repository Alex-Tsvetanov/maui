// Apple (AppKit) backend tests for the date_picker seam — run only for MAUI_BACKEND=apple. Drives a
// genuine NSDatePicker (year/month/day elements, UTC): Date/Minimum/MaximumDate map onto
// dateValue/minDate/maxDate, and a native edit (set dateValue + send the wired target-action, no run
// loop needed) flows back through the handler into the control — whose own coercion clamps it.
// Compiled as Objective-C++ with ARC.
#import <AppKit/AppKit.h>

#include <memory>
#include <optional>
#include <string>

#include "maui/controls/date_picker.hpp"
#include "maui/core/date_picker_handler.hpp"
#include "maui/core/date_time.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/visibility.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::date_picker;
    using maui::core::date_picker_handler;
    using maui::core::date_time;
    using maui::core::i_element_handler;

    NSDatePicker* native_picker(const std::shared_ptr<date_picker_handler>& handler)
    {
        return (__bridge NSDatePicker*)handler->typed_platform_view()->native;
    }

    // The UTC components of an NSDate (the picker runs on the UTC time zone).
    NSDateComponents* utc_components(NSDate* date)
    {
        NSCalendar* const calendar = [NSCalendar calendarWithIdentifier:NSCalendarIdentifierGregorian];
        calendar.timeZone = [NSTimeZone timeZoneWithName:@"UTC"];
        return [calendar components:NSCalendarUnitYear | NSCalendarUnitMonth | NSCalendarUnitDay fromDate:date];
    }

    NSDate* utc_date(int year, int month, int day)
    {
        NSDateComponents* const components = [[NSDateComponents alloc] init];
        components.year = year;
        components.month = month;
        components.day = day;
        NSCalendar* const calendar = [NSCalendar calendarWithIdentifier:NSCalendarIdentifierGregorian];
        calendar.timeZone = [NSTimeZone timeZoneWithName:@"UTC"];
        return [calendar dateFromComponents:components];
    }

    class apple_date_picker_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_date_picker_seam, attaching_handler_maps_date_and_bounds)
    {
        date_picker control;
        control.set_date(date_time(2008, 5, 5));
        auto handler = std::make_shared<date_picker_handler>();
        control.set_handler(handler);

        NSDatePicker* const native = native_picker(handler);
        ASSERT_NE(native, nil);
        NSDateComponents* const components = utc_components(native.dateValue);
        EXPECT_EQ(components.year, 2008);
        EXPECT_EQ(components.month, 5);
        EXPECT_EQ(components.day, 5);
        // The default bounds (1900-01-01 / 2100-12-31) land on minDate/maxDate.
        EXPECT_TRUE([native.minDate isEqualToDate:utc_date(1900, 1, 1)]);
        EXPECT_TRUE([native.maxDate isEqualToDate:utc_date(2100, 12, 31)]);
    }

    TEST_F(apple_date_picker_seam, setting_date_updates_the_native_value)
    {
        date_picker control;
        auto handler = std::make_shared<date_picker_handler>();
        control.set_handler(handler);

        control.set_date(date_time(2011, 11, 30));
        NSDateComponents* const components = utc_components(native_picker(handler).dateValue);
        EXPECT_EQ(components.year, 2011);
        EXPECT_EQ(components.month, 11);
        EXPECT_EQ(components.day, 30);
    }

    TEST_F(apple_date_picker_seam, null_date_falls_back_to_today_on_the_native_wheel)
    {
        date_picker control;
        control.set_date(std::nullopt);
        auto handler = std::make_shared<date_picker_handler>();
        control.set_handler(handler);

        const date_time today = date_time::today();
        NSDateComponents* const components = utc_components(native_picker(handler).dateValue);
        EXPECT_EQ(components.year, today.year());
        EXPECT_EQ(static_cast<unsigned>(components.month), today.month());
        EXPECT_EQ(static_cast<unsigned>(components.day), today.day());
    }

    TEST_F(apple_date_picker_seam, native_edit_flows_back_and_is_clamped)
    {
        date_picker control;
        control.set_maximum_date(date_time(2050, 1, 1));
        auto handler = std::make_shared<date_picker_handler>();
        control.set_handler(handler);

        bool selected = false;
        control.date_selected.connect(
            [&selected](const std::optional<date_time>&, const std::optional<date_time>&) { selected = true; });

        NSDatePicker* const native = native_picker(handler);
        native.dateValue = utc_date(2020, 6, 15); // the user edits the field...
        [native sendAction:native.action to:native.target];

        EXPECT_TRUE(selected);
        EXPECT_EQ(control.date(), std::optional<date_time>(date_time(2020, 6, 15)));
    }

    TEST_F(apple_date_picker_seam, generic_iview_properties_reach_the_native_picker)
    {
        date_picker control;
        auto handler = std::make_shared<date_picker_handler>();
        control.set_handler(handler);
        NSDatePicker* const native = native_picker(handler);

        control.set_is_enabled(false);
        EXPECT_FALSE(native.enabled);

        control.set_visibility(maui::core::visibility::hidden);
        EXPECT_TRUE(native.hidden);

        control.set_opacity(0.5);
        EXPECT_EQ(native.alphaValue, 0.5);
    }

    TEST_F(apple_date_picker_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<date_picker>();
        ASSERT_NE(handler, nullptr);
        EXPECT_NE(dynamic_cast<date_picker_handler*>(handler.get()), nullptr);
    }
} // namespace
