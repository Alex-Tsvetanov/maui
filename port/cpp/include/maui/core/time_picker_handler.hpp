#pragma once
// maui::core::time_picker_handler  <=  Microsoft.Maui.Handlers.TimePickerHandler
//
// The handler for the time picker. Time/Format/appearance flow virtual→native through the property
// mapper; a native pick commits back through i_time_picker::set_time with the SECONDS DROPPED
// (TimePickerHandler.SetVirtualViewTime builds `new TimeSpan(hour, minute, 0)`). Ported from
// TimePickerHandler.cs (cross-platform) + TimePickerHandler.iOS.cs / Platform/iOS/MauiTimePicker.cs +
// TimePickerExtensions.cs (the UITextField-whose-inputView-is-a-time-mode-UIDatePicker recipe —
// replicated 1:1 on the ios backend; the AppKit backend translates it idiomatically to a time-only
// NSDatePicker, deviations documented in the .mm).
//
// Partial-class split (PROFILE §5): mapper tables + ctor here/cpp; the platform recipe per backend
// under src/platform/<backend>/time_picker_handler.{cpp,mm}.
//
// time_picker_platform mirrors every mapped property for the headless backend. `time` is the native
// wheel's current value (a null virtual Time falls back to zero, per TimePickerExtensions.UpdateTime);
// `text` is the formatted display string. `on_done` is the inbound channel: the headless stand-in for
// the Done-accessory tap (MauiTimePicker's dateSelected callback → SetVirtualViewTime).

#include <memory>
#include <string>
#include <string_view>

#include "maui/core/command_mapper.hpp"
#include "maui/core/date_time.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_time_picker.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    struct time_picker_platform : view_platform_base
    {
        time_picker_platform() = default;
        ~time_picker_platform() override; // backend-defined: releases the retained native view on Apple/iOS
        time_picker_platform(const time_picker_platform&) = delete;
        time_picker_platform(time_picker_platform&&) = delete;
        time_picker_platform& operator=(const time_picker_platform&) = delete;
        time_picker_platform& operator=(time_picker_platform&&) = delete;

        void* native = nullptr;
        // Headless mirror of every mapped property (the Apple/iOS builds write to `native` instead).
        time_span time;   // the native wheel value (falls back to zero)
        std::string text; // the formatted display string (MauiTimePicker.Text)
        maui::graphics::color text_color;
        font text_font;
        double character_spacing = 0;

        // Inbound channel (wired by the platform partial; headless tests invoke it directly): commit
        // the wheel's current `time` — the Done-tap analog (SetVirtualViewTime, seconds dropped).
        move_only_function<void()> on_done;

#ifdef MAUI_PLATFORM_APPLE
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_shadow(const maui::core::i_shadow* value) override;
        void update_clip(const maui::graphics::i_shape* value) override;
        void update_semantics(const maui::core::semantics* value) override;
        void update_input_transparent(bool value) override;
#endif

#ifdef MAUI_PLATFORM_IOS
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
#endif
    };

    class time_picker_handler : public view_handler<time_picker_handler, i_time_picker, time_picker_platform>
    {
    public:
        time_picker_handler();

        static property_mapper<i_time_picker, time_picker_handler>& mapper();
        static command_mapper<i_time_picker, time_picker_handler>& command_mapper();

        static std::unique_ptr<time_picker_platform> create_platform_view();
        void on_connect_handler(time_picker_platform& platform);
        static void on_disconnect_handler(time_picker_platform& platform);

        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // Property map functions (platform recipe). map_format shares map_time's body — C#'s
        // UpdateFormat routes straight into UpdateTime (a format change re-renders the text).
        static void map_format(time_picker_handler& handler, i_time_picker& view);
        static void map_time(time_picker_handler& handler, i_time_picker& view);
        static void map_text_color(time_picker_handler& handler, i_time_picker& view);
        static void map_font(time_picker_handler& handler, i_time_picker& view);
        static void map_character_spacing(time_picker_handler& handler, i_time_picker& view);
    };
} // namespace maui::core
