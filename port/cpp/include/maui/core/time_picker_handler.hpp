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

#include <cstdint>
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
        // Render transform pushed to the native view via the shared ios apply_transform helper
        // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
        void update_transform(const maui::core::transform_spec& value) override;
        // BackgroundColor IS pushed to the MauiTimePicker (a RoundedRect UITextField): a solid fill goes to
        // the UIView backgroundColor property (flat fill, bezel suppressed, like MAUI); gradient/image use
        // the shared backing-layer helper. Mirrors the picker/date_picker handlers' update_background.
        void update_background(const maui::graphics::paint* value) override;
        // Clip IS pushed: WrapperView.SetClip masks the MauiIosTimePicker (UITextField)'s layer (the shared
        // apply_and_store_clip; MauiIosTimePicker.layoutSubviews re-frames the mask to the live bounds, the
        // 0×0-at-map-time fix).
        void update_clip(const maui::graphics::i_shape* value) override;
#endif

#ifdef MAUI_PLATFORM_ANDROID
        // Android backend: a non-editable android.widget.EditText stand-in for MauiTimePicker (the field text
        // is the formatted Time; the TimePickerDialog + 24h-view mode are deferred). Defined in
        // src/platform/android/time_picker_handler.cpp; base body FIRST then widget push. IsEnabled IS pushed.
        // Clip IS pushed (wave 24): the generic-view clip rides a ViewOutlineProvider + setClipToOutline(true)
        // (android_clip_ops.hpp apply_outline_clip) — convex shapes (ellipse/rect/rounded-rect, the
        // clip_views page's shared EllipseGeometry) clip exactly; a non-convex path keeps the headless mirror
        // (the honest deferral documented there). update_clip stashes the borrow in clip_shape so
        // platform_arrange re-resolves it against the live bounds (the geometry is bounds-dependent — the
        // iOS reapply_clip analog).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_semantics(const maui::core::semantics* value) override;
        void update_clip(const maui::graphics::i_shape* value) override;
        // The clip borrow platform_arrange re-resolves against the live bounds (null = no clip). Android-gated.
        const maui::graphics::i_shape* clip_shape = nullptr;
#endif

#ifdef MAUI_PLATFORM_WINDOWS
        // Windows (WinUI 3) backend: push the fundamental IView properties + Background to the real
        // Microsoft.UI.Xaml.Controls.TimePicker (src/platform/windows/time_picker_handler.cpp). Each
        // override calls the view_platform_base body FIRST — the windows preset also runs the
        // cross-platform suite on the host WITHOUT a XAML runtime (create_platform_view degrades to a
        // null native there) and that suite observes the headless mirrors — then pushes to the control
        // when one exists. Background joins the four because C#'s TimePickerHandler.Windows.cs has a
        // Windows-specific MapBackground (TimePickerExtensions.UpdateBackground). transform /
        // flow_direction / semantics / shadow / clip / input_transparent keep the base mirrors in this
        // first cut (deferred — see the partial).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_background(const maui::graphics::paint* value) override;
        // Native event wiring state (NO winrt types in shared headers — opaque ints only): the
        // winrt::event_token value of the SelectedTimeChanged subscription
        // TimePickerHandler.Windows.cs ConnectHandler installs; revoked in on_disconnect_handler.
        std::int64_t selected_time_changed_token = 0;
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
        // TimePickerHandler.MapIsOpen: become first responder when IsOpen, else resign.
        static void map_is_open(time_picker_handler& handler, i_time_picker& view);
    };
} // namespace maui::core
