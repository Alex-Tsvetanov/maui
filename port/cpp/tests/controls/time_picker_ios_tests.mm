// iOS (UIKit) backend tests for the time_picker seam — run only for MAUI_BACKEND=ios (executed ON
// the iOS simulator via tools/ios-sim-run.sh). Drives the real MauiTimePicker shape: a UITextField
// whose inputView is a time-mode UIDatePicker on UTC — Time maps onto the wheel, the formatted text
// renders into the field (the invariant/en-US time formatter), and the Done commit
// (SetVirtualViewTime) flows back through the portable on_done channel with the SECONDS DROPPED.
// Compiled as Objective-C++ with ARC.
#import <UIKit/UIKit.h>

#include <memory>
#include <optional>
#include <string>

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

    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    UITextField* native_field(const std::shared_ptr<time_picker_handler>& handler)
    {
        return (__bridge UITextField*)handler->typed_platform_view()->native;
    }

    UIDatePicker* wheel_of(UITextField* field)
    {
        return (UIDatePicker*)field.inputView;
    }

    // Replicates -[UIControl sendActionsForControlEvents:]'s dispatch-table walk for one event (as in
    // button_ios_tests.mm): no key window in the spawned process drives BecomeFirstResponder, so invoke
    // every (target, action) pair registered for `event` with the control as sender — the exact path
    // the native callback takes on a device.
    void send_control_event(UIControl* control, UIControlEvents event)
    {
        NSArray* const targets = control.allTargets.allObjects;
        for (NSUInteger t = 0; t < targets.count; ++t)
        {
            id const target = targets[t];
            NSArray<NSString*>* const actions = [control actionsForTarget:target forControlEvent:event];
            for (NSUInteger a = 0; a < actions.count; ++a)
            {
                SEL const action = NSSelectorFromString(actions[a]);
                NSMethodSignature* const signature = [target methodSignatureForSelector:action];
                ASSERT_NE(signature, nil);
                NSInvocation* const invocation = [NSInvocation invocationWithMethodSignature:signature];
                invocation.selector = action;
                id sender = control;
                [invocation setArgument:&sender atIndex:2]; // 0 = self, 1 = _cmd, 2 = the sender
                [invocation invokeWithTarget:target];
            }
        }
    }

    NSDateComponents* utc_components(NSDate* date)
    {
        NSCalendar* const calendar = [NSCalendar calendarWithIdentifier:NSCalendarIdentifierGregorian];
        calendar.timeZone = [NSTimeZone timeZoneForSecondsFromGMT:0]; // nonnull (UTC)
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
        calendar.timeZone = [NSTimeZone timeZoneForSecondsFromGMT:0]; // nonnull (UTC)
        NSDate* const result = [calendar dateFromComponents:components];
        EXPECT_NE(result, nil); // dateFromComponents: is nullable; the fixed components always resolve
        return result != nil ? result : NSDate.distantPast;
    }

    TEST(ios_time_picker_seam, attaching_handler_maps_time_and_text)
    {
        time_picker control;
        control.set_time(time_span(17, 30, 0));
        auto handler = std::make_shared<time_picker_handler>();
        control.set_handler(handler);

        UITextField* const field = native_field(handler);
        ASSERT_NE(field, nil);
        EXPECT_EQ(field.borderStyle, UITextBorderStyleRoundedRect);
        ASSERT_TRUE([field.inputView isKindOfClass:[UIDatePicker class]]);
        EXPECT_NE(field.inputAccessoryView, nil); // the Done toolbar

        UIDatePicker* const wheel = wheel_of(field);
        EXPECT_EQ(wheel.datePickerMode, UIDatePickerModeTime);
        NSDateComponents* const components = utc_components(wheel.date);
        EXPECT_EQ(components.hour, 17);
        EXPECT_EQ(components.minute, 30);
        EXPECT_EQ(to_std_string(field.text), "5:30 PM"); // the "t" default in the en-US lean
    }

    TEST(ios_time_picker_seam, format_change_rerenders_the_field_text)
    {
        time_picker control;
        control.set_time(time_span(17, 30, 0));
        auto handler = std::make_shared<time_picker_handler>();
        control.set_handler(handler);

        control.set_format("HH:mm");
        EXPECT_EQ(to_std_string(native_field(handler).text), "17:30");
    }

    TEST(ios_time_picker_seam, null_time_renders_empty_and_falls_back_to_zero)
    {
        time_picker control;
        control.set_time(std::nullopt);
        auto handler = std::make_shared<time_picker_handler>();
        control.set_handler(handler);

        UITextField* const field = native_field(handler);
        EXPECT_EQ(field.text.length, 0U);
        NSDateComponents* const components = utc_components(wheel_of(field).date);
        EXPECT_EQ(components.hour, 0);
        EXPECT_EQ(components.minute, 0);
    }

    TEST(ios_time_picker_seam, done_commits_hours_and_minutes_dropping_seconds)
    {
        time_picker control;
        auto handler = std::make_shared<time_picker_handler>();
        control.set_handler(handler);

        int selected = 0;
        control.time_selected.connect(
            [&selected](const std::optional<time_span>&, const std::optional<time_span>&) { ++selected; });

        UITextField* const field = native_field(handler);
        wheel_of(field).date = utc_time(9, 45, 30); // the user spins the wheel...
        handler->typed_platform_view()->on_done();  // ...and taps Done

        EXPECT_EQ(control.time(), std::optional<time_span>(time_span(9, 45, 0))); // seconds dropped
        EXPECT_EQ(selected, 1);
        EXPECT_EQ(to_std_string(field.text), "9:45 AM");
    }

    TEST(ios_time_picker_seam, generic_iview_properties_reach_the_field)
    {
        time_picker control;
        auto handler = std::make_shared<time_picker_handler>();
        control.set_handler(handler);
        UITextField* const field = native_field(handler);

        control.set_is_enabled(false);
        EXPECT_FALSE(field.enabled);

        control.set_visibility(maui::core::visibility::hidden);
        EXPECT_TRUE(field.hidden);

        control.set_opacity(0.5);
        EXPECT_NEAR(field.alpha, 0.5, 0.001);
    }

    // ---- the IsOpen focus dance (TimePickerHandler.iOS.cs OnStarted/OnEnded) ----

    TEST(ios_time_picker_seam, native_editing_begin_sets_is_open_and_is_focused)
    {
        time_picker control;
        auto handler = std::make_shared<time_picker_handler>();
        control.set_handler(handler);

        int opened = 0;
        control.opened.connect([&opened] { ++opened; });

        send_control_event(native_field(handler), UIControlEventEditingDidBegin);
        EXPECT_TRUE(control.is_open());
        EXPECT_TRUE(control.is_focused());
        EXPECT_EQ(opened, 1);
    }

    TEST(ios_time_picker_seam, native_editing_end_clears_is_open_and_is_focused)
    {
        time_picker control;
        auto handler = std::make_shared<time_picker_handler>();
        control.set_handler(handler);
        UITextField* const field = native_field(handler);

        send_control_event(field, UIControlEventEditingDidBegin);
        int closed = 0;
        control.closed.connect([&closed] { ++closed; });

        send_control_event(field, UIControlEventEditingDidEnd);
        EXPECT_FALSE(control.is_open());
        EXPECT_FALSE(control.is_focused());
        EXPECT_EQ(closed, 1);
    }

    TEST(ios_time_picker_seam, programmatic_is_open_invokes_map_is_open_without_crashing)
    {
        time_picker control;
        auto handler = std::make_shared<time_picker_handler>();
        control.set_handler(handler);

        control.set_is_open(true); // MapIsOpen → BecomeFirstResponder (NO in the spawned process)
        EXPECT_TRUE(control.is_open());
        control.set_is_open(false); // MapIsOpen → ResignFirstResponder
        EXPECT_FALSE(control.is_open());
    }

    TEST(ios_time_picker_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<time_picker>();
        ASSERT_NE(handler, nullptr);
        EXPECT_NE(dynamic_cast<time_picker_handler*>(handler.get()), nullptr);
    }
} // namespace
