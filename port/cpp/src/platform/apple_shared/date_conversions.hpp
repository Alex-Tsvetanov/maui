#pragma once
// NSDate ⇄ maui::core::date_time / time_span conversions, shared by the Apple (AppKit) AND iOS
// (UIKit) picker partials — Foundation's NSDate is identical on both, exactly like the C# oracle's
// shared DateTimeExtensions.ToNSDate/ToDateTime (src/Core/src/Platform/iOS/DateTimeExtensions.cs).
// Objective-C++ only — include exclusively from .mm files.
//
// Convention (matching the C# recipes): the native pickers run on the UTC time zone
// (`new NSTimeZone("UTC")` in MauiDatePicker/MauiTimePicker), so a date_time's calendar day maps to
// midnight UTC and the components survive the round trip exactly. NSDate's reference epoch is
// 2001-01-01T00:00Z; we convert through timeIntervalSince1970 (the Unix epoch std::chrono::sys_days
// is anchored on).

#import <Foundation/Foundation.h>

#include <chrono>
#include <cmath>

#include "maui/core/date_time.hpp"

namespace maui::platform::apple_shared
{
    // DateTimeExtensions.ToNSDate (UTC): the day at midnight plus the time-of-day.
    inline NSDate* to_ns_date(const maui::core::date_time& value)
    {
        const auto since_epoch =
            std::chrono::duration_cast<std::chrono::milliseconds>(value.days().time_since_epoch()) +
            value.time_of_day();
        return [NSDate dateWithTimeIntervalSince1970:static_cast<NSTimeInterval>(since_epoch.count()) / 1000.0];
    }

    // DateTimeExtensions.ToDateTime (UTC): split the Unix interval back into day + time-of-day.
    inline maui::core::date_time to_date_time(NSDate* value)
    {
        // llround (round-half-away) keeps pre-1970 (negative) intervals exact — a +0.5 truncation
        // would skew them by 1ms and break the midnight round trip for e.g. the 1900-01-01 minimum.
        const auto since_epoch = std::chrono::milliseconds(std::llround(value.timeIntervalSince1970 * 1000.0));
        const auto day = std::chrono::floor<std::chrono::days>(since_epoch);
        return maui::core::date_time{std::chrono::sys_days{day},
                                     std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch - day)};
    }

    // The time-of-day round trip the time picker uses: a time_span anchored on 0001-01-01 in C#
    // (MauiTimePicker.UpdateTime) — the port anchors on the Unix epoch day instead, which is
    // equivalent for extracting hours/minutes/seconds back out under UTC.
    inline NSDate* to_ns_date(const maui::core::time_span& value)
    {
        return to_ns_date(maui::core::date_time{std::chrono::sys_days{},
                                                std::chrono::duration_cast<std::chrono::milliseconds>(value.value())});
    }

    inline maui::core::time_span to_time_span(NSDate* value)
    {
        return maui::core::time_span{to_date_time(value).time_of_day()};
    }
} // namespace maui::platform::apple_shared
