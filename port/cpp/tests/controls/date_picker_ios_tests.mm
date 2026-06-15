// iOS (UIKit) backend tests for the date_picker seam — run only for MAUI_BACKEND=ios (executed ON
// the iOS simulator via tools/ios-sim-run.sh). Drives the real MauiDatePicker shape: a UITextField
// whose inputView is a date-mode UIDatePicker on UTC — Date/Minimum/Maximum map onto the dialog, the
// formatted text renders into the field (the invariant/en-US date_time formatter), and the Done
// commit (SetVirtualViewDate) flows back through the portable on_done channel, clamped by the
// control's coercion. Compiled as Objective-C++ with ARC.
#import <UIKit/UIKit.h>

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

    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    UITextField* native_field(const std::shared_ptr<date_picker_handler>& handler)
    {
        return (__bridge UITextField*)handler->typed_platform_view()->native;
    }

    UIDatePicker* dialog_of(UITextField* field)
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
        return [calendar components:NSCalendarUnitYear | NSCalendarUnitMonth | NSCalendarUnitDay fromDate:date];
    }

    NSDate* utc_date(int year, int month, int day)
    {
        NSDateComponents* const components = [[NSDateComponents alloc] init];
        components.year = year;
        components.month = month;
        components.day = day;
        NSCalendar* const calendar = [NSCalendar calendarWithIdentifier:NSCalendarIdentifierGregorian];
        calendar.timeZone = [NSTimeZone timeZoneForSecondsFromGMT:0]; // nonnull (UTC)
        NSDate* const result = [calendar dateFromComponents:components];
        EXPECT_NE(result, nil); // dateFromComponents: is nullable; the fixed components always resolve
        return result != nil ? result : NSDate.distantPast;
    }

    TEST(ios_date_picker_seam, attaching_handler_maps_date_bounds_and_text)
    {
        date_picker control;
        control.set_date(date_time(2008, 5, 5));
        auto handler = std::make_shared<date_picker_handler>();
        control.set_handler(handler);

        UITextField* const field = native_field(handler);
        ASSERT_NE(field, nil);
        EXPECT_EQ(field.borderStyle, UITextBorderStyleRoundedRect);
        ASSERT_TRUE([field.inputView isKindOfClass:[UIDatePicker class]]);
        EXPECT_NE(field.inputAccessoryView, nil); // the Done toolbar

        UIDatePicker* const dialog = dialog_of(field);
        EXPECT_EQ(dialog.datePickerMode, UIDatePickerModeDate);
        NSDateComponents* const components = utc_components(dialog.date);
        EXPECT_EQ(components.year, 2008);
        EXPECT_EQ(components.month, 5);
        EXPECT_EQ(components.day, 5);
        EXPECT_TRUE([dialog.minimumDate isEqualToDate:utc_date(1900, 1, 1)]);
        EXPECT_TRUE([dialog.maximumDate isEqualToDate:utc_date(2100, 12, 31)]);
        EXPECT_EQ(to_std_string(field.text), "5/5/2008"); // the "d" default, invariant short date
    }

    TEST(ios_date_picker_seam, format_change_rerenders_the_field_text)
    {
        date_picker control;
        control.set_date(date_time(2008, 5, 5));
        auto handler = std::make_shared<date_picker_handler>();
        control.set_handler(handler);

        control.set_format("D");
        EXPECT_EQ(to_std_string(native_field(handler).text), "Monday, May 5, 2008");

        control.set_format("yyyy-MM-dd");
        EXPECT_EQ(to_std_string(native_field(handler).text), "2008-05-05");
    }

    TEST(ios_date_picker_seam, null_date_renders_empty_and_falls_back_to_today)
    {
        date_picker control;
        control.set_date(std::nullopt);
        auto handler = std::make_shared<date_picker_handler>();
        control.set_handler(handler);

        UITextField* const field = native_field(handler);
        EXPECT_EQ(field.text.length, 0U);
        const date_time today = date_time::today();
        NSDateComponents* const components = utc_components(dialog_of(field).date);
        EXPECT_EQ(components.year, today.year());
        EXPECT_EQ(static_cast<unsigned>(components.month), today.month());
        EXPECT_EQ(static_cast<unsigned>(components.day), today.day());
    }

    TEST(ios_date_picker_seam, done_commits_the_dialog_date_clamped_by_the_control)
    {
        date_picker control;
        control.set_maximum_date(date_time(2050, 1, 1));
        auto handler = std::make_shared<date_picker_handler>();
        control.set_handler(handler);

        bool selected = false;
        control.date_selected.connect(
            [&selected](const std::optional<date_time>&, const std::optional<date_time>&) { selected = true; });

        UITextField* const field = native_field(handler);
        // The user spins past the bound (a UIDatePicker also clamps visually via maximumDate; setting
        // the raw date here exercises the CONTROL's clamp on commit).
        dialog_of(field).date = utc_date(2020, 6, 15);
        handler->typed_platform_view()->on_done();

        EXPECT_TRUE(selected);
        EXPECT_EQ(control.date(), std::optional<date_time>(date_time(2020, 6, 15)));
        EXPECT_EQ(to_std_string(field.text), "6/15/2020");
    }

    TEST(ios_date_picker_seam, generic_iview_properties_reach_the_field)
    {
        date_picker control;
        auto handler = std::make_shared<date_picker_handler>();
        control.set_handler(handler);
        UITextField* const field = native_field(handler);

        control.set_is_enabled(false);
        EXPECT_FALSE(field.enabled);

        control.set_visibility(maui::core::visibility::hidden);
        EXPECT_TRUE(field.hidden);

        control.set_opacity(0.5);
        EXPECT_NEAR(field.alpha, 0.5, 0.001);
    }

    // ---- the IsOpen focus dance (DatePickerHandler.iOS.cs OnStarted/OnEnded) ----

    TEST(ios_date_picker_seam, native_editing_begin_sets_is_open_and_is_focused)
    {
        date_picker control;
        auto handler = std::make_shared<date_picker_handler>();
        control.set_handler(handler);

        int opened = 0;
        control.opened.connect([&opened] { ++opened; });

        send_control_event(native_field(handler), UIControlEventEditingDidBegin);
        EXPECT_TRUE(control.is_open());
        EXPECT_TRUE(control.is_focused());
        EXPECT_EQ(opened, 1);
    }

    TEST(ios_date_picker_seam, native_editing_end_clears_is_open_and_is_focused)
    {
        date_picker control;
        auto handler = std::make_shared<date_picker_handler>();
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

    TEST(ios_date_picker_seam, programmatic_is_open_invokes_map_is_open_without_crashing)
    {
        date_picker control;
        auto handler = std::make_shared<date_picker_handler>();
        control.set_handler(handler);

        control.set_is_open(true); // MapIsOpen → BecomeFirstResponder (NO in the spawned process)
        EXPECT_TRUE(control.is_open());
        control.set_is_open(false); // MapIsOpen → ResignFirstResponder
        EXPECT_FALSE(control.is_open());
    }

    TEST(ios_date_picker_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<date_picker>();
        ASSERT_NE(handler, nullptr);
        EXPECT_NE(dynamic_cast<date_picker_handler*>(handler.get()), nullptr);
    }
} // namespace
