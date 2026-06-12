// Apple (AppKit) backend tests for the time_picker seam — run only for MAUI_BACKEND=apple. Drives a
// genuine NSDatePicker (hour/minute elements, UTC): Time maps onto dateValue anchored on the epoch
// day, and a native edit (set dateValue + send the wired target-action, no run loop needed) flows
// back through the handler with the SECONDS DROPPED (SetVirtualViewTime). Compiled as Objective-C++
// with ARC.
#import <AppKit/AppKit.h>

#include <memory>
#include <optional>

#include "maui/controls/time_picker.hpp"
#include "maui/core/date_time.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/time_picker_handler.hpp"
#include "maui/core/visibility.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::time_picker;
    using maui::core::i_element_handler;
    using maui::core::time_picker_handler;
    using maui::core::time_span;

    NSDatePicker* native_picker(const std::shared_ptr<time_picker_handler>& handler)
    {
        return (__bridge NSDatePicker*)handler->typed_platform_view()->native;
    }

    // The UTC hour/minute/second components of an NSDate (the picker runs on the UTC time zone).
    NSDateComponents* utc_components(NSDate* date)
    {
        NSCalendar* const calendar = [NSCalendar calendarWithIdentifier:NSCalendarIdentifierGregorian];
        calendar.timeZone = [NSTimeZone timeZoneWithName:@"UTC"];
        return [calendar components:NSCalendarUnitHour | NSCalendarUnitMinute | NSCalendarUnitSecond fromDate:date];
    }

    NSDate* utc_time(int hour, int minute, int second)
    {
        NSDateComponents* const components = [[NSDateComponents alloc] init];
        components.year = 1970;
        components.month = 1;
        components.day = 1;
        components.hour = hour;
        components.minute = minute;
        components.second = second;
        NSCalendar* const calendar = [NSCalendar calendarWithIdentifier:NSCalendarIdentifierGregorian];
        calendar.timeZone = [NSTimeZone timeZoneWithName:@"UTC"];
        return [calendar dateFromComponents:components];
    }

    class apple_time_picker_seam : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            [NSApplication sharedApplication];
        }
    };

    TEST_F(apple_time_picker_seam, attaching_handler_maps_the_time)
    {
        time_picker control;
        control.set_time(time_span(17, 30, 0));
        auto handler = std::make_shared<time_picker_handler>();
        control.set_handler(handler);

        NSDatePicker* const native = native_picker(handler);
        ASSERT_NE(native, nil);
        NSDateComponents* const components = utc_components(native.dateValue);
        EXPECT_EQ(components.hour, 17);
        EXPECT_EQ(components.minute, 30);
    }

    TEST_F(apple_time_picker_seam, setting_time_updates_the_native_value)
    {
        time_picker control;
        auto handler = std::make_shared<time_picker_handler>();
        control.set_handler(handler);

        control.set_time(time_span(8, 5, 0));
        NSDateComponents* const components = utc_components(native_picker(handler).dateValue);
        EXPECT_EQ(components.hour, 8);
        EXPECT_EQ(components.minute, 5);
    }

    TEST_F(apple_time_picker_seam, null_time_falls_back_to_zero_on_the_native_wheel)
    {
        time_picker control;
        control.set_time(std::nullopt);
        auto handler = std::make_shared<time_picker_handler>();
        control.set_handler(handler);

        NSDateComponents* const components = utc_components(native_picker(handler).dateValue);
        EXPECT_EQ(components.hour, 0);
        EXPECT_EQ(components.minute, 0);
    }

    TEST_F(apple_time_picker_seam, native_edit_flows_back_dropping_seconds)
    {
        time_picker control;
        auto handler = std::make_shared<time_picker_handler>();
        control.set_handler(handler);

        int selected = 0;
        control.time_selected.connect(
            [&selected](const std::optional<time_span>&, const std::optional<time_span>&) { ++selected; });

        NSDatePicker* const native = native_picker(handler);
        native.dateValue = utc_time(9, 45, 30); // the user edits the field...
        [native sendAction:native.action to:native.target];

        EXPECT_EQ(control.time(), std::optional<time_span>(time_span(9, 45, 0))); // seconds dropped
        EXPECT_EQ(selected, 1);
    }

    TEST_F(apple_time_picker_seam, generic_iview_properties_reach_the_native_picker)
    {
        time_picker control;
        auto handler = std::make_shared<time_picker_handler>();
        control.set_handler(handler);
        NSDatePicker* const native = native_picker(handler);

        control.set_is_enabled(false);
        EXPECT_FALSE(native.enabled);

        control.set_visibility(maui::core::visibility::hidden);
        EXPECT_TRUE(native.hidden);

        control.set_opacity(0.5);
        EXPECT_EQ(native.alphaValue, 0.5);
    }

    TEST_F(apple_time_picker_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<time_picker>();
        ASSERT_NE(handler, nullptr);
        EXPECT_NE(dynamic_cast<time_picker_handler*>(handler.get()), nullptr);
    }
} // namespace
